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


#include "UBShapeRecognizer.h"

#include <QLineF>
#include <QPolygonF>
#include <QRectF>
#include <QtMath>

namespace
{
    qreal distanceToSegment(const QPointF& p, const QPointF& a, const QPointF& b)
    {
        const QPointF ab = b - a;
        const qreal length2 = QPointF::dotProduct(ab, ab);

        if (length2 <= 0)
        {
            return QLineF(p, a).length();
        }

        const qreal t = qBound<qreal>(0, QPointF::dotProduct(p - a, ab) / length2, 1);
        return QLineF(p, a + t * ab).length();
    }

    // Douglas-Peucker: the points that keep the shape within epsilon.
    void simplify(const QList<QPointF>& points, int first, int last, qreal epsilon, QList<int>& kept)
    {
        qreal farthest = 0;
        int index = -1;

        for (int i = first + 1; i < last; ++i)
        {
            const qreal distance = distanceToSegment(points.at(i), points.at(first), points.at(last));

            if (distance > farthest)
            {
                farthest = distance;
                index = i;
            }
        }

        if (index >= 0 && farthest > epsilon)
        {
            simplify(points, first, index, epsilon, kept);
            kept << index;
            simplify(points, index, last, epsilon, kept);
        }
    }

    qreal pathLength(const QList<QPointF>& points)
    {
        qreal length = 0;

        for (int i = 1; i < points.size(); ++i)
        {
            length += QLineF(points.at(i - 1), points.at(i)).length();
        }

        return length;
    }

    // A line close to horizontal or vertical becomes exactly so.
    QPointF straighten(const QPointF& start, QPointF end)
    {
        const qreal angle = QLineF(start, end).angle();
        const qreal tolerance = 6;

        for (qreal axis : { 0.0, 90.0, 180.0, 270.0, 360.0 })
        {
            if (qAbs(angle - axis) < tolerance)
            {
                if (int(axis) % 180 == 0)
                {
                    end.setY(start.y());
                }
                else
                {
                    end.setX(start.x());
                }
            }
        }

        return end;
    }

    // How far the points are, on average, from the ellipse that fills the rectangle,
    // as a part of its size: 0 on the ellipse.
    qreal ellipseError(const QList<QPointF>& points, const QRectF& box)
    {
        const QPointF center = box.center();
        const qreal rx = box.width() / 2;
        const qreal ry = box.height() / 2;
        qreal error = 0;

        for (const QPointF& p : points)
        {
            const qreal dx = (p.x() - center.x()) / rx;
            const qreal dy = (p.y() - center.y()) / ry;
            error += qAbs(qSqrt(dx * dx + dy * dy) - 1);
        }

        return error / points.size();
    }

    // How far the points are, on average, from the sides of the rectangle, as a part of its diagonal.
    qreal rectangleError(const QList<QPointF>& points, const QRectF& box)
    {
        const qreal diagonal = QLineF(box.topLeft(), box.bottomRight()).length();
        qreal error = 0;

        for (const QPointF& p : points)
        {
            const qreal distance = qMin(qMin(qAbs(p.x() - box.left()), qAbs(p.x() - box.right())),
                                        qMin(qAbs(p.y() - box.top()), qAbs(p.y() - box.bottom())));
            error += distance;
        }

        return error / points.size() / diagonal;
    }
}

UBShapeRecognizer::Shape UBShapeRecognizer::recognize(const QList<QPointF>& points)
{
    Shape shape;

    if (points.size() < 5)
    {
        return shape;
    }

    const QRectF box = QPolygonF(points.toVector()).boundingRect();
    const qreal diagonal = QLineF(box.topLeft(), box.bottomRight()).length();
    const qreal length = pathLength(points);
    const QPointF start = points.first();
    const QPointF end = points.last();
    const qreal gap = QLineF(start, end).length();

    // Too small to be meant as a shape: a dot, a letter, a tick.
    if (diagonal < 20 || length <= 0)
    {
        return shape;
    }

    // A straight line: every point near the line from the start to the end.
    qreal farthest = 0;

    for (const QPointF& p : points)
    {
        farthest = qMax(farthest, distanceToSegment(p, start, end));
    }

    if (gap > 0.85 * length && farthest < qMax<qreal>(4, 0.05 * gap))
    {
        shape.points = { start, straighten(start, end) };
        return shape;
    }

    // The other shapes are closed: the line ends near where it started.
    if (gap > 0.22 * length || gap > 0.3 * diagonal)
    {
        return shape;
    }

    // A circle or an oval.
    if (ellipseError(points, box) < 0.085)
    {
        QRectF ellipse = box;
        const qreal ratio = box.width() / box.height();

        // Nearly round: a circle.
        if (ratio > 0.85 && ratio < 1.18)
        {
            const qreal radius = (box.width() + box.height()) / 4;
            ellipse = QRectF(box.center() - QPointF(radius, radius), QSizeF(2 * radius, 2 * radius));
        }

        const int steps = 72;

        for (int i = 0; i <= steps; ++i)
        {
            const qreal a = 2 * M_PI * i / steps;
            shape.points << QPointF(ellipse.center().x() + ellipse.width() / 2 * qCos(a),
                                    ellipse.center().y() + ellipse.height() / 2 * qSin(a));
        }

        shape.closed = true;
        shape.smooth = true;
        return shape;
    }

    // A square or a rectangle, with its sides along the page.
    if (rectangleError(points, box) < 0.035)
    {
        QRectF rectangle = box;
        const qreal ratio = box.width() / box.height();

        if (ratio > 0.88 && ratio < 1.14)
        {
            const qreal side = (box.width() + box.height()) / 2;
            rectangle = QRectF(box.center() - QPointF(side / 2, side / 2), QSizeF(side, side));
        }

        shape.points = { rectangle.topLeft(), rectangle.topRight(), rectangle.bottomRight(), rectangle.bottomLeft(), rectangle.topLeft() };
        shape.closed = true;
        return shape;
    }

    // A triangle or another polygon of few sides: the corners of the line.
    QList<QPointF> loop = points;
    loop << start;
    QList<int> kept{ 0 };
    simplify(loop, 0, loop.size() - 1, 0.07 * diagonal, kept);
    kept << loop.size() - 1;

    QList<QPointF> corners;

    for (int index : kept)
    {
        corners << loop.at(index);
    }

    // The end comes back to the start: they are one corner.
    corners.removeLast();

    // A corner on the way of a side, not a real one, goes away.
    for (int i = 0; corners.size() > 3 && i < corners.size();)
    {
        const QPointF before = corners.at((i + corners.size() - 1) % corners.size());
        const QPointF after = corners.at((i + 1) % corners.size());

        if (distanceToSegment(corners.at(i), before, after) < 0.08 * diagonal)
        {
            corners.removeAt(i);
        }
        else
        {
            ++i;
        }
    }

    if (corners.size() < 3 || corners.size() > 6)
    {
        return shape;
    }

    // Every point must stay near a side, or the drawing was not a polygon.
    qreal error = 0;

    for (const QPointF& p : points)
    {
        qreal nearest = 1e9;

        for (int i = 0; i < corners.size(); ++i)
        {
            nearest = qMin(nearest, distanceToSegment(p, corners.at(i), corners.at((i + 1) % corners.size())));
        }

        error += nearest;
    }

    if (error / points.size() > 0.04 * diagonal)
    {
        return shape;
    }

    shape.points = corners;
    shape.points << corners.first();
    shape.closed = true;
    return shape;
}
