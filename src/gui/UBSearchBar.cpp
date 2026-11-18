/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBSearchBar.h"

#include <QApplication>
#include <QEvent>
#include <QGraphicsPathItem>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBMainWindow.h"
#include "pdf/UBPdfTextSearch.h"

UBSearchBar::UBSearchBar(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("searchBar");
    setStyleSheet(
        "#searchBar { background: #2b2f36; border: 1px solid #4a505b; border-radius: 6px; }"
        "QLineEdit { background: #111317; color: #f0f0f0; border: 1px solid #3a404b; border-radius: 4px; padding: 3px 6px; }"
        "QLabel { color: #c8ccd2; }"
        "QToolButton { color: #f0f0f0; background: transparent; border: none; font-size: 15px; padding: 2px 6px; }"
        "QToolButton:hover { background: #4a505b; border-radius: 3px; }");

    mText = new QLineEdit(this);
    mText->setPlaceholderText(tr("Search..."));
    mText->setMinimumWidth(240);
    mText->installEventFilter(this);

    auto previous = new QToolButton(this);
    previous->setText(QStringLiteral("\u2039"));
    previous->setToolTip(tr("Previous result"));

    auto next = new QToolButton(this);
    next->setText(QStringLiteral("\u203A"));
    next->setToolTip(tr("Next result"));

    auto close = new QToolButton(this);
    close->setText(QStringLiteral("\u2715"));
    close->setToolTip(tr("Close"));

    mCount = new QLabel(this);
    mCount->setMinimumWidth(90);

    auto row = new QHBoxLayout();
    row->setSpacing(4);
    row->addWidget(mText, 1);
    row->addWidget(previous);
    row->addWidget(next);
    row->addWidget(close);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 6, 6, 6);
    layout->setSpacing(3);
    layout->addLayout(row);
    layout->addWidget(mCount);

    connect(mText, &QLineEdit::returnPressed, this, [this]() { step(+1); });
    connect(previous, &QToolButton::clicked, this, [this]() { step(-1); });
    connect(next, &QToolButton::clicked, this, [this]() { step(+1); });
    connect(close, &QToolButton::clicked, this, &QWidget::hide);

    parent->installEventFilter(this);
    hide();
}

void UBSearchBar::open()
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBGraphicsScene> scene = board ? board->activeScene() : nullptr;

    if (!scene || !pdfItemOf(scene.get()))
    {
        QMessageBox::information(window(), tr("Search"), tr("The search only works in a PDF with text."));
        return;
    }

    UBApplication::applicationController->showBoard();
    placeInWindow();
    show();
    raise();
    mText->setFocus();
    mText->selectAll();
}

bool UBSearchBar::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parent() && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    {
        placeInWindow();
    }
    else if (watched == mText && event->type() == QEvent::KeyPress)
    {
        auto key = static_cast<QKeyEvent*>(event);

        if (key->key() == Qt::Key_Escape)
        {
            hide();
            return true;
        }

        if ((key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter) && (key->modifiers() & Qt::ShiftModifier))
        {
            step(-1);
            return true;
        }
    }

    return QFrame::eventFilter(watched, event);
}

void UBSearchBar::hideEvent(QHideEvent* event)
{
    clearMark();
    QFrame::hideEvent(event);
}

void UBSearchBar::placeInWindow()
{
    auto window = qobject_cast<QMainWindow*>(parentWidget());
    const int top = window && window->menuBar() ? window->menuBar()->geometry().bottom() + 6 : 6;
    adjustSize();
    move(parentWidget()->width() - width() - 12, top);
}

void UBSearchBar::step(int direction)
{
    const QString text = mText->text().trimmed();
    UBBoardController* board = UBApplication::boardController;

    if (text.isEmpty() || !board)
    {
        return;
    }

    // A new text, or another document: search again.
    if (text != mSearchedText || board->selectedDocument() != mDocument)
    {
        mSearchedText = text;
        search();

        if (mPlaces.isEmpty())
        {
            return;
        }

        // The first place from the page shown on.
        std::shared_ptr<UBGraphicsScene> scene = board->activeScene();
        UBGraphicsPDFItem* item = scene ? pdfItemOf(scene.get()) : nullptr;
        const int shownPage = item ? item->pageNumber() : 1;
        int first = 0;

        while (first < mPlaces.size() && mPlaces.at(first).pdfPage < shownPage)
        {
            ++first;
        }

        showPlace(first < mPlaces.size() ? first : 0);
        return;
    }

    if (mPlaces.isEmpty())
    {
        return;
    }

    showPlace((mCurrent + direction + mPlaces.size()) % mPlaces.size());
}

