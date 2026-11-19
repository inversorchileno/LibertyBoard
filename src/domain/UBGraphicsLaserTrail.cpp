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

#include "UBGraphicsLaserTrail.h"

#include <algorithm>

#include <QPainter>
#include <QPen>

#include "core/UBSettings.h"

#include <cmath>

// The glow around the trail is wider than the trail itself.
static constexpr qreal sGlowWidthFactor = 3.2;
static constexpr int sFrameIntervalMs = 16;

UBGraphicsLaserTrail::UBGraphicsLaserTrail(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    mClock.start();
    mFrameTimer.setInterval(sFrameIntervalMs);
    connect(&mFrameTimer, &QTimer::timeout, this, &UBGraphicsLaserTrail::onFrame);
}

void UBGraphicsLaserTrail::startTrail(const QPointF& point, qreal width)
{
    UBSettings* settings = UBSettings::settings();

    Trail trail;
    trail.points << point;
    trail.width = width;
    trail.color = QColor(settings->laserColor->get().toString());
    trail.fadeDelay = qRound64(settings->laserFadeDelay->get().toDouble() * 1000);
    trail.fadeDuration = qRound64(settings->laserFadeDuration->get().toDouble() * 1000);
    trail.opacity = qBound(0.0, settings->laserOpacity->get().toDouble(), 1.0);
    trail.pointOnly = settings->laserMode->get().toInt() == UBLaserMode::Point;
    trail.whiteBurn = settings->laserEffect->get().toInt() == UBLaserEffect::WhiteBurn;
    trail.rainbow = settings->laserRainbow->get().toBool();

    if (!trail.color.isValid())
    {
        trail.color = Qt::red;
    }

    mTrails << trail;
    updateBounds();
    update();

    if (!mFrameTimer.isActive())
    {
        mFrameTimer.start();
    }
}

void UBGraphicsLaserTrail::extendTrail(const QPointF& point)
{
    if (mTrails.isEmpty() || mTrails.last().finishedAt >= 0)
    {
        return;
    }

    Trail& trail = mTrails.last();

    if (trail.points.last() == point)
    {
        return;
    }

    if (trail.pointOnly)
    {
        // Only the position of the pen is shown: the old point is erased.
        update(boundsOf(trail));
        trail.points.clear();
    }

    trail.points << point;

    const qreal margin = marginOf(trail);
    const QRectF pointBounds(point.x() - margin, point.y() - margin, 2 * margin, 2 * margin);

    if (!mBounds.contains(pointBounds))
    {
        prepareGeometryChange();
        mBounds = mBounds.united(pointBounds);
    }

    update(pointBounds.united(QRectF(trail.points.at(trail.points.size() - 2), point).normalized()
                                  .adjusted(-margin, -margin, margin, margin)));
}

void UBGraphicsLaserTrail::finishTrail()
{
    if (!mTrails.isEmpty() && mTrails.last().finishedAt < 0)
    {
        mTrails.last().finishedAt = mClock.elapsed();
    }
}

QRectF UBGraphicsLaserTrail::boundingRect() const
{
    return mBounds;
}

void UBGraphicsLaserTrail::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const qint64 now = mClock.elapsed();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);

    for (const Trail& trail : std::as_const(mTrails))
    {
        const qreal opacity = opacityOf(trail, now) * trail.opacity;

        if (opacity <= 0)
        {
            continue;
        }

        if (trail.rainbow)
        {
            paintRainbow(painter, trail, opacity, now);
            continue;
        }

        const QPainterPath path = smoothPath(trail.points);

        // From the outside in: a soft wide glow, a stronger inner glow,
        // the colored trail and a light core that makes it look bright.
        struct Layer
        {
            qreal widthFactor;
            qreal alpha;
            QColor color;
        };

        // A point is drawn wider than a trail so that it is easy to follow.
        const qreal width = trail.pointOnly ? trail.width * 2 : trail.width;

        const Layer standardLayers[] = {
            { sGlowWidthFactor, 0.15, trail.color },
            { 2.0, 0.35, trail.color },
            { 1.0, 1.0, trail.color },
            { 0.4, 0.85, trail.color.lighter(170) },
        };

        // White burn: a white trail inside a wide colored halo.
        const Layer whiteBurnLayers[] = {
            { sGlowWidthFactor, 0.25, trail.color },
            { 2.0, 0.6, trail.color },
            { 1.0, 1.0, QColor(Qt::white) },
        };

        const auto drawLayer = [&](const Layer& layer) {
            QColor color = layer.color;
            color.setAlphaF(layer.alpha * opacity);

            QPen pen(color, width * layer.widthFactor, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);
            painter->drawPath(path);
        };

        if (trail.whiteBurn)
        {
            for (const Layer& layer : whiteBurnLayers)
            {
                drawLayer(layer);
            }

            continue;
        }

        for (const Layer& layer : standardLayers)
        {
            drawLayer(layer);
        }
    }
}

