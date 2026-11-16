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

#include "UBGraphicsNoteItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>

#include "core/UBApplication.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBMainWindow.h"
#include "gui/UBNoteDialog.h"

static constexpr qreal sRadius = 15;

UBGraphicsNoteItem::UBGraphicsNoteItem(const QString& text, QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Object));
    setText(text);
}

QString UBGraphicsNoteItem::text() const
{
    return mText;
}

void UBGraphicsNoteItem::setText(const QString& text)
{
    mText = text;
    setToolTip(text);
}

void UBGraphicsNoteItem::edit()
{
    UBNoteDialog dialog(mText, false, UBApplication::mainWindow);

    switch (dialog.ask())
    {
        case UBNoteDialog::Result::Saved:
            setText(dialog.text());
            markDocumentChanged();
            break;

        case UBNoteDialog::Result::Deleted:
            markDocumentChanged();
            scene()->removeItem(this);
            deleteLater();
            break;

        default:
            break;
    }
}

QRectF UBGraphicsNoteItem::boundingRect() const
{
    return QRectF(-sRadius - 1, -sRadius - 1, 2 * sRadius + 2, 2 * sRadius + 2);
}

void UBGraphicsNoteItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing);

    // A teal disc with a white sticky note on it.
    painter->setPen(QPen(QColor(255, 255, 255, 200), 1.5));
    painter->setBrush(QColor("#1aa5a0"));
    painter->drawEllipse(QPointF(0, 0), sRadius, sRadius);

    QPainterPath sheet;
    sheet.moveTo(-7, -7);
    sheet.lineTo(7, -7);
    sheet.lineTo(7, 3);
    sheet.lineTo(3, 7);
    sheet.lineTo(-7, 7);
    sheet.closeSubpath();

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawPath(sheet);

    painter->setPen(QPen(QColor("#1aa5a0"), 1.4));
    painter->drawLine(QPointF(-4, -3), QPointF(4, -3));
    painter->drawLine(QPointF(-4, 1), QPointF(2, 1));
}

void UBGraphicsNoteItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    mPressPosition = event->screenPos();
    QGraphicsObject::mousePressEvent(event);
}

void UBGraphicsNoteItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mouseReleaseEvent(event);

    const bool clicked = QLineF(mPressPosition, event->screenPos()).length() < 4;

    if (clicked)
    {
        // After the mouse event: the dialog runs its own event loop.
        QTimer::singleShot(0, this, &UBGraphicsNoteItem::edit);
    }
    else
    {
        markDocumentChanged();
    }
}

void UBGraphicsNoteItem::markDocumentChanged()
{
    if (auto boardScene = dynamic_cast<UBGraphicsScene*>(scene()))
    {
        boardScene->setModified(true);
    }
}
