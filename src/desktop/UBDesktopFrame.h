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

#include <QWidget>

class QScreen;

/**
 * @brief The colored border around the screen that shows the desktop mode is on.
 *
 * A window of its own, above the desktop and transparent to the mouse: the
 * clicks go through it to the desktop and the drawings. Its color and
 * width are set in the preferences, where it can also be turned off.
 */
class UBDesktopFrame : public QWidget
{
    Q_OBJECT

public:
    explicit UBDesktopFrame(QWidget* parent = nullptr);

    /** @brief Shows the border around this screen, as set in the preferences, or nothing when it is off. */
    void showOn(QScreen* screen);

    static constexpr int sMaximumWidth = 12;

protected:
    void paintEvent(QPaintEvent* event) override;
};
