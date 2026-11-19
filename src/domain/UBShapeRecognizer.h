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

#include <QList>
#include <QPointF>

/**
 * @brief Recognises a shape drawn by hand with the pen: a straight line, a
 * circle, an oval, a triangle, a square, a rectangle or another polygon of
 * few sides, and gives it drawn cleanly.
 */
namespace UBShapeRecognizer
{
    struct Shape
    {
        /** @brief The points of the clean shape, in order. */
        QList<QPointF> points;

        /** @brief True when the last point joins the first one. */
        bool closed{false};

        /** @brief True for a round shape, drawn as a smooth curve; false for straight sides. */
        bool smooth{false};

        bool isValid() const { return points.size() >= 2; }
    };

    /** @brief The clean shape of the points of a stroke, or an invalid one when none is recognised. */
    Shape recognize(const QList<QPointF>& points);
}
