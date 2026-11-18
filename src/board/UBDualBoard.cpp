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


#include "UBDualBoard.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QPainter>
#include <QTabletEvent>
#include <QTimer>

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBDocumentTabs.h"
#include "gui/UBMainWindow.h"

// The frames telling which side the tools work on.
static const QColor sActiveFrame("#1f3f7a");
static const QColor sOtherFrame("#c9ced6");

UBOtherBoardView::UBOtherBoardView(QWidget* parent)
    : QGraphicsView(parent)
{
    setObjectName("otherBoardView");
    setFrameShape(QFrame::NoFrame);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(false);
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Click to work on this side"));
}

void UBOtherBoardView::mousePressEvent(QMouseEvent* event)
{
    event->accept();
    emit activationRequested();
}

void UBOtherBoardView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    emit resized();
}

void UBOtherBoardView::tabletEvent(QTabletEvent* event)
{
    if (event->type() == QEvent::TabletPress)
    {
        event->accept();
        emit activationRequested();
        return;
    }

    QGraphicsView::tabletEvent(event);
}

void UBOtherBoardView::drawBackground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawBackground(painter, rect);

    // The edge of the page, as on the board.
    auto boardScene = dynamic_cast<UBGraphicsScene*>(scene());

    if (boardScene)
    {
        const qreal penWidth = 8.0 / transform().m11();
        painter->setPen(QPen(boardScene->isDarkBackground() ? UBSettings::documentSizeMarkColorDarkBackground
                                                            : UBSettings::documentSizeMarkColorLightBackground, penWidth));
        painter->setBrush(Qt::NoBrush);
        const QRectF page = boardScene->normalizedSceneRect();
        painter->drawRect(page.adjusted(-penWidth / 2, -penWidth / 2, penWidth / 2, penWidth / 2));
    }
}

void UBOtherBoardView::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawForeground(painter, rect);

    painter->save();
    painter->resetTransform();
    painter->setPen(QPen(sOtherFrame, 3));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(QRectF(viewport()->rect()).adjusted(1.5, 1.5, -1.5, -1.5));
    painter->restore();
}

UBDualBoard::UBDualBoard(UBDocumentTabs* tabs, QObject* parent)
    : QObject(parent)
    , mTabs(tabs)
{
    connect(mTabs, &UBDocumentTabs::documentClosing, this, &UBDualBoard::onDocumentClosing);
    connect(mTabs, &UBDocumentTabs::countChanged, this, &UBDualBoard::onDocumentCountChanged);

    // A click on the tab of the other side makes it the active side.
    mTabs->setShowHandler([this](const std::shared_ptr<UBDocumentProxy>& document) {
        if (isEnabled() && document == mOtherDocument)
        {
            activateOtherSide();
            return true;
        }

        return false;
    });
}

bool UBDualBoard::isEnabled() const
{
    return mOtherView;
}

void UBDualBoard::setEnabled(bool enabled)
{
    UBBoardController* board = UBApplication::boardController;

    if (enabled == isEnabled() || !board)
    {
        emit enabledChanged(isEnabled());
        return;
    }

    UBBoardView* control = board->controlView();

    if (!enabled)
    {
        // The document of the other side keeps its tab.
        mOtherView->hide();
        mOtherView->deleteLater();
        mOtherView = nullptr;

        // The board alone again, after the pages sidebar.
        QHBoxLayout* layout = board->controlLayout();
        control->setParent(board->controlContainer());
        layout->removeWidget(mSplitter);
        layout->addWidget(control);
        control->show();
        control->lower();
        mSplitter->deleteLater();
        mSplitter = nullptr;
        mOtherScene.reset();
        mOtherDocument.reset();
        control->setActiveFrameShown(false);
        QTimer::singleShot(0, this, []() {
            UBApplication::boardController->paletteManager()->containerResized();
        });
        emit enabledChanged(false);
        return;
    }

    // Two documents at most: the others are closed first, after asking.
    const std::shared_ptr<UBDocumentProxy> active = board->selectedDocument();

    if (mTabs->count() > 2 && !keepTwoDocuments(active))
    {
        emit enabledChanged(false);
        return;
    }

    for (const std::shared_ptr<UBDocumentProxy>& document : mTabs->documents())
    {
        if (document != board->selectedDocument())
        {
            mOtherDocument = document;
        }
    }

    // With a single document, the other side is a new blank board, with a tab.
    if (!mOtherDocument)
    {
        mOtherDocument = UBPersistenceManager::persistenceManager()->createNewDocument();

        if (!mOtherDocument)
        {
            emit enabledChanged(false);
            return;
        }

        mTabs->addDocument(mOtherDocument);
    }

    mOtherPage = qBound(0, mTabs->pageOf(mOtherDocument), mOtherDocument->pageCount() - 1);
    mActiveOnLeft = true;

    mOtherView = new UBOtherBoardView(board->controlContainer());
    connect(mOtherView, &UBOtherBoardView::activationRequested, this, &UBDualBoard::activateOtherSide);

    placeSides();
    control->setActiveFrameShown(true);
    emit enabledChanged(true);

    // Seen whole, also while the two sides take their size.
    mFitPending = true;
    connect(mOtherView, &UBOtherBoardView::resized, this, &UBDualBoard::fitOtherSide);
    fitOtherSide();
}

