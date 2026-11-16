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
