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


#include "UBPdfOutlinePanel.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBSearchBar.h"
#include "pdf/UBPdfNavigation.h"

static const int PageRole = Qt::UserRole + 1;

UBPdfOutlinePanel::UBPdfOutlinePanel(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("outlinePanel");
    setStyleSheet(
        "#outlinePanel { background: #2b2f36; border: 1px solid #4a505b; border-radius: 6px; }"
        "QLabel { color: #f0f0f0; font-weight: bold; }"
        "QToolButton { color: #f0f0f0; background: transparent; border: none; font-size: 15px; padding: 2px 6px; }"
        "QToolButton:hover { background: #4a505b; border-radius: 3px; }"
        "QTreeWidget { background: #1d2026; color: #e8e8e8; border: 1px solid #3a404b; border-radius: 4px; outline: none; }"
        "QTreeWidget::item { padding: 4px 2px; }"
        "QTreeWidget::item:selected { background: #3d5a80; color: #ffffff; }"
        "QTreeWidget::item:hover:!selected { background: #2b3038; }");

    mTitle = new QLabel(tr("Index"), this);

    auto close = new QToolButton(this);
    close->setText(QStringLiteral("\u2715"));
    close->setToolTip(tr("Close"));
    connect(close, &QToolButton::clicked, this, &QWidget::hide);

    auto row = new QHBoxLayout;
    row->addWidget(mTitle, 1);
    row->addWidget(close);

    mTree = new QTreeWidget(this);
    mTree->setColumnCount(2);
    mTree->setHeaderHidden(true);
    mTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mTree->setWordWrap(true);
    mTree->header()->setStretchLastSection(false);
    mTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    connect(mTree, &QTreeWidget::itemClicked, this, &UBPdfOutlinePanel::goTo);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 6, 6, 6);
    layout->setSpacing(4);
    layout->addLayout(row);
    layout->addWidget(mTree, 1);

    parent->installEventFilter(this);
    hide();
}

static void addEntries(QTreeWidget* tree, QTreeWidgetItem* parent, const QList<UBPdfNavigation::Entry>& entries)
{
    for (const UBPdfNavigation::Entry& entry : entries)
    {
        auto item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(tree);
        item->setText(0, entry.title);
        item->setToolTip(0, entry.title);
        item->setData(0, PageRole, entry.page);

        if (entry.page > 0)
        {
            item->setText(1, QString::number(entry.page));
            item->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        }

        addEntries(tree, item, entry.children);
    }
}

void UBPdfOutlinePanel::open()
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBGraphicsScene> scene = board ? board->activeScene() : nullptr;
    auto item = scene ? qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject()) : nullptr;

    if (!item)
    {
        QMessageBox::information(window(), tr("Index"), tr("The index is only for a PDF."));
        return;
    }

    mPdfFile = UBPdfNavigation::pdfFileOf(board->selectedDocument(), item);
    const QList<UBPdfNavigation::Entry> entries = UBPdfNavigation(mPdfFile).outline();

    if (entries.isEmpty())
    {
        QMessageBox::information(window(), tr("Index"), tr("This PDF has no index."));
        return;
    }

    mTree->clear();
    addEntries(mTree, nullptr, entries);

    // The first level open, as most readers show it.
    for (int i = 0; i < mTree->topLevelItemCount(); ++i)
    {
        mTree->topLevelItem(i)->setExpanded(true);
    }

    UBApplication::applicationController->showBoard();
    placeInWindow();
    show();
    raise();
    mTree->setFocus();
}

bool UBPdfOutlinePanel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parent() && (event->type() == QEvent::Resize || event->type() == QEvent::Show) && isVisible())
    {
        placeInWindow();
    }

    return QFrame::eventFilter(watched, event);
}

void UBPdfOutlinePanel::placeInWindow()
{
    const QRect rect = UBSearchBar::panelRect(parentWidget(), 340, true);
    setFixedSize(rect.size());
    move(rect.topLeft());
}

void UBPdfOutlinePanel::goTo(QTreeWidgetItem* item)
{
    const int page = item ? item->data(0, PageRole).toInt() : 0;

    if (page > 0 && !UBPdfNavigation::goToPdfPage(mPdfFile, page))
    {
        UBApplication::showMessage(tr("That page is not in the document any more."));
    }
}
