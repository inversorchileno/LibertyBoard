/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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




#include "UBGraphicsStroke.h"

#include <algorithm>
#include <cmath>

#include <QPainterPath>
#include <QRandomGenerator>
#include <QtMath>

#include "UBGraphicsPolygonItem.h"

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/memcheck.h"
#include "domain/UBGraphicsScene.h"

#include "frameworks/UBGeometryUtils.h"


typedef QPair<QPointF, qreal> strokePoint;

UBGraphicsStroke::UBGraphicsStroke(std::shared_ptr<UBGraphicsScene> scene)
    :mScene(scene)
{
    mAntiScaleRatio = 1./(UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
}


UBGraphicsStroke::~UBGraphicsStroke()
{
    foreach(UBGraphicsPolygonItem* poly, mPolygons)
        poly->setStroke(NULL);

    mPolygons.clear();
}


void UBGraphicsStroke::addPolygon(UBGraphicsPolygonItem* pol)
{
    remove(pol);
    mPolygons << pol;
}

void UBGraphicsStroke::remove(UBGraphicsPolygonItem* polygonItem)
{
    int n = mPolygons.indexOf(polygonItem);
    if (n>=0)
        mPolygons.removeAt(n);
}

QList<UBGraphicsPolygonItem*> UBGraphicsStroke::polygons() const
{
    return mPolygons;
}

/**
 * @brief Add a point to the curve, interpolating extra points if required
 * @param point The position of the point to add
 * @param width The width of the stroke at that point.
 * @param interpolate If true, a Bézier curve will be drawn rather than a straight line
 * @return A list containing the last point drawn plus the point(s) that were added
 *
 * This method should be called when a new point is given by the input method (mouse, pen or other), and the points that are returned
 * should be used to draw the actual stroke on-screen. This is because if interpolation (bézier curves) are to be used, the points to draw
 * do not correspond to the points that were given by the input method.
 *
 */
QList<QPair<QPointF, qreal> > UBGraphicsStroke::addPoint(const QPointF& point, qreal width, bool interpolate)
{
    strokePoint newPoint(point, width);

    int n = mReceivedPoints.size();

    if (n == 0) {
        mReceivedPoints << newPoint;
        mDrawnPoints << newPoint;
        return QList<strokePoint>() << newPoint;
    }

    if (!interpolate) {
        strokePoint lastPoint = mReceivedPoints.last();
        mReceivedPoints << newPoint;
        mDrawnPoints << newPoint;
        return QList<strokePoint>() << lastPoint << newPoint;
    }

    else {
        // The curve we are interpolating is not between two drawn points;
        // it is between the midway points of the second-to-last and last point, and last and current point.


        // The first segment is just a straight line to the first midway point
        if (n == 1) {
            QPointF lastPoint = mReceivedPoints[0].first;
            qreal lastWidth = mReceivedPoints[0].second;
            strokePoint p(((lastPoint+point)/2.0), (lastWidth+width)/2.0);
            mReceivedPoints << newPoint;
            mDrawnPoints << p;

            return QList<strokePoint>() << mReceivedPoints[0] << p;
        }

        QPointF p0 = mReceivedPoints[mReceivedPoints.size() - 2].first;
        QPointF p1 = mReceivedPoints[mReceivedPoints.size() - 1].first;
        QPointF p2 = point;

        QPointF startPoint = (p1+p0)/2.0;
        QPointF endPoint = (p2+p1)/2.0;

        QList<QPointF> calculated = UBGeometryUtils::quadraticBezier(startPoint, p1, endPoint, 10);
        QList<strokePoint> newPoints;

        qreal startWidth = mDrawnPoints.last().second;

        for (int i(0); i < calculated.size(); ++i) {
            qreal w = startWidth + (qreal(i)/qreal(calculated.size()-1)) * (width - startWidth);
            newPoints << strokePoint(calculated[i], w);
        }

        // avoid adding duplicates
        if (newPoints.first().first == mDrawnPoints.last().first)
            mDrawnPoints.removeLast();

        foreach(strokePoint p, newPoints)
            mDrawnPoints << p;

        mReceivedPoints << strokePoint(point, width);
        return newPoints;
    }

    return QList<strokePoint>();
}

bool UBGraphicsStroke::hasPressure()
{
    if (mPolygons.count() > 2)
    {
        qreal nominalWidth = mPolygons.at(0)->originalWidth();

        foreach(UBGraphicsPolygonItem* pol, mPolygons)
        {
            if (!pol->isNominalLine() || pol->originalWidth() != nominalWidth)
                return true;
        }
        return false;
    }
    else
        return true;
}


UBGraphicsStroke* UBGraphicsStroke::deepCopy()
{
    UBGraphicsStroke* clone = new UBGraphicsStroke();

    return clone;
}

bool UBGraphicsStroke::hasAlpha() const
{
    if (mPolygons.length() > 0 && mPolygons.at(0)->color().alphaF() != 1.0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void UBGraphicsStroke::clear()
{
    if(!mPolygons.empty()){
        mPolygons.clear();
    }
}

static QPolygonF convexHull(QList<QPointF> points)
{
    std::sort(points.begin(), points.end(), [](const QPointF& a, const QPointF& b) {
        return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());
    });

    auto cross = [](const QPointF& o, const QPointF& a, const QPointF& b) {
        return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x());
    };

    QList<QPointF> hull;

    for (int pass = 0; pass < 2; ++pass)
    {
        const int start = hull.size();

        for (const QPointF& point : std::as_const(points))
        {
            while (hull.size() >= start + 2 && cross(hull.at(hull.size() - 2), hull.last(), point) <= 0)
            {
                hull.removeLast();
            }

            hull << point;
        }

        hull.removeLast();
        std::reverse(points.begin(), points.end());
    }

    return QPolygonF(hull);
}

UBGraphicsStroke* UBGraphicsStroke::redrawnWithChiselTip(std::shared_ptr<UBGraphicsScene> scene, const QPair<QPointF, qreal>& lastPosition,
                                                         qreal nibAngle, qreal nibThickness)
{
    if (mDrawnPoints.isEmpty() || !scene)
    {
        return nullptr;
    }

    // The stroke goes on to where the pen was lifted.
    QList<strokePoint> points = mDrawnPoints;

    if (lastPosition.second > 0 && points.last().first != lastPosition.first)
    {
        points << lastPosition;
    }

    // The tip is a flat rectangle as long as the width of the stroke,
    // tilted like a nib or a highlighter held in the right hand.
    const qreal angle = qDegreesToRadians(nibAngle);
    const QPointF along(std::cos(angle), std::sin(angle));
    const QPointF across(-along.y(), along.x());

    auto tipCorners = [&](const QPointF& center, qreal width) {
        const QPointF length = along * width / 2;
        const QPointF thickness = across * width * nibThickness;
        return QList<QPointF>{ center + length + thickness, center + length - thickness,
                               center - length - thickness, center - length + thickness };
    };

    QPainterPath outline;
    outline.setFillRule(Qt::WindingFill);

    if (points.size() == 1)
    {
        outline.addPolygon(convexHull(tipCorners(points.first().first, points.first().second)));
        outline.closeSubpath();
    }

    // The tip swept from each point to the next one.
    for (int i = 1; i < points.size(); ++i)
    {
        const strokePoint& from = points.at(i - 1);
        const strokePoint& to = points.at(i);
        outline.addPolygon(convexHull(tipCorners(from.first, from.second) + tipCorners(to.first, to.second)));
        outline.closeSubpath();
    }

    UBGraphicsStroke* newStroke = new UBGraphicsStroke(scene);
    newStroke->mDrawnPoints = points;

    // One outline without overlaps keeps the transparency even.
    for (const QPolygonF& polygon : outline.simplified().toFillPolygons())
    {
        if (polygon.size() > 2)
        {
            UBGraphicsPolygonItem* poly = scene->polygonToPolygonItem(polygon);
            poly->setFillRule(Qt::WindingFill);
            poly->setStroke(newStroke);
            newStroke->mPolygons << poly;
        }
    }

    if (newStroke->mPolygons.isEmpty())
    {
        delete newStroke;
        return nullptr;
    }

    return newStroke;
}

UBGraphicsStroke* UBGraphicsStroke::redrawnTextured(std::shared_ptr<UBGraphicsScene> scene, const QPair<QPointF, qreal>& lastPosition, bool pencil)
{
    if (mDrawnPoints.isEmpty() || !scene)
    {
        return nullptr;
    }

    QList<strokePoint> drawn = mDrawnPoints;

    if (lastPosition.second > 0 && drawn.last().first != lastPosition.first)
    {
        drawn << lastPosition;
    }

    // Points close together, so that the grain is finer than the points
    // the pen gave.
    qreal meanWidth = 0;

    for (const strokePoint& point : std::as_const(drawn))
    {
        meanWidth += point.second;
    }

    meanWidth /= drawn.size();
    const qreal step = qMax(meanWidth * 0.35, 0.5);
    QList<strokePoint> points{ drawn.first() };

    for (int i = 1; i < drawn.size(); ++i)
    {
        const strokePoint& from = drawn.at(i - 1);
        const strokePoint& to = drawn.at(i);
        const int pieces = qMax(1, int(QLineF(from.first, to.first).length() / step));

        for (int k = 1; k <= pieces; ++k)
        {
            const qreal t = qreal(k) / pieces;
            points << strokePoint(from.first + (to.first - from.first) * t, from.second + (to.second - from.second) * t);
        }
    }

    if (points.size() == 1)
    {
        points << strokePoint(points.first().first + QPointF(0.01, 0), points.first().second);
    }

    struct Strand
    {
        qreal offset;   // across the stroke, in widths
        qreal width;    // in widths
        qreal alpha;
    };

    // A pencil: a few thin strands; a marker: a body and rough strands around it.
    const QList<Strand> strands = pencil
            ? QList<Strand>{ { 0, 0.7, 0.5 }, { -0.25, 0.45, 0.45 }, { 0.25, 0.45, 0.45 }, { 0.08, 0.35, 0.4 } }
            : QList<Strand>{ { 0, 0.75, 0.6 }, { -0.3, 0.45, 0.4 }, { 0.3, 0.45, 0.4 }, { -0.12, 0.5, 0.35 },
                             { 0.15, 0.5, 0.35 }, { 0, 1.0, 0.25 } };
    const qreal roughness = pencil ? 0.12 : 0.1;

    // The same grain each time the same stroke is drawn again.
    QRandomGenerator random(quint32(qHash(qRound(points.first().first.x() * 10)) ^ quint32(points.size())));

    UBGraphicsStroke* newStroke = new UBGraphicsStroke(scene);
    newStroke->mDrawnPoints = drawn;

    for (const Strand& strand : strands)
    {
        QPolygonF left;
        QPolygonF right;

        // Noise that changes slowly along the strand, with a little fine
        // grain on top: rough edges without spikes.
        qreal driftOffset = 0;
        qreal driftWidth = 0;

        for (int i = 0; i < points.size(); ++i)
        {
            const QPointF before = points.at(qMax(0, i - 1)).first;
            const QPointF after = points.at(qMin(points.size() - 1, i + 1)).first;
            QPointF direction = after - before;
            const qreal length = std::hypot(direction.x(), direction.y());
            direction = length > 0 ? direction / length : QPointF(1, 0);
            const QPointF normal(-direction.y(), direction.x());

            const qreal width = points.at(i).second;
            driftOffset = 0.85 * driftOffset + 0.15 * (random.generateDouble() * 2 - 1);
            driftWidth = 0.85 * driftWidth + 0.15 * (random.generateDouble() * 2 - 1);
            const qreal grain = (random.generateDouble() * 2 - 1) * 0.25;
            const qreal jitter = (driftOffset * 2 + grain) * roughness * width;
            const qreal half = strand.width * width / 2 * (1 + (driftWidth * 2 + grain) * roughness);
            const QPointF center = points.at(i).first + normal * (strand.offset * width + jitter);

            left << center + normal * half;
            right.prepend(center - normal * half);
        }

        UBGraphicsPolygonItem* poly = scene->polygonToPolygonItem(left + right);
        QColor dark = poly->colorOnDarkBackground();
        QColor light = poly->colorOnLightBackground();
        dark.setAlphaF(dark.alphaF() * strand.alpha);
        light.setAlphaF(light.alphaF() * strand.alpha);
        poly->setColorOnDarkBackground(dark);
        poly->setColorOnLightBackground(light);
        poly->setColor(scene->isDarkBackground() ? dark : light);
        poly->setFillRule(Qt::WindingFill);
        poly->setStroke(newStroke);
        newStroke->mPolygons << poly;
    }

    return newStroke;
}

/**
 * @brief Return a simplified version of the stroke, with less points and polygons.
 *
 */
UBGraphicsStroke* UBGraphicsStroke::redrawnAlong(std::shared_ptr<UBGraphicsScene> scene, const QList<QPointF>& outline, bool smooth)
{
    if (outline.size() < 2 || mDrawnPoints.isEmpty() || !scene)
    {
        return nullptr;
    }

    qreal width = 0;

    for (const strokePoint& point : std::as_const(mDrawnPoints))
    {
        width += point.second;
    }

    width /= mDrawnPoints.size();

    UBGraphicsStroke* newStroke = new UBGraphicsStroke(scene);

    for (const QPointF& point : outline)
    {
        newStroke->mDrawnPoints << strokePoint(point, width);
    }

    // A round shape is one curve; a shape of straight sides has one piece
    // for each side, with round ends that make its corners.
    QList<QList<strokePoint>> pieces;

    if (smooth)
    {
        pieces << newStroke->mDrawnPoints;
    }
    else
    {
        for (int i = 1; i < newStroke->mDrawnPoints.size(); ++i)
        {
            pieces << QList<strokePoint>{ newStroke->mDrawnPoints.at(i - 1), newStroke->mDrawnPoints.at(i) };
        }
    }

    QList<UBGraphicsPolygonItem*> newPolygons;

    for (const QList<strokePoint>& piece : std::as_const(pieces))
    {
        UBGraphicsPolygonItem* poly = scene->polygonToPolygonItem(UBGeometryUtils::curveToPolygon(piece, true, true));

        // Translucent pieces do not cover each other twice.
        if (!poly->brush().isOpaque())
        {
            for (UBGraphicsPolygonItem* previous : std::as_const(newPolygons))
            {
                poly->subtract(previous);
            }
        }

        newPolygons << poly;
    }

    newStroke->mPolygons = newPolygons;

    for (UBGraphicsPolygonItem* poly : std::as_const(newStroke->mPolygons))
    {
        poly->setFillRule(Qt::WindingFill);
        poly->setStroke(newStroke);
    }

    return newStroke;
}

UBGraphicsStroke* UBGraphicsStroke::simplify()
{
    if (mDrawnPoints.size() < 3)
        return NULL;

    UBGraphicsStroke* newStroke = new UBGraphicsStroke();
    newStroke->mDrawnPoints = QList<strokePoint>(mDrawnPoints);

    QList<strokePoint>& points = newStroke->mDrawnPoints;
    //qDebug() << "Simplifying. Before: " << points.size() << " points and " << polygons().size() << " polygons";

    /* Basic simplifying algorithm: consider A, B and C the current point and the two following ones.
     * If the angle between (AB) and (BC) is lower than a certain threshold,
     * the three points are considered to be aligned and the middle one (B) is removed.
     *
     * We then consider the two following points as the new B and C while keeping the same A, and
     * test these three points. As long as they are aligned, B is erased and we start over.
     * If not, the current B becomes the new A, and so on.
     *
     * In case the stroke thickness varies a lot between A and B, then B is not removed even if it
     * should be according to the previous criteria.
     *
     * TODO: more advanced algorithm that could also simplify curved sections of the stroke
     */

    // angle difference in degrees between AB and BC below which the segments are considered colinear
    qreal thresholdAngle = UBSettings::settings()->boardSimplifyPenStrokesThresholdAngle->get().toReal();

    // Relative difference in thickness between two consecutive points (A and B) below which they are considered equal
    qreal thresholdWidthDifference = UBSettings::settings()->boardSimplifyPenStrokesThresholdWidthDifference->get().toReal();

    QList<strokePoint>::iterator it = points.begin();
    QList<QList<strokePoint>::iterator> toDelete;

    while (it+2 != points.end()) {
        // it, b_it and (b_it+1) correspond to A, B and C respectively
        QList<strokePoint>::iterator b_it(it+1);

        while (b_it+1 != points.end()) {
            qreal angle = qFabs(180-(UBGeometryUtils::angle(it->first, b_it->first, (b_it+1)->first)));
            qreal widthRatio = qMax(it->second, b_it->second)/qMin(it->second, b_it->second);

            if (angle < thresholdAngle && widthRatio < thresholdWidthDifference)
                b_it = points.erase(b_it);
            else
                break;
        }

        if (b_it+1 == points.end())
            break;
        else
            it = b_it;
    }

    // Next, we iterate over the new points to build the polygons that make up the stroke.
    // A new polygon is created every time drawCurve is true.

    QList<UBGraphicsPolygonItem*> newPolygons;
    QList<strokePoint> newStrokePoints;
    int i(0);

    auto scene = mScene.lock();

    while (i < points.size()) {
        bool drawCurve = false;

        newStrokePoints << points[i];

        // Limiting the size of the polygons, and creating new ones when there is a sharp angle between
        // consecutive point helps with two issues:
        // 1. When a polygon is transparent and it overlaps with itself, it is *sometimes* filled incorrectly.
        // 2. This way of simplifying strokes resuls in sharp, rather than rounded, corners when there is a sharp angle
        //    in the stroke

        if (newStrokePoints.size() > 1 && i < points.size() - 1) {
            qreal angle = qFabs(UBGeometryUtils::angle(points[i-1].first, points[i].first, points[i+1].first));
            if (angle < 150) // arbitrary threshold, change if necessary
                drawCurve = true;
        }

        if (hasAlpha() && newStrokePoints.size() % 20 == 0)
            drawCurve = true;

        if (drawCurve) {
            UBGraphicsPolygonItem* poly = scene->polygonToPolygonItem(UBGeometryUtils::curveToPolygon(newStrokePoints, true, true));
            //poly->setColor(QColor(rand()%256, rand()%256, rand()%256, poly->brush().color().alpha())); // useful for debugging

            // Subtract overlapping polygons if the stroke is translucent
            if (!poly->brush().isOpaque()) {
                foreach(UBGraphicsPolygonItem* prev, newPolygons)
                    poly->subtract(prev);
            }

            newPolygons << poly;
            newStrokePoints.clear();
            --i;
        }

        ++i;
    }

    if (newStrokePoints.size() > 0) {
        UBGraphicsPolygonItem* poly = scene->polygonToPolygonItem(UBGeometryUtils::curveToPolygon(newStrokePoints, true, true));

        if (!poly->brush().isOpaque()) {
            foreach(UBGraphicsPolygonItem* prev, newPolygons)
                poly->subtract(prev);
        }

        newPolygons << poly;
    }


    newStroke->mPolygons = QList<UBGraphicsPolygonItem*>(newPolygons);

    foreach(UBGraphicsPolygonItem* poly, newStroke->mPolygons) {
        poly->setFillRule(Qt::WindingFill);
        poly->setStroke(newStroke);
    }

    //qDebug() << "After: " << points.size() << " points and " << newStroke->polygons().size() << " polygons";

    return newStroke;
}
