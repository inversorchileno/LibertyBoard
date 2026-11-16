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

#include "UBShapeGeometry.h"

#include <cmath>

#include <QLineF>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QRectF>
#include <QtMath>

#include "core/UB.h"

namespace
{

struct Piece
{
    QPolygonF points;
    Qt::PenCapStyle cap{Qt::FlatCap};
};

qreal length(const QPointF& vector)
{
    return std::hypot(vector.x(), vector.y());
}

QPolygonF ellipseOutline(const QRectF& rect)
{
    QPainterPath path;
    path.addEllipse(rect);
    const QList<QPolygonF> polygons = path.toSubpathPolygons();

    if (polygons.isEmpty())
    {
        return {};
    }

    QPolygonF outline = polygons.first();

    if (outline.size() > 1 && outline.first() == outline.last())
    {
        outline.removeLast();
    }

    return outline;
}

QPolygonF rectangleOutline(const QRectF& rect)
{
    return QPolygonF({ rect.topLeft(), rect.topRight(), rect.bottomRight(), rect.bottomLeft() });
}

/** Total length of a polyline, closed or not. */
qreal polylineLength(const QPolygonF& points, bool closed)
{
    qreal total = 0;

    for (int i = 1; i < points.size(); ++i)
    {
        total += QLineF(points.at(i - 1), points.at(i)).length();
    }

    if (closed && points.size() > 1)
    {
        total += QLineF(points.last(), points.first()).length();
    }

    return total;
}

/** The polyline of a closed outline, with its first point repeated at the end. */
QPolygonF openRing(const QPolygonF& points)
{
    QPolygonF ring = points;

    if (!ring.isEmpty())
    {
        ring << ring.first();
    }

    return ring;
}

/** The part of an open polyline between two distances from its start. */
QPolygonF subPolyline(const QPolygonF& points, qreal from, qreal to)
{
    QPolygonF result;
    qreal travelled = 0;

    for (int i = 1; i < points.size(); ++i)
    {
        const QLineF segment(points.at(i - 1), points.at(i));
        const qreal segmentLength = segment.length();
        const qreal segmentStart = travelled;
        const qreal segmentEnd = travelled + segmentLength;
        travelled = segmentEnd;

        if (segmentEnd < from || segmentLength <= 0)
        {
            continue;
        }

        if (segmentStart > to)
        {
            break;
        }

        const qreal startRatio = qMax(0.0, (from - segmentStart) / segmentLength);
        const qreal endRatio = qMin(1.0, (to - segmentStart) / segmentLength);

        if (result.isEmpty())
        {
            result << segment.pointAt(startRatio);
        }

        result << segment.pointAt(endRatio);
    }

    return result;
}

/** Bumps along a closed outline, like the cloud border of Drawboard PDF. */
QPolygonF cloudOutline(const QPolygonF& outline, qreal width)
{
    const qreal perimeter = polylineLength(outline, true);

    if (perimeter <= 0)
    {
        return outline;
    }

    const int bumpCount = qMax(6, qRound(perimeter / qMax(width * 8, 24.0)));
    const qreal step = perimeter / bumpCount;
    const QPolygonF ring = openRing(outline);

    QList<QPointF> samples;

    for (int i = 0; i < bumpCount; ++i)
    {
        const QPolygonF part = subPolyline(ring, i * step, i * step);
        samples << (part.isEmpty() ? ring.first() : part.first());
    }

    QPainterPath cloud;
    cloud.moveTo(samples.first());

    for (int i = 0; i < bumpCount; ++i)
    {
        const QPointF from = samples.at(i);
        const QPointF to = samples.at((i + 1) % bumpCount);
        const QPointF middle = (from + to) / 2;
        const QPointF direction = to - from;
        const qreal directionLength = length(direction);

        if (directionLength <= 0)
        {
            continue;
        }

        QPointF normal(direction.y() / directionLength, -direction.x() / directionLength);

        // The bump goes outside of the shape.
        if (outline.containsPoint(middle + normal * 0.5, Qt::OddEvenFill))
        {
            normal = -normal;
        }

        cloud.quadTo(middle + normal * step * 0.6, to);
    }

    const QList<QPolygonF> polygons = cloud.toSubpathPolygons();

    if (polygons.isEmpty())
    {
        return outline;
    }

    QPolygonF result = polygons.first();

    if (result.size() > 1 && result.first() == result.last())
    {
        result.removeLast();
    }

    return result;
}

/** Splits a line into the pieces drawn by a dash pattern. */
QList<Piece> dashPieces(const QPolygonF& line, int dash, qreal width)
{
    QList<Piece> pieces;
    const qreal total = polylineLength(line, false);

    if (dash == UBLineDash::Solid)
    {
        pieces << Piece{ line, Qt::FlatCap };
        return pieces;
    }

    // On and off lengths, in widths. Round caps add half a width at each
    // end of a dash, so the gaps are made longer by one width.
    QList<qreal> pattern;

    switch (dash)
    {
        case UBLineDash::Dotted:
            pattern = { 0, 2.5 };
            break;
        case UBLineDash::ShortDash:
            pattern = { 2, 3 };
            break;
        case UBLineDash::LongDash:
            pattern = { 5, 3 };
            break;
        default:
            pattern = { 4, 2.5, 0, 2.5 };
            break;
    }

    qreal position = 0;
    int index = 0;

    while (position <= total)
    {
        const qreal on = pattern.at(index) * width;
        const qreal off = pattern.at(index + 1) * width;
        QPolygonF dashLine = subPolyline(line, position, qMin(position + on, total));

        if (dashLine.size() == 1 || (dashLine.size() == 2 && dashLine.first() == dashLine.last()))
        {
            // A dot: a very short segment drawn with round caps.
            const QPointF point = dashLine.first();
            dashLine = QPolygonF({ point, point + QPointF(0.01, 0) });
        }

        if (!dashLine.isEmpty())
        {
            pieces << Piece{ dashLine, Qt::RoundCap };
        }

        position += on + off;
        index = (index + 2) % pattern.size();
    }

    return pieces;
}

/** The pieces of a closed outline. A closed solid line is cut in four so that no polygon has a hole. */
QList<Piece> closedOutlinePieces(const QPolygonF& outline, int dash, qreal width)
{
    const QPolygonF ring = openRing(outline);

    if (dash != UBLineDash::Solid)
    {
        return dashPieces(ring, dash, width);
    }

    QList<Piece> pieces;
    const qreal total = polylineLength(ring, false);
    const int pieceCount = 4;

    for (int i = 0; i < pieceCount; ++i)
    {
        // Cut between corners, where flat caps meet without a notch.
        const qreal from = total * (i + 0.125) / pieceCount;
        const qreal to = total * (i + 1.125) / pieceCount;

        QPolygonF piece = subPolyline(ring, from, qMin(to, total));

        if (to > total)
        {
            const QPolygonF wrapped = subPolyline(ring, 0, to - total);

            for (int j = 1; j < wrapped.size(); ++j)
            {
                piece << wrapped.at(j);
            }
        }

        pieces << Piece{ piece, Qt::FlatCap };
    }

    return pieces;
}

QList<QPolygonF> strokePolygons(const QPainterPath& path, qreal width, Qt::PenCapStyle cap)
{
    QPainterPathStroker stroker;
    stroker.setWidth(width);
    stroker.setCapStyle(cap);
    stroker.setJoinStyle(Qt::RoundJoin);

    return stroker.createStroke(path).simplified().toFillPolygons();
}

QPainterPath polylinePath(const QPolygonF& points)
{
    QPainterPath path;

    if (points.isEmpty())
    {
        return path;
    }

    path.moveTo(points.first());

    for (int i = 1; i < points.size(); ++i)
    {
        path.lineTo(points.at(i));
    }

    return path;
}

void addPolygons(QList<UBShapePart>& parts, const QList<QPolygonF>& polygons, const QColor& color)
{
    for (const QPolygonF& polygon : polygons)
    {
        if (polygon.size() > 2)
        {
            parts << UBShapePart{ polygon, color };
        }
    }
}

/** How far the line is shortened so that it stops where a filled ending starts. */
qreal shaftShortening(int ending, qreal headSize)
{
    switch (ending)
    {
        case UBLineEnding::ClosedArrow:
            return headSize * 0.8;
        case UBLineEnding::Circle:
        case UBLineEnding::Square:
        case UBLineEnding::Diamond:
            return headSize;
        default:
            return 0;
    }
}

/** A filled ending at the tip, or the two wings of an open arrow added to the wings path. */
QPolygonF endingPolygon(int ending, const QPointF& tip, const QPointF& along, qreal headSize, QPainterPath& wings)
{
    const QPointF across(-along.y(), along.x());
    const QPointF center = tip - along * headSize / 2;

    switch (ending)
    {
        case UBLineEnding::OpenArrow:
            wings.moveTo(tip - along * headSize + across * headSize * 0.6);
            wings.lineTo(tip);
            wings.lineTo(tip - along * headSize - across * headSize * 0.6);
            return {};

        case UBLineEnding::ClosedArrow:
            return QPolygonF({ tip, tip - along * headSize + across * headSize * 0.55,
                               tip - along * headSize - across * headSize * 0.55 });

        case UBLineEnding::Circle:
            return ellipseOutline(QRectF(center.x() - headSize / 2, center.y() - headSize / 2, headSize, headSize));

        case UBLineEnding::Square:
        {
            const qreal half = headSize * 0.45;
            return QPolygonF({ center + along * half + across * half, center + along * half - across * half,
                               center - along * half - across * half, center - along * half + across * half });
        }

        case UBLineEnding::Diamond:
            return QPolygonF({ center + along * headSize / 2, center + across * headSize * 0.4,
                               center - along * headSize / 2, center - across * headSize * 0.4 });

        default:
            return {};
    }
}

QList<UBShapePart> buildClosedShape(int tool, const QPointF& start, const QPointF& end, const UBShapeStyle& style,
                                    const QColor& strokeColor, const QColor& fillColor)
{
    QList<UBShapePart> parts;
    const QRectF rect = QRectF(start, end).normalized();

    QPolygonF outline = (tool == UBStylusTool::Ellipse) ? ellipseOutline(rect) : rectangleOutline(rect);

    if (style.borderStyle == UBBorderStyle::Cloud)
    {
        outline = cloudOutline(outline, style.width);
    }

    if (fillColor.isValid())
    {
        parts << UBShapePart{ outline, fillColor };
    }

    const QList<Piece> pieces = closedOutlinePieces(outline, style.dash, style.width);

    for (const Piece& piece : pieces)
    {
        addPolygons(parts, strokePolygons(polylinePath(piece.points), style.width, piece.cap), strokeColor);
    }

    return parts;
}

QList<UBShapePart> buildArrow(const QPointF& start, const QPointF& end, const UBShapeStyle& style, const QColor& color)
{
    QList<UBShapePart> parts;
    const QPointF direction = end - start;
    const qreal lineLength = length(direction);

    if (lineLength <= 0)
    {
        return parts;
    }

    const QPointF along = direction / lineLength;
    const qreal headSize = qMax(style.width * 3.5, style.minimumHeadSize);

    const qreal startCut = qMin(shaftShortening(style.startEnding, headSize), lineLength / 2);
    const qreal endCut = qMin(shaftShortening(style.endEnding, headSize), lineLength / 2);
    const QPolygonF shaft({ start + along * startCut, end - along * endCut });

    QPainterPath wings;
    const QPolygonF startShape = endingPolygon(style.startEnding, start, -along, headSize, wings);
    const QPolygonF endShape = endingPolygon(style.endEnding, end, along, headSize, wings);

    if (style.dash == UBLineDash::Solid)
    {
        // The line and the wings of open arrows are one outline, so that
        // they do not overlap.
        QPainterPath path = polylinePath(shaft);
        path.addPath(wings);
        addPolygons(parts, strokePolygons(path, style.width, Qt::RoundCap), color);
    }
    else
    {
        const QList<Piece> pieces = dashPieces(shaft, style.dash, style.width);

        for (const Piece& piece : pieces)
        {
            addPolygons(parts, strokePolygons(polylinePath(piece.points), style.width, piece.cap), color);
        }

        if (!wings.isEmpty())
        {
            addPolygons(parts, strokePolygons(wings, style.width, Qt::RoundCap), color);
        }
    }

    if (!startShape.isEmpty())
    {
        parts << UBShapePart{ startShape, color };
    }

    if (!endShape.isEmpty())
    {
        parts << UBShapePart{ endShape, color };
    }

    return parts;
}

QColor withOpacity(QColor color, qreal opacity)
{
    if (color.isValid())
    {
        color.setAlphaF(qBound(0.0, opacity, 1.0));
    }

    return color;
}

} // namespace

