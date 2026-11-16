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

#include "UBToolOptionsDock.h"

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "gui/UBToolOptionsPanel.h"
#include "gui/UBToolOptionsPopup.h"

static constexpr int sDockWidth = 340;

UBToolOptionsDock::UBToolOptionsDock(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("toolOptionsDock");
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("QWidget#toolOptionsDock, QScrollArea, QWidget#toolOptionsContent { background: #20242b; border: none; }"
                  "QLabel#noOptions { color: #9aa3b2; padding: 16px; }");
    setFixedWidth(sDockWidth);

    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto content = new QWidget(scrollArea);
    content->setObjectName("toolOptionsContent");
    mContentLayout = new QVBoxLayout(content);
    mContentLayout->setContentsMargins(0, 0, 0, 0);

    mNoOptions = new QLabel(tr("This tool has no options."), content);
    mNoOptions->setObjectName("noOptions");
    mNoOptions->setWordWrap(true);
    mContentLayout->addWidget(mNoOptions);
    mContentLayout->addStretch();

    scrollArea->setWidget(content);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scrollArea);

    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged,
            this, &UBToolOptionsDock::onStylusToolChanged);
}

void UBToolOptionsDock::showTool(int tool)
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

        panel->setMinimumWidth(0);
        mContentLayout->insertWidget(0, panel);
        mPanels.insert(panelTool, panel);
    }

    mNoOptions->hide();
    mCurrentPanel = mPanels.value(panelTool);
    mCurrentPanel->refresh();
    mCurrentPanel->show();
}

void UBToolOptionsDock::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    showTool(UBDrawingController::drawingController()->stylusTool());
}

void UBToolOptionsDock::onStylusToolChanged(int tool)
{
    if (isVisible())
    {
        showTool(tool);
    }
}