void UBDualBoard::fitOtherSide()
{
    if (!mOtherView || !mFitPending || !mOtherDocument)
    {
        return;
    }

    std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(mOtherDocument, mOtherPage);
    const QRectF page = scene ? scene->normalizedSceneRect() : QRectF();

    if (page.isValid() && mOtherView->viewport()->width() > 0)
    {
        const qreal scale = qMin(mOtherView->viewport()->width() / page.width(), mOtherView->viewport()->height() / page.height());
        mOtherTransform = QTransform::fromScale(scale, scale);
        mOtherCenter = page.center();
    }

    showOtherSide();
}

bool UBDualBoard::keepTwoDocuments(const std::shared_ptr<UBDocumentProxy>& kept)
{
    const QList<std::shared_ptr<UBDocumentProxy>> documents = mTabs->documents();
    const int toClose = documents.size() - 2;

    QDialog dialog(UBApplication::mainWindow);
    dialog.setWindowTitle(tr("Double board"));

    auto layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel(tr("The double board shows two documents at a time. Choose which to close (%1):").arg(toClose), &dialog));

    QList<QPair<QCheckBox*, std::shared_ptr<UBDocumentProxy>>> choices;

    for (const std::shared_ptr<UBDocumentProxy>& document : documents)
    {
        auto box = new QCheckBox(mTabs->nameOf(document), &dialog);

        if (document == kept)
        {
            box->setEnabled(false);
            box->setToolTip(tr("The document you work on stays open"));
        }

        layout->addWidget(box);
        choices << qMakePair(box, document);
    }

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
    QPushButton* close = buttons->addButton(tr("Close and continue"), QDialogButtonBox::AcceptRole);
    close->setEnabled(false);
    layout->addWidget(buttons);

    for (const auto& choice : std::as_const(choices))
    {
        connect(choice.first, &QCheckBox::toggled, &dialog, [&choices, close, toClose]() {
            int checked = 0;

            for (const auto& other : std::as_const(choices))
            {
                checked += other.first->isChecked() ? 1 : 0;
            }

            close->setEnabled(checked == toClose);
        });
    }

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }

    for (const auto& choice : std::as_const(choices))
    {
        if (choice.first->isChecked() && !mTabs->closeDocument(choice.second))
        {
            return false;
        }
    }

    return mTabs->count() <= 2;
}

void UBDualBoard::onDocumentClosing(std::shared_ptr<UBDocumentProxy> document)
{
    // Without one of its two documents, the board is single again.
    if (isEnabled() && (document == mOtherDocument || document == UBApplication::boardController->selectedDocument()))
    {
        setEnabled(false);
    }
}

void UBDualBoard::onDocumentCountChanged(int count)
{
    if (!isEnabled() || count <= 2)
    {
        return;
    }

    // A third document was opened on the active side: one of the others closes.
    QTimer::singleShot(0, this, [this]() {
        if (!isEnabled() || mTabs->count() <= 2)
        {
            return;
        }

        const std::shared_ptr<UBDocumentProxy> active = UBApplication::boardController->selectedDocument();
        setEnabled(false);

        if (keepTwoDocuments(active))
        {
            setEnabled(true);
        }
    });
}

