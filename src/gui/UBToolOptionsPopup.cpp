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

#include "UBToolOptionsPopup.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "core/UB.h"
#include "gui/UBToolOptionsPanel.h"

static constexpr int sGap = 8;

UBToolOptionsPopup::UBToolOptionsPopup(QWidget* parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint)
{
    setObjectName("toolOptionsPopup");
    setStyleSheet("QFrame#toolOptionsPopup { background: #20242b; border: 1px solid #3a404b; }");

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(1, 1, 1, 1);
    mLayout->setSizeConstraint(QLayout::SetFixedSize);
}

bool UBToolOptionsPopup::hasOptions(int tool)
{
    switch (tool)
    {
        case UBStylusTool::Pen:
        case UBStylusTool::Eraser:
        case UBStylusTool::Marker:
        case UBStylusTool::Pointer:
        case UBStylusTool::Line:
        case UBStylusTool::Text:
        case UBStylusTool::Arrow:
        case UBStylusTool::Rectangle:
        case UBStylusTool::Ellipse:
        case UBStylusTool::Underline:
            return true;
        default:
            return false;
    }
}

void UBToolOptionsPopup::showForTool(int tool, const QRect& anchor)
{
    UBToolOptionsPanel* panel = panelFor(tool);

    if (!panel)
    {
        return;
    }

    // Only the panel of this tool is laid out, so that the popup takes its size.
    for (UBToolOptionsPanel* other : std::as_const(mPanels))
    {
        if (other != panel)
        {
            mLayout->removeWidget(other);
            other->hide();
        }
    }

    panel->refresh();
    mLayout->addWidget(panel);
    panel->show();
    adjustSize();

    const QScreen* screen = QGuiApplication::screenAt(anchor.center());
    const QRect available = screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);

    // Beside the button, on the side where there is room.
    QPoint position(anchor.right() + sGap, anchor.top());

    if (position.x() + width() > available.right())
    {
        position.setX(anchor.left() - sGap - width());
    }

    position.setY(qBound(available.top(), position.y(), available.bottom() - height()));
    position.setX(qBound(available.left(), position.x(), available.right() - width()));

    move(position);
    show();
    raise();
    activateWindow();
}

UBToolOptionsPanel* UBToolOptionsPopup::panelFor(int tool)
{
    // The line tool shares the color and width of the pen.
    const int panelTool = (tool == UBStylusTool::Line) ? int(UBStylusTool::Pen) : tool;

    if (!mPanels.contains(panelTool))
    {
        UBToolOptionsPanel* panel = UBToolOptionsPanel::create(panelTool, this);

        if (!panel)
        {
            return nullptr;
        }

        panel->hide();
        mPanels.insert(panelTool, panel);
    }

    return mPanels.value(panelTool);
}
