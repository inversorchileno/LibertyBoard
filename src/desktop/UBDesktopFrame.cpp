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

#include "UBDesktopFrame.h"

#include <QPainter>
#include <QScreen>

#include "core/UBSetting.h"
#include "core/UBSettings.h"

UBDesktopFrame::UBDesktopFrame(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                      | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus | Qt::X11BypassWindowManagerHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void UBDesktopFrame::showOn(QScreen* screen)
{
    UBSettings* settings = UBSettings::settings();

    if (!screen || !settings->desktopBorderEnabled->get().toBool())
    {
        hide();
        return;
    }

    setGeometry(screen->geometry());
    show();
    raise();
    update();
}

void UBDesktopFrame::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    UBSettings* settings = UBSettings::settings();
    const int width = qBound(1, settings->desktopBorderWidth->get().toInt(), sMaximumWidth);
    QColor color(settings->desktopBorderColor->get().toString());

    if (!color.isValid())
    {
        color = Qt::red;
    }

    // Only the band along the edges is painted: the rest stays transparent.
    QPainter painter(this);
    const QRect area = rect();
    painter.fillRect(QRect(area.left(), area.top(), area.width(), width), color);
    painter.fillRect(QRect(area.left(), area.bottom() - width + 1, area.width(), width), color);
    painter.fillRect(QRect(area.left(), area.top() + width, width, area.height() - 2 * width), color);
    painter.fillRect(QRect(area.right() - width + 1, area.top() + width, width, area.height() - 2 * width), color);
}
