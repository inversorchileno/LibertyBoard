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




#ifndef UBGRAPHICSSTROKE_H_
#define UBGRAPHICSSTROKE_H_

#include <QtGui>

#include "core/UB.h"



class UBGraphicsPolygonItem;
class UBGraphicsScene;

class UBGraphicsStroke
{
    friend class UBGraphicsPolygonItem;

    public:
        UBGraphicsStroke(std::shared_ptr<UBGraphicsScene> scene = NULL);
        virtual ~UBGraphicsStroke();

        bool hasPressure();

        QList<UBGraphicsPolygonItem*> polygons() const;

        void remove(UBGraphicsPolygonItem* polygonItem); 

        UBGraphicsStroke *deepCopy();

        bool hasAlpha() const;

        void clear();

        QList<QPair<QPointF, qreal> > addPoint(const QPointF& point, qreal width, bool interpolate = false);

        const QList<QPair<QPointF, qreal> >& points() { return mDrawnPoints; }

        UBGraphicsStroke* simplify();

        /**
         * @brief A new stroke along the given points, of the mean width of
         * this one: smooth for a round shape, with sharp corners otherwise.
         */
        /**
         * @brief A new stroke along the same points, drawn as by the slanted
         * flat tip of a highlighter: wide across a horizontal line, thin
         * along a steep one, with straight ends.
         */
        UBGraphicsStroke* redrawnWithChiselTip(std::shared_ptr<UBGraphicsScene> scene, const QPair<QPointF, qreal>& lastPosition,
                                               qreal nibAngle, qreal nibThickness);

        /**
         * @brief A new stroke along the same points, made of several rough
         * translucent strands: the grain of a pencil, thin and dark, or of a
         * felt marker, wide.
         */
        UBGraphicsStroke* redrawnTextured(std::shared_ptr<UBGraphicsScene> scene, const QPair<QPointF, qreal>& lastPosition, bool pencil);

        UBGraphicsStroke* redrawnAlong(std::shared_ptr<UBGraphicsScene> scene, const QList<QPointF>& outline, bool smooth);

    protected:
        void addPolygon(UBGraphicsPolygonItem* pol);

    private:

        std::weak_ptr<UBGraphicsScene> mScene;

        QList<UBGraphicsPolygonItem*> mPolygons;

        /// Points that were drawn by the user (i.e, actually received through input device)
        QList<QPair<QPointF, qreal> > mReceivedPoints;

        /// All the points (including interpolated) that are used to draw the stroke
        QList<QPair<QPointF, qreal> > mDrawnPoints;

        qreal mAntiScaleRatio;
};

#endif /* UBGRAPHICSSTROKE_H_ */
