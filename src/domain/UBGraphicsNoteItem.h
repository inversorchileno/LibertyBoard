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

#include <QGraphicsObject>

#include "core/UB.h"

/**
 * @brief A note on a page: a small marker that shows its text when the
 * pointer is over it, and opens it for editing when clicked.
 *
 * The marker keeps the same size on screen whatever the zoom.
 */
class UBGraphicsNoteItem : public QGraphicsObject
{
    Q_OBJECT

public:
    enum { Type = UBGraphicsItemType::NoteItemType };

    explicit UBGraphicsNoteItem(const QString& text, QGraphicsItem* parent = nullptr);

    int type() const override { return Type; }

    QString text() const;
    void setText(const QString& text);

    /** @brief Opens the note: the user can change its text or delete it. */
    void edit();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void markDocumentChanged();

    QString mText;
    QPointF mPressPosition;
};