void UBGraphicsLaserTrail::paintRainbow(QPainter* painter, const Trail& trail, qreal opacity, qint64 now) const
{
    // The colors of the rainbow follow one another along the trail; a point
    // goes through them with time.
    const qreal width = trail.pointOnly ? trail.width * 2 : trail.width;
    const qreal cycle = qMax(900.0, width * 100);
    const qreal timeShift = trail.pointOnly ? qreal(now % 2000) / 2000 : 0;

    auto colorAt = [cycle, timeShift](qreal distance, qreal lightness) {
        qreal hue = std::fmod(distance / cycle + timeShift, 1.0);
        return QColor::fromHslF(hue, 0.85, lightness);
    };

    struct Layer
    {
        qreal widthFactor;
        qreal alpha;
        qreal lightness;
    };

    const Layer layers[] = {
        { sGlowWidthFactor, 0.15, 0.6 },
        { 2.0, 0.35, 0.6 },
        { 1.0, 1.0, 0.62 },
        { 0.4, 0.85, 0.82 },
    };

    // Pieces of a few widths, each one of a single color: their flat ends
    // meet without covering each other, so the glow keeps its transparency.
    const qreal pieceLength = qMax(width * 2, 8.0);
    // Along the same smooth curve as a trail of one color.
    QPolygonF points;

    for (const QPolygonF& part : smoothPath(trail.points).toSubpathPolygons())
    {
        points << part;
    }

    if (points.isEmpty())
    {
        return;
    }

    // The trail cut into pieces of that length, long segments included, so
    // that the color changes smoothly.
    QList<QPolygonF> pieces;
    QList<qreal> pieceMiddles;
    QPolygonF piece{ points.first() };
    qreal distance = 0;
    qreal pieceStart = 0;

    for (int i = 1; i < points.size(); ++i)
    {
        QPointF from = points.at(i - 1);
        const QPointF to = points.at(i);
        qreal remaining = QLineF(from, to).length();

        while (distance + remaining - pieceStart >= pieceLength)
        {
            const qreal step = pieceLength - (distance - pieceStart);
            const QPointF cut = from + (to - from) * (step / remaining);
            piece << cut;
            pieces << piece;
            pieceMiddles << pieceStart + pieceLength / 2;

            distance += step;
            remaining -= step;
            pieceStart = distance;
            from = cut;
            piece = QPolygonF{ cut };
        }

        distance += remaining;
        piece << to;
    }

    if (piece.size() > 1 || pieces.isEmpty())
    {
        pieces << piece;
        pieceMiddles << (pieceStart + distance) / 2;
    }

    for (const Layer& layer : layers)
    {
        for (int i = 0; i < pieces.size(); ++i)
        {
            QColor color = colorAt(pieceMiddles.at(i), layer.lightness);
            color.setAlphaF(layer.alpha * opacity);

            // The two ends of the trail are round, like the trail of one color.
            const bool end = (i == 0 || i == pieces.size() - 1);
            painter->setPen(QPen(color, width * layer.widthFactor, Qt::SolidLine, end ? Qt::RoundCap : Qt::FlatCap, Qt::RoundJoin));

            if (pieces.at(i).size() == 1)
            {
                painter->drawPoint(pieces.at(i).first());
            }
            else
            {
                painter->drawPolyline(pieces.at(i));
            }
        }
    }
}

void UBGraphicsLaserTrail::onFrame()
{
    const qint64 now = mClock.elapsed();
    const int countBefore = mTrails.size();

    mTrails.erase(std::remove_if(mTrails.begin(), mTrails.end(), [this, now](const Trail& trail) {
        return trail.finishedAt >= 0 && opacityOf(trail, now) <= 0;
    }), mTrails.end());

    if (mTrails.size() != countBefore)
    {
        updateBounds();
    }

    update();

    if (mTrails.isEmpty())
    {
        mFrameTimer.stop();
    }
}

qreal UBGraphicsLaserTrail::opacityOf(const Trail& trail, qint64 now) const
{
    if (trail.finishedAt < 0)
    {
        return 1.0;
    }

    const qint64 fadingFor = now - trail.finishedAt - trail.fadeDelay;

    if (fadingFor <= 0)
    {
        return 1.0;
    }

    if (trail.fadeDuration <= 0)
    {
        return 0.0;
    }

    return qMax(0.0, 1.0 - qreal(fadingFor) / trail.fadeDuration);
}

void UBGraphicsLaserTrail::updateBounds()
{
    QRectF bounds;

    for (const Trail& trail : std::as_const(mTrails))
    {
        bounds = bounds.united(boundsOf(trail));
    }

    prepareGeometryChange();
    mBounds = bounds;
}

qreal UBGraphicsLaserTrail::marginOf(const Trail& trail)
{
    const qreal width = trail.pointOnly ? trail.width * 2 : trail.width;
    return width * sGlowWidthFactor / 2 + 2;
}

QRectF UBGraphicsLaserTrail::boundsOf(const Trail& trail)
{
    const qreal margin = marginOf(trail);
    QRectF bounds;

    for (const QPointF& point : trail.points)
    {
        bounds = bounds.united(QRectF(point.x() - margin, point.y() - margin, 2 * margin, 2 * margin));
    }

    return bounds;
}

QPainterPath UBGraphicsLaserTrail::smoothPath(const QVector<QPointF>& points)
{
    QPainterPath path;

    if (points.isEmpty())
    {
        return path;
    }

    path.moveTo(points.first());

    if (points.size() == 1)
    {
        // A single tap still shows a dot.
        path.lineTo(points.first() + QPointF(0.01, 0));
        return path;
    }

    // Curves through the middle of each segment hide the corners of a
    // trail drawn quickly.
    for (int i = 1; i < points.size() - 1; ++i)
    {
        const QPointF middle = (points.at(i) + points.at(i + 1)) / 2;
        path.quadTo(points.at(i), middle);
    }

    path.lineTo(points.last());
    return path;
}
