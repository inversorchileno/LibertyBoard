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


#include "UBGraphicsPageMarkItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QSvgRenderer>
#include <QTimer>

#include "core/UB.h"
#include "core/UBApplication.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBMainWindow.h"

// The icon, in pixels on screen, centered on the clicked point.
static constexpr qreal sHalfSize = 13;

UBGraphicsPageMarkItem::UBGraphicsPageMarkItem(const QString& iconFile, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , mIcon(new QSvgRenderer(iconFile, this))
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setCursor(Qt::PointingHandCursor);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Object));
    // Above the page and the strokes, so that the icon is always seen.
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::DrawingItem));
}

bool UBGraphicsPageMarkItem::isPageMark(const QGraphicsItem* item)
{
    const int type = item ? item->type() : 0;
    return type == UBGraphicsItemType::CommentItemType || type == UBGraphicsItemType::AttachmentItemType
           || type == UBGraphicsItemType::SoundItemType;
}

QRectF UBGraphicsPageMarkItem::boundingRect() const
{
    return QRectF(-sHalfSize - 3, -sHalfSize - 3, 2 * sHalfSize + 6, 2 * sHalfSize + 6);
}

void UBGraphicsPageMarkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // On screen the icon keeps its size in pixels. In a PDF or a picture it
    // keeps its size relative to the page, as it looks on the board.
    UBGraphicsScene* scene = boardScene();

    // A PDF gets the mark as an annotation of its own, which its readers show.
    if (scene && scene->renderingContext() == UBGraphicsScene::PdfExport)
    {
        painter->restore();
        return;
    }

    if (scene && scene->renderingContext() != UBGraphicsScene::Screen && painter->device())
    {
        const qreal factor = painter->device()->width() / 600.0;
        painter->scale(factor, factor);
    }

    const QRectF icon(-sHalfSize, -sHalfSize, 2 * sHalfSize, 2 * sHalfSize);

    if (mHighlighted)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 196, 0, 150));
        painter->drawEllipse(icon.adjusted(-3, -3, 3, 3));
    }

    mIcon->render(painter, icon);

    // Clicked: a dashed frame, as long as Delete would delete it.
    if (hasFocus() && (!scene || scene->renderingContext() == UBGraphicsScene::Screen))
    {
        painter->setPen(QPen(QColor(30, 110, 230), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(icon.adjusted(-2, -2, 2, 2));
    }

    painter->restore();
}

void UBGraphicsPageMarkItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    mPressPosition = event->screenPos();
    setFocus();
    QGraphicsObject::mousePressEvent(event);
}

void UBGraphicsPageMarkItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mouseReleaseEvent(event);

    if (QLineF(mPressPosition, event->screenPos()).length() < 4)
    {
        // After the mouse event: what opens may run its own event loop.
        QTimer::singleShot(0, this, &UBGraphicsPageMarkItem::open);
    }
    else
    {
        markDocumentChanged();
    }
}

void UBGraphicsPageMarkItem::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        event->accept();
        QTimer::singleShot(0, this, &UBGraphicsPageMarkItem::deleteFromPage);
        return;
    }

    QGraphicsObject::keyPressEvent(event);
}

UBGraphicsScene* UBGraphicsPageMarkItem::boardScene() const
{
    return dynamic_cast<UBGraphicsScene*>(scene());
}

QString UBGraphicsPageMarkItem::documentFolder() const
{
    UBGraphicsScene* scene = boardScene();
    return scene && scene->document() ? scene->document()->persistencePath() : QString();
}

void UBGraphicsPageMarkItem::markDocumentChanged()
{
    if (UBGraphicsScene* scene = boardScene())
    {
        scene->setModified(true);
    }
}

void UBGraphicsPageMarkItem::deleteFromPage()
{
    const auto answer = QMessageBox::question(UBApplication::mainWindow, tr("Delete"),
                                              tr("Delete this icon from the page?"));

    if (answer != QMessageBox::Yes || !scene())
    {
        return;
    }

    markDocumentChanged();
    scene()->removeItem(this);
    deleteLater();
}

void UBGraphicsPageMarkItem::setHighlighted(bool highlighted)
{
    mHighlighted = highlighted;
    update();
}
