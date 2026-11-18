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

#include <QIcon>
#include <QPoint>

class QAction;
#include <QSize>
#include <Qt>

/**
 * @brief The small triangles on the tools that have options.
 *
 * Only the desktop palette shows them. It shares its actions with the
 * board palette, so it puts the triangles on them when it appears and
 * removes them when the board is back. Each triangle is black with a white
 * border and points to where the options open: away from the edge of the
 * screen where the palette stands.
 */
namespace UBToolOptionsArrows
{
    /**
     * @brief Draws the triangles for a palette standing at this edge.
     * Left: pointing right, bottom right. Right: pointing left, bottom left.
     * Top: pointing down, bottom center. Bottom: pointing up, top center.
     */
    void apply(Qt::Edge paletteEdge);

    /** @brief Gives the tools back their icons without triangle. */
    void remove();

    /** @brief Changes the icon of a tool, with its triangle when the triangles are shown. */
    void setIcon(QAction* action, const QIcon& icon);

    /** @brief True when a press at this point of a tool button is on its triangle. */
    bool isOnTriangle(Qt::Edge paletteEdge, const QPoint& position, const QSize& buttonSize, const QSize& iconSize);

    /** @brief The side where the options open, for a palette standing at this edge. */
    Qt::Edge openingSide(Qt::Edge paletteEdge);
}
