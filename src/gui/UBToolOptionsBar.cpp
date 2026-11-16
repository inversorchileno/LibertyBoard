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

#include "UBToolOptionsBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "gui/UBToolOptionsPanel.h"
#include "gui/UBToolOptionsPopup.h"

UBToolOptionsBar::UBToolOptionsBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("toolOptionsBar");
    setStyleSheet("QWidget#toolOptionsBar, QScrollArea, QWidget#toolOptionsContent { background: transparent; border: none; }"
                  "QLabel#noOptions { color: #9aa3b2; padding: 0 12px; }");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto content = new QWidget(scrollArea);
    mContent = content;
    content->setObjectName("toolOptionsContent");
    mContentLayout = new QHBoxLayout(content);
    mContentLayout->setContentsMargins(0, 0, 0, 0);

    mNoOptions = new QLabel(tr("This tool has no options."), content);
    mNoOptions->setObjectName("noOptions");
    mContentLayout->addWidget(mNoOptions);
    mContentLayout->addStretch();

    scrollArea->setWidget(content);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scrollArea);

    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged,
            this, &UBToolOptionsBar::onStylusToolChanged);
}

void UBToolOptionsBar::showTool(int tool)
{
    // The line tool shares the options of the pen.
    const int panelTool = (tool == UBStylusTool::Line) ? int(UBStylusTool::Pen) : tool;

    if (mCurrentPanel)
    {
        mCurrentPanel->hide();
        mCurrentPanel = nullptr;
    }

    if (!UBToolOptionsPopup::hasOptions(panelTool))
    {
        mNoOptions->show();
        return;
    }

    if (!mPanels.contains(panelTool))
    {
        UBToolOptionsPanel* panel = UBToolOptionsPanel::create(panelTool, mNoOptions->parentWidget());

        if (!panel)
        {
            mNoOptions->show();
            return;
        }

        panel->setHorizontal();
        mContentLayout->insertWidget(0, panel);
        mPanels.insert(panelTool, panel);
    }

    mNoOptions->hide();
    mCurrentPanel = mPanels.value(panelTool);
    mCurrentPanel->refresh();
    mCurrentPanel->show();
    updateGeometry();
}

QSize UBToolOptionsBar::sizeHint() const
{
    // As wide as the options, so that the toolbar gives them their room.
    return QSize(mContent->sizeHint().width(), QWidget::sizeHint().height());
}

void UBToolOptionsBar::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    showTool(UBDrawingController::drawingController()->stylusTool());
}

void UBToolOptionsBar::onStylusToolChanged(int tool)
{
    if (isVisible())
    {
        showTool(tool);
    }
}
