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
#include <QElapsedTimer>
#include <QGraphicsObject>
#include <QList>
#include <QPainterPath>
#include <QTimer>
#include <QVector>

#include "core/UB.h"

/**
 * @brief Draws the trail of the laser pointer and lets it fade out.
 *
 * Each press of the laser pointer draws one trail. While the pen is down,
 * the trail stays fully visible. Once the pen is lifted, the trail waits
 * for the configured delay and then fades from fully visible to invisible
 * over the configured duration. The trail is never part of the document.
 */
class UBGraphicsLaserTrail : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit UBGraphicsLaserTrail(QGraphicsItem* parent = nullptr);

    void startTrail(const QPointF& point, qreal width);
    void extendTrail(const QPointF& point);
    void finishTrail();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    struct Trail
    {
        QVector<QPointF> points;
        qreal width{0};
        QColor color;
        qreal opacity{1};
        bool pointOnly{false};
        bool whiteBurn{false};
        bool rainbow{false};
        qint64 finishedAt{-1};
        qint64 fadeDelay{0};
        qint64 fadeDuration{0};
    };

    void onFrame();
    void paintRainbow(QPainter* painter, const Trail& trail, qreal opacity, qint64 now) const;
    qreal opacityOf(const Trail& trail, qint64 now) const;
    void updateBounds();
    static qreal marginOf(const Trail& trail);
    static QRectF boundsOf(const Trail& trail);
    static QPainterPath smoothPath(const QVector<QPointF>& points);

    QList<Trail> mTrails;
    QRectF mBounds;
    QElapsedTimer mClock;
    QTimer mFrameTimer;
};