QList<UBShapePart> UBShapeGeometry::build(int tool, const QPointF& start, const QPointF& end, const UBShapeStyle& style)
{
    const QColor strokeColor = withOpacity(style.strokeColor.isValid() ? style.strokeColor : QColor(Qt::black), style.opacity);

    switch (tool)
    {
        case UBStylusTool::Rectangle:
        case UBStylusTool::Ellipse:
            return buildClosedShape(tool, start, end, style, strokeColor, withOpacity(style.fillColor, style.opacity));

        case UBStylusTool::Arrow:
            return buildArrow(start, end, style, strokeColor);

        case UBStylusTool::Highlight:
        {
            // A horizontal band over a line of text, as tall as the width.
            const qreal left = qMin(start.x(), end.x());
            const qreal right = qMax(start.x(), end.x());
            const qreal top = start.y() - style.width / 2;
            const QRectF band(left, top, right - left, style.width);

            return { UBShapePart{ QPolygonF(band), strokeColor } };
        }

        case UBStylusTool::Underline:
        {
            // Always fully opaque and horizontal.
            QColor color = strokeColor;
            color.setAlphaF(1.0);
            const QPolygonF line({ start, QPointF(end.x(), start.y()) });

            QList<UBShapePart> parts;
            addPolygons(parts, strokePolygons(polylinePath(line), style.width, Qt::FlatCap), color);
            return parts;
        }

        default:
            return {};
    }
}

QPointF UBShapeGeometry::constrainedEnd(int tool, const QPointF& start, const QPointF& end)
{
    const QPointF delta = end - start;

    if (tool == UBStylusTool::Rectangle || tool == UBStylusTool::Ellipse)
    {
        const qreal side = qMax(std::fabs(delta.x()), std::fabs(delta.y()));
        return start + QPointF(delta.x() < 0 ? -side : side, delta.y() < 0 ? -side : side);
    }

    if (tool == UBStylusTool::Arrow)
    {
        QLineF line(start, end);
        line.setAngle(qRound(line.angle() / 15.0) * 15.0);
        return line.p2();
    }

    return end;
}