void UBSearchBar::search()
{
    clearMark();
    mPlaces.clear();
    mCurrent = -1;

    UBBoardController* board = UBApplication::boardController;
    mDocument = board->selectedDocument();
    std::shared_ptr<UBGraphicsScene> scene = board->activeScene();
    UBGraphicsPDFItem* item = scene ? pdfItemOf(scene.get()) : nullptr;

    if (!mDocument || !item)
    {
        updateCount();
        return;
    }

    mPdfFile = mDocument->persistencePath() + "/" + UBPersistenceManager::objectDirectory + "/"
               + item->fileUuid().toString() + ".pdf";

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const UBPdfTextSearch pdf(mPdfFile);

    for (int page = 1; page <= pdf.pageCount(); ++page)
    {
        for (const QRectF& rect : pdf.find(page, mSearchedText))
        {
            mPlaces << Place{ page, rect };
        }
    }

    QApplication::restoreOverrideCursor();
    updateCount();
}

void UBSearchBar::showPlace(int index)
{
    UBBoardController* board = UBApplication::boardController;
    const Place place = mPlaces.at(index);
    const int sceneIndex = sceneIndexOf(place.pdfPage);

    if (sceneIndex < 0)
    {
        return;
    }

    mCurrent = index;
    updateCount();
    clearMark();

    if (sceneIndex != board->activeSceneIndex())
    {
        board->persistCurrentScene(false, true);
        board->setActiveDocumentScene(sceneIndex);
    }

    std::shared_ptr<UBGraphicsScene> scene = board->activeScene();
    UBGraphicsPDFItem* item = scene ? pdfItemOf(scene.get()) : nullptr;

    if (!item)
    {
        return;
    }

    // From points of the page to the page item, then to the board.
    const QRectF bounds = item->boundingRect();
    const QSizeF points = item->pageSize();
    const qreal sx = bounds.width() / points.width();
    const qreal sy = bounds.height() / points.height();
    const QRectF onItem(bounds.left() + place.rect.left() * sx, bounds.top() + place.rect.top() * sy,
                        place.rect.width() * sx, place.rect.height() * sy);
    const QPolygonF onBoard = item->mapToScene(onItem.adjusted(-2, -2, 2, 2));

    QPainterPath path;
    path.addPolygon(onBoard);
    path.closeSubpath();

    // A mark of the screen only: it is not part of the page and is not saved.
    mMark = new QGraphicsPathItem(path);
    mMark->setBrush(QColor(255, 214, 0, 110));
    mMark->setPen(QPen(QColor(255, 140, 0), 0));
    mMark->setZValue(1e9);
    mMark->setAcceptedMouseButtons(Qt::NoButton);
    scene->addItem(mMark);
    mMarkScene = scene;

    board->controlView()->centerOn(onBoard.boundingRect().center());
}

void UBSearchBar::clearMark()
{
    std::shared_ptr<UBGraphicsScene> scene = mMarkScene.lock();

    if (mMark && scene && mMark->scene() == scene.get())
    {
        scene->removeItem(mMark);
        delete mMark;
    }

    mMark = nullptr;
    mMarkScene.reset();
}

void UBSearchBar::updateCount()
{
    if (mSearchedText.isEmpty())
    {
        mCount->clear();
    }
    else if (mPlaces.isEmpty())
    {
        mCount->setText(tr("Not found"));
    }
    else
    {
        mCount->setText(tr("%1 of %2").arg(mCurrent + 1).arg(mPlaces.size()));
    }

    adjustSize();
}

int UBSearchBar::sceneIndexOf(int pdfPage) const
{
    if (!mDocument)
    {
        return -1;
    }

    // An imported PDF has one page of the board for each page of the PDF,
    // in the same order: that page is tried first.
    QList<int> candidates{ pdfPage - 1 };

    for (int i = 0; i < mDocument->pageCount(); ++i)
    {
        if (i != pdfPage - 1)
        {
            candidates << i;
        }
    }

    for (int index : std::as_const(candidates))
    {
        if (index < 0 || index >= mDocument->pageCount())
        {
            continue;
        }

        std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(mDocument, index);
        UBGraphicsPDFItem* item = scene ? pdfItemOf(scene.get()) : nullptr;

        if (item && item->pageNumber() == pdfPage
                && mPdfFile.endsWith(item->fileUuid().toString() + ".pdf"))
        {
            return index;
        }
    }

    return -1;
}

UBGraphicsPDFItem* UBSearchBar::pdfItemOf(UBGraphicsScene* scene)
{
    return qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());
}
