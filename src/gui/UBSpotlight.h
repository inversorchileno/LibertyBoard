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

#include <QPixmap>
#include <QPoint>
#include <QTimer>
#include <QWidget>

/**
 * @brief The spotlight and the magnifier, which follow the pointer.
 *
 * A window over the whole screen where the pointer is, transparent to the
 * mouse. The spotlight darkens all but a circle around the pointer; the
 * magnifier shows, in a circle around the pointer, what is under it,
 * bigger. Shown while the spotlight tool is chosen, with the radius, the
 * darkness and the zoom set in its options.
 */
class UBSpotlight : public QWidget
{
    Q_OBJECT

public:
    explicit UBSpotlight(QWidget* parent = nullptr);

    /** @brief Follows the pointer, or hides. */
    void setActive(bool active);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void followPointer();
    QPixmap pictureUnder(const QRect& area) const;

    QTimer mTimer;
    QPoint mPointer;
    bool mHiddenFromCapture{false};
};
