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

#include "UBEyedropper.h"

#include <QCursor>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>

void UBEyedropper::pick(std::function<void(const QColor&)> picked)
{
    // Popups and menus close first, so that the picture shows what is under them.
    QTimer::singleShot(200, [picked]() {
        QScreen* screen = QGuiApplication::screenAt(QCursor::pos());

        if (!screen)
        {
            screen = QGuiApplication::primaryScreen();
        }

        if (!screen)
        {
            return;
        }

        auto eyedropper = new UBEyedropper(screen->grabWindow(0), screen->geometry(), picked);
        eyedropper->show();
        eyedropper->raise();
        eyedropper->activateWindow();
        eyedropper->setFocus();
    });
}

UBEyedropper::UBEyedropper(const QPixmap& screenPicture, const QRect& screenGeometry, std::function<void(const QColor&)> picked)
    : QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint)
    , mPicture(screenPicture.toImage())
    , mRatio(screenPicture.devicePixelRatio() > 0 ? screenPicture.devicePixelRatio() : 1)
    , mPointer(QCursor::pos() - screenGeometry.topLeft())
    , mPicked(std::move(picked))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::StrongFocus);
    setGeometry(screenGeometry);
}

QColor UBEyedropper::colorAt(const QPoint& position) const
{
    const QPoint pixel(qRound(position.x() * mRatio), qRound(position.y() * mRatio));

    if (!mPicture.rect().contains(pixel))
    {
        return QColor();
    }

    return QColor(mPicture.pixel(pixel));
}

void UBEyedropper::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawImage(rect(), mPicture);

    // A magnified view of the pixels around the pointer, with the one taken
    // in the middle, and its color under it.
    const int pixels = 11;
    const int zoom = 9;
    const int size = pixels * zoom;
    QPoint corner = mPointer + QPoint(24, 24);

    if (corner.x() + size > width())
    {
        corner.setX(mPointer.x() - 24 - size);
    }

    if (corner.y() + size + 30 > height())
    {
        corner.setY(mPointer.y() - 24 - size - 30);
    }

    const QPoint center(qRound(mPointer.x() * mRatio), qRound(mPointer.y() * mRatio));
    const QRect source(center.x() - pixels / 2, center.y() - pixels / 2, pixels, pixels);
    const QRect target(corner, QSize(size, size));

    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawImage(target, mPicture, source);
    painter.setPen(QPen(Qt::white, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(corner + QPoint(pixels / 2 * zoom, pixels / 2 * zoom), QSize(zoom, zoom)));
    painter.setPen(QPen(QColor("#202328"), 2));
    painter.drawRect(target);

    const QColor color = colorAt(mPointer);
    const QRect label(corner.x(), corner.y() + size + 4, size, 24);
    painter.fillRect(label, QColor("#202328"));
    painter.fillRect(QRect(label.left() + 4, label.top() + 4, 16, 16), color);
    painter.setPen(Qt::white);
    painter.drawText(label.adjusted(26, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, color.name());
}

void UBEyedropper::mouseMoveEvent(QMouseEvent* event)
{
    mPointer = event->pos();
    update();
}

void UBEyedropper::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const QColor color = colorAt(event->pos());

        if (color.isValid() && mPicked)
        {
            mPicked(color);
        }
    }

    close();
}

void UBEyedropper::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        close();
        return;
    }

    QWidget::keyPressEvent(event);
}
