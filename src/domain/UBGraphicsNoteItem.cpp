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

// The bookmark, in pixels on screen, with the clicked point at the middle of its top.
static constexpr qreal sHalfWidth = 11;
static constexpr qreal sHeight = 30;
static constexpr qreal sNotch = 8;

UBGraphicsNoteItem::UBGraphicsNoteItem(const QString& text, const QColor& color, QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Object));
    // Above the page and the strokes, so that the marker is always seen.
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::DrawingItem));
    setText(text);
    setColor(color.isValid() ? color : colors().first());
}

QList<QColor> UBGraphicsNoteItem::colors()
{
    return { QColor("#f5b400"), QColor("#e53935"), QColor("#fb8c00"), QColor("#43a047"),
             QColor("#1e88e5"), QColor("#8e24aa"), QColor("#757575") };
}

QColor UBGraphicsNoteItem::color() const
{
    return mColor;
}

void UBGraphicsNoteItem::setColor(const QColor& color)
{
    mColor = color;
    update();
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
    UBNoteDialog dialog(mText, mColor, false, UBApplication::mainWindow);

    switch (dialog.ask())
    {
        case UBNoteDialog::Result::Saved:
            setText(dialog.text());
            setColor(dialog.color());
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
    return QRectF(-sHalfWidth - 2, -2, 2 * sHalfWidth + 4, sHeight + 4);
}

void UBGraphicsNoteItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing);

    // On screen the marker keeps its size in pixels. In a PDF or a picture
    // it keeps its size relative to the page, as it looks on the board.
    auto boardScene = dynamic_cast<UBGraphicsScene*>(scene());

    if (boardScene && boardScene->renderingContext() != UBGraphicsScene::Screen && painter->device())
    {
        painter->save();
        const qreal factor = painter->device()->width() / 600.0;
        painter->scale(factor, factor);
    }

    // A bookmark ribbon with a notch at the bottom.
    QPainterPath ribbon;
    ribbon.moveTo(-sHalfWidth, 0);
    ribbon.lineTo(sHalfWidth, 0);
    ribbon.lineTo(sHalfWidth, sHeight);
    ribbon.lineTo(0, sHeight - sNotch);
    ribbon.lineTo(-sHalfWidth, sHeight);
    ribbon.closeSubpath();

    painter->setPen(QPen(mColor.darker(160), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(mColor);
    painter->drawPath(ribbon);

    // Two lines of text, so that it reads as a note.
    painter->setPen(QPen(QColor(255, 255, 255, 220), 1.6, Qt::SolidLine, Qt::RoundCap));
    painter->drawLine(QPointF(-5, 7), QPointF(5, 7));
    painter->drawLine(QPointF(-5, 12), QPointF(3, 12));

    if (boardScene && boardScene->renderingContext() != UBGraphicsScene::Screen && painter->device())
    {
        painter->restore();
    }
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
