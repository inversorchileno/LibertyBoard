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

#include <QColor>
#include <QList>
#include <QPointF>
#include <QPolygonF>

/** @brief One filled polygon of a shape, with its color. */
struct UBShapePart
{
    QPolygonF polygon;
    QColor color;
};

/** @brief How a shape is drawn: the values of the shape tool settings. */
struct UBShapeStyle
{
    QColor strokeColor;
    QColor fillColor;       // invalid: not filled
    qreal width{3};
    int dash{0};            // UBLineDash
    int borderStyle{0};     // UBBorderStyle
    int startEnding{0};     // UBLineEnding
    int endEnding{0};       // UBLineEnding
    qreal opacity{1};
    qreal minimumHeadSize{10};  // smallest size of arrow endings, in scene units
    bool tapered{false};        // arrow line growing from its start to its tip
    int kind{0};                // UBShapeKind, for the shapes tool
};

/**
 * @brief Turns a rectangle, an ellipse, an arrow or an underline into
 * filled polygons.
 *
 * Shapes are drawn with the same polygons as pen strokes, so that they are
 * saved, erased and moved like strokes and every version of OpenBoard can
 * read the documents. The polygons of one color never overlap, so that a
 * transparent shape has the same transparency everywhere.
 */
class UBShapeGeometry
{
public:
    /**
     * @param tool the shape tool, from UBStylusTool
     * @param start, end the points where the drag started and ended
     * @param style the look of the shape, its width in scene units
     */
    static QList<UBShapePart> build(int tool, const QPointF& start, const QPointF& end, const UBShapeStyle& style);

    /** @brief A line through these corners, with the line style and width of the style. */
    static QList<UBShapePart> buildPolyline(const QPolygonF& corners, const UBShapeStyle& style);

    /**
     * @brief The outline of a shape of UBShapeKind in the rectangle, for the
     * shapes and their pictures. A line is its two ends, a polyline a zigzag.
     */
    static QPolygonF outline(int kind, const QRectF& rect);

    /** @brief The shapes of UBShapeKind, in the order they are offered. */
    static QList<int> shapeKinds();

    /** @brief Corrects the end point while Shift is held: squares, circles and lines at 15 degree steps. */
    static QPointF constrainedEnd(int tool, const QPointF& start, const QPointF& end, int kind = 0);
};