void UBDualBoard::activateOtherSide()
{
    UBBoardController* board = UBApplication::boardController;

    if (!mOtherView || !mOtherDocument || !board)
    {
        return;
    }

    UBBoardView* control = board->controlView();

    // The active side as it is now, to be shown on the other side.
    std::shared_ptr<UBDocumentProxy> document = board->selectedDocument();
    const int page = board->activeSceneIndex();
    const QTransform transform = control->transform();
    const QPointF center = control->mapToScene(control->viewport()->rect().center());

    mFitPending = false;

    // The other side becomes the board, at its zoom and place.
    const std::shared_ptr<UBDocumentProxy> otherDocument = mOtherDocument;
    const int otherPage = mOtherPage;
    const QTransform otherTransform = mOtherTransform;
    const QPointF otherCenter = mOtherCenter;

    board->persistCurrentScene(false, true);
    board->setActiveDocumentScene(otherDocument, qBound(0, otherPage, otherDocument->pageCount() - 1));

    auto restore = [control, otherTransform, otherCenter]() {
        const qreal now = control->transform().m11();

        if (now > 0 && otherTransform.m11() > 0 && qAbs(now - otherTransform.m11()) > 1e-6)
        {
            UBApplication::boardController->zoom(otherTransform.m11() / now, control->mapToScene(control->viewport()->rect().center()));
        }

        control->centerOn(otherCenter);
        UBApplication::applicationController->adjustDisplayView();
    };

    mOtherDocument = document;
    mOtherPage = page;
    mOtherTransform = transform;
    mOtherCenter = center;

    // The two sides change places, so that each document stays where it is seen.
    mActiveOnLeft = !mActiveOnLeft;
    placeSides();
    showOtherSide();

    restore();
    // Showing a page restores its saved view once the events are processed.
    QTimer::singleShot(0, this, restore);
}

void UBDualBoard::showOtherSide()
{
    if (!mOtherView || !mOtherDocument)
    {
        return;
    }

    mOtherScene = UBPersistenceManager::persistenceManager()->loadDocumentScene(mOtherDocument, mOtherPage);
    mOtherView->setScene(mOtherScene.get());
    mOtherView->setSceneRect(mOtherView->rect().translated(-mOtherView->rect().center()).adjusted(-100000, -100000, 100000, 100000));
    mOtherView->setTransform(mOtherTransform);
    mOtherView->centerOn(mOtherCenter);
}

void UBDualBoard::placeSides()
{
    UBBoardController* board = UBApplication::boardController;
    QHBoxLayout* layout = board->controlLayout();
    UBBoardView* control = board->controlView();

    // The two sides in a splitter whose line can be dragged, after the pages sidebar.
    if (!mSplitter)
    {
        mSplitter = new QSplitter(Qt::Horizontal, board->controlContainer());
        mSplitter->setObjectName("dualBoardSplitter");
        mSplitter->setChildrenCollapsible(false);
        mSplitter->setHandleWidth(8);
        mSplitter->setStyleSheet("QSplitter::handle { background: #3a404b; }"
                                 "QSplitter::handle:hover, QSplitter::handle:pressed { background: #f6d77a; }");
        layout->removeWidget(control);
        layout->addWidget(mSplitter, 1);
        connect(mSplitter, &QSplitter::splitterMoved, this, &UBDualBoard::limitSides);
    }

    // The sides change places: each keeps its width where it is.
    const QList<int> sizes = mSplitter->sizes();

    QWidget* left = mActiveOnLeft ? static_cast<QWidget*>(control) : mOtherView.data();
    QWidget* right = mActiveOnLeft ? static_cast<QWidget*>(mOtherView.data()) : control;
    mSplitter->insertWidget(0, left);
    mSplitter->insertWidget(1, right);
    mSplitter->setStretchFactor(0, 1);
    mSplitter->setStretchFactor(1, 1);

    if (sizes.size() == 2 && sizes.at(0) > 0 && sizes.at(1) > 0)
    {
        mSplitter->setSizes(sizes);
    }
    else
    {
        // Half and half to begin with.
        const int half = qMax(1, mSplitter->width() / 2);
        mSplitter->setSizes({ half, half });
    }

    control->show();
    mOtherView->show();

    // Under the palettes, which float over both sides.
    mSplitter->lower();

    // The palettes along the edges of both sides, once they have their size.
    QTimer::singleShot(0, this, []() {
        UBApplication::boardController->paletteManager()->containerResized();
    });
}

void UBDualBoard::limitSides()
{
    if (!mSplitter)
    {
        return;
    }

    const QList<int> sizes = mSplitter->sizes();
    const int total = sizes.value(0) + sizes.value(1);
    const int least = total / 5;

    if (sizes.size() == 2 && total > 0 && (sizes.at(0) < least || sizes.at(1) < least))
    {
        const int left = qBound(least, sizes.at(0), total - least);
        mSplitter->setSizes({ left, total - left });
    }

    UBApplication::boardController->paletteManager()->containerResized();
}
