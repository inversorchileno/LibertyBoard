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

#pragma once

#include <functional>

#include <QColor>
#include <QImage>
#include <QWidget>

/**
 * @brief Picks a color anywhere on the screen, like an eyedropper.
 *
 * A picture of the screen is shown over it, with a cross and a magnified
 * view of the place under the pointer. A click takes the color there;
 * Escape or a right click gives up.
 */
class UBEyedropper : public QWidget
{
    Q_OBJECT

public:
    /** @brief Lets the user pick a color, then calls back with it; nothing when given up. */
    static void pick(std::function<void(const QColor&)> picked);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    UBEyedropper(const QPixmap& screenPicture, const QRect& screenGeometry, std::function<void(const QColor&)> picked);

    QColor colorAt(const QPoint& position) const;

    QImage mPicture;
    qreal mRatio{1};
    QPoint mPointer;
    std::function<void(const QColor&)> mPicked;
};
