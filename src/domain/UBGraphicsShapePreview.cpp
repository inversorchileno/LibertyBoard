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

#include "UBGraphicsShapePreview.h"

#include <QPainter>

UBGraphicsShapePreview::UBGraphicsShapePreview(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setAcceptedMouseButtons(Qt::NoButton);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void UBGraphicsShapePreview::setParts(const QList<UBShapePart>& parts)
{
    QRectF bounds;

    for (const UBShapePart& part : parts)
    {
        bounds = bounds.united(part.polygon.boundingRect());
    }

    prepareGeometryChange();
    mParts = parts;
    mBounds = bounds.adjusted(-1, -1, 1, 1);
    update();
}

void UBGraphicsShapePreview::clear()
{
    setParts({});
}

QRectF UBGraphicsShapePreview::boundingRect() const
{
    return mBounds;
}

void UBGraphicsShapePreview::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    for (const UBShapePart& part : std::as_const(mParts))
    {
        painter->setBrush(part.color);
        painter->drawPolygon(part.polygon, Qt::OddEvenFill);
    }
}
