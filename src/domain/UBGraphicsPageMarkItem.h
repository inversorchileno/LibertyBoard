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
#include <QPointF>
#include <QString>

class QSvgRenderer;
class UBGraphicsScene;

/**
 * @brief Base of the small icons put on a page to hold something: a quick
 * note, an attached file or a sound. The icon keeps its size on screen
 * whatever the zoom, can be moved, opens what it holds when clicked and is
 * deleted with the Delete key once clicked.
 */
class UBGraphicsPageMarkItem : public QGraphicsObject
{
    Q_OBJECT

public:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    /** @brief Opens what the mark holds. */
    virtual void open() = 0;

    /** @brief True for the items of the classes that derive from this one. */
    static bool isPageMark(const QGraphicsItem* item);

protected:
    UBGraphicsPageMarkItem(const QString& iconFile, QGraphicsItem* parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    /** @brief The board scene of the mark, if it is on one. */
    UBGraphicsScene* boardScene() const;

    /** @brief The folder of the document of the page, empty when unknown. */
    QString documentFolder() const;

    void markDocumentChanged();

    /** @brief Takes the mark off the page, after asking. */
    void deleteFromPage();

    /** @brief Draws a lighter icon, for example while a sound plays. */
    void setHighlighted(bool highlighted);

private:
    QSvgRenderer* mIcon{nullptr};
    QPointF mPressPosition;
    bool mHighlighted{false};
};
