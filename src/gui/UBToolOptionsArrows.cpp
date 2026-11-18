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


#include "UBToolOptionsArrows.h"

#include <QAction>
#include <QIcon>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"

namespace
{
    // The icons as they are without a triangle, kept the first time a tool is seen.
    QMap<QAction*, QIcon>& plainIcons()
    {
        static QMap<QAction*, QIcon> icons;
        return icons;
    }

    QList<QAction*> toolsWithOptions()
    {
        UBMainWindow* w = UBApplication::mainWindow;
        return { w->actionPen, w->actionEraser, w->actionMarker, w->actionPointer, w->actionHighlight,
                 w->actionNote, w->actionUnderline, w->actionText, w->actionLine, w->actionArrowShape,
                 w->actionRectangle, w->actionEllipse };
    }

    QIcon plainIcon(QAction* action)
    {
        UBMainWindow* w = UBApplication::mainWindow;

        // The pen, eraser and marker have icons with an old black arrow:
        // their plain icons are used instead.
        auto fromFiles = [](const QString& name) {
            QIcon icon;
            icon.addFile(":images/stylusPalette/" + name + ".svg", QSize(), QIcon::Normal, QIcon::Off);
            icon.addFile(":images/stylusPalette/" + name + "On.svg", QSize(), QIcon::Normal, QIcon::On);
            return icon;
        };

        if (action == w->actionPen)
        {
            return fromFiles("pen");
        }

        if (action == w->actionEraser)
        {
            return fromFiles("eraser");
        }

        if (action == w->actionMarker)
        {
            return fromFiles("marker");
        }

        if (!plainIcons().contains(action))
        {
            plainIcons().insert(action, action->icon());
        }

        return plainIcons().value(action);
    }

    /** The triangle in a 42 x 42 icon. */
    QPolygonF triangle(Qt::Edge paletteEdge)
    {
        switch (paletteEdge)
        {
            case Qt::RightEdge: // pointing left, bottom left
                return QPolygonF({ QPointF(7, 30), QPointF(1, 35), QPointF(7, 40) });
            case Qt::TopEdge: // pointing down, bottom center
                return QPolygonF({ QPointF(16, 35), QPointF(26, 35), QPointF(21, 41) });
            case Qt::BottomEdge: // pointing up, top center
                return QPolygonF({ QPointF(16, 7), QPointF(26, 7), QPointF(21, 1) });
            default: // left: pointing right, bottom right
                return QPolygonF({ QPointF(35, 30), QPointF(41, 35), QPointF(35, 40) });
        }
    }

    QIcon withTriangle(const QIcon& icon, Qt::Edge paletteEdge)
    {
        QIcon result;
        const QSize size(42, 42);

        for (QIcon::State state : { QIcon::Off, QIcon::On })
        {
            QPixmap pixmap = icon.pixmap(size, QIcon::Normal, state);

            if (pixmap.size() != size)
            {
                // An icon smaller than the button is centered, as the button would.
                QPixmap centered(size);
                centered.fill(Qt::transparent);
                QPainter center(&centered);
                center.drawPixmap((size.width() - pixmap.width()) / 2, (size.height() - pixmap.height()) / 2, pixmap);
                center.end();
                pixmap = centered;
            }

            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(QPen(QColor(235, 235, 235), 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.setBrush(Qt::black);
            painter.drawPolygon(triangle(paletteEdge));
            painter.end();

            result.addPixmap(pixmap, QIcon::Normal, state);
        }

        return result;
    }
}

void UBToolOptionsArrows::apply(Qt::Edge paletteEdge)
{
    if (!UBApplication::mainWindow)
    {
        return;
    }

    for (QAction* action : toolsWithOptions())
    {
        action->setIcon(withTriangle(plainIcon(action), paletteEdge));
    }
}

bool UBToolOptionsArrows::isOnTriangle(Qt::Edge paletteEdge, const QPoint& position, const QSize& buttonSize)
{
    // The icon is centered in the button; a margin around the triangle
    // makes it easy to hit with a pen.
    const QPointF offset((buttonSize.width() - 42) / 2.0, (buttonSize.height() - 42) / 2.0);
    const QRectF area = triangle(paletteEdge).boundingRect().translated(offset).adjusted(-5, -5, 5, 5);
    return area.contains(position);
}

Qt::Edge UBToolOptionsArrows::openingSide(Qt::Edge paletteEdge)
{
    switch (paletteEdge)
    {
        case Qt::RightEdge:
            return Qt::LeftEdge;
        case Qt::TopEdge:
            return Qt::BottomEdge;
        case Qt::BottomEdge:
            return Qt::TopEdge;
        default:
            return Qt::RightEdge;
    }
}
