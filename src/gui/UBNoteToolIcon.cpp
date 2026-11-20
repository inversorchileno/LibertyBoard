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


#include "UBNoteToolIcon.h"

#include <QIconEngine>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

namespace
{
    QByteArray ribbonSvg(const QColor& color, bool selected)
    {
        const QString light = color.lighter(150).name();
        const QString middle = color.name();
        const QString dark = color.darker(170).name();
        const QString disk = selected
            ? QStringLiteral("<defs><radialGradient id='bg' cx='21' cy='19' r='21' gradientUnits='userSpaceOnUse'>"
                             "<stop offset='0' stop-color='#ffffff'/><stop offset='0.7' stop-color='#e2e2e2'/>"
                             "<stop offset='1' stop-color='#a9a9a9'/></radialGradient></defs>"
                             "<circle cx='21' cy='21' r='20' fill='url(#bg)'/>")
            : QString();

        const QString svg = QStringLiteral(
            "<svg xmlns='http://www.w3.org/2000/svg' width='42' height='42' viewBox='0 0 42 42'>%1"
            "<defs><linearGradient id='ribbon' x1='0' y1='0' x2='1' y2='0'>"
            "<stop offset='0' stop-color='%3'/><stop offset='0.4' stop-color='%2'/><stop offset='1' stop-color='%4'/>"
            "</linearGradient></defs>"
            "<path d='M13.2 7.4 L30.2 7.4 L30.2 35.4 L21.7 28.4 L13.2 35.4 Z' fill='#000' fill-opacity='0.3'/>"
            "<path d='M12 6 L29 6 L29 34 L20.5 27 L12 34 Z' fill='url(#ribbon)' stroke='%4' stroke-width='1.2' stroke-linejoin='round'/>"
            "<path d='M14 8 L14 29' stroke='#ffffff' stroke-opacity='0.55' stroke-width='1.6' stroke-linecap='round'/>"
            "<path d='M16 12 L25 12 M16 17 L23 17' stroke='#ffffff' stroke-opacity='0.9' stroke-width='2' stroke-linecap='round'/>"
            "</svg>").arg(disk, middle, light, dark);

        return svg.toUtf8();
    }

    /** Draws the ribbon at the size and resolution of the screen. */
    class RibbonIconEngine : public QIconEngine
    {
    public:
        explicit RibbonIconEngine(const QColor& color)
            : mColor(color)
        {
        }

        void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
        {
            // A tool that cannot be used is faint.
            painter->save();

            if (mode == QIcon::Disabled)
            {
                painter->setOpacity(0.3);
            }

            QSvgRenderer renderer(ribbonSvg(mColor, state == QIcon::On));
            renderer.render(painter, QRectF(rect));
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
            return new RibbonIconEngine(mColor);
        }

    private:
        QColor mColor;
    };
}

QIcon UBNoteToolIcon::create(const QColor& color)
{
    return QIcon(new RibbonIconEngine(color));
}
