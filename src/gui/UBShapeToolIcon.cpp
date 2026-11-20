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


#include "UBShapeToolIcon.h"

#include <QIconEngine>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRadialGradient>
#include <QtMath>

#include "core/UB.h"

namespace
{
    /** Draws the shape at the size and resolution of the screen. */
    class ShapeIconEngine : public QIconEngine
    {
    public:
        ShapeIconEngine(int tool, const QColor& stroke, const QColor& fill)
            : mTool(tool)
            , mStroke(stroke)
            , mFill(fill)
        {
            mStroke.setAlpha(255);
        }

        void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);

            // A tool that cannot be used is faint.
            if (mode == QIcon::Disabled)
            {
                painter->setOpacity(0.3);
            }

            painter->translate(rect.topLeft());
            painter->scale(rect.width() / 42.0, rect.height() / 42.0);

            if (state == QIcon::On)
            {
                // The light disk of the selected tools.
                QRadialGradient disk(QPointF(21, 19), 21);
                disk.setColorAt(0, Qt::white);
                disk.setColorAt(0.7, QColor("#e2e2e2"));
                disk.setColorAt(1, QColor("#a9a9a9"));
                painter->setPen(Qt::NoPen);
                painter->setBrush(disk);
                painter->drawEllipse(QPointF(21, 21), 20, 20);
            }

            QPainterPath outline;
            QPainterPath head;

            switch (mTool)
            {
                case UBStylusTool::Arrow:
                {
                    outline.moveTo(10, 32);
                    outline.lineTo(28.5, 13.5);
                    head.moveTo(32.5, 9.5);
                    head.lineTo(21, 12.5);
                    head.lineTo(29.5, 21);
                    head.closeSubpath();
                    break;
                }
                case UBStylusTool::Rectangle:
                    outline.addRect(QRectF(9, 12, 24, 18));
                    break;
                default:
                    outline.addEllipse(QRectF(8, 11, 26, 20));
                    break;
            }

            // The filling, then a light halo and the border, seen on the dark palette.
            if (mFill.isValid() && mFill.alpha() > 0 && mTool != UBStylusTool::Arrow)
            {
                QColor fill = mFill;
                fill.setAlpha(qMax(fill.alpha(), 200));
                painter->setPen(Qt::NoPen);
                painter->setBrush(fill);
                painter->drawPath(outline);
            }

            const QColor halo(196, 201, 210, state == QIcon::On ? 120 : 215);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(halo, 5.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(outline);

            if (!head.isEmpty())
            {
                painter->setBrush(halo);
                painter->drawPath(head);
            }

            painter->setPen(QPen(mStroke, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(outline);

            if (!head.isEmpty())
            {
                painter->setPen(QPen(mStroke, 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->setBrush(mStroke);
                painter->drawPath(head);
            }

            painter->restore();
        }

        QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
        {
            return scaledPixmap(size, mode, state, 1.0);
        }

        QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override
        {
            QPixmap pixmap(size * scale);
            pixmap.fill(Qt::transparent);

            QPainter painter(&pixmap);
            paint(&painter, QRect(QPoint(0, 0), pixmap.size()), mode, state);
            painter.end();

            pixmap.setDevicePixelRatio(scale);
            return pixmap;
        }

        QSize actualSize(const QSize& size, QIcon::Mode, QIcon::State) override
        {
            return size;
        }

        QIconEngine* clone() const override
        {
            return new ShapeIconEngine(mTool, mStroke, mFill);
        }

    private:
        int mTool;
        QColor mStroke;
        QColor mFill;
    };
}

QIcon UBShapeToolIcon::create(int tool, const QColor& stroke, const QColor& fill)
{
    return QIcon(new ShapeIconEngine(tool, stroke, fill));
}
