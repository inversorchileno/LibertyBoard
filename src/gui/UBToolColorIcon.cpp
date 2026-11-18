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


#include "UBToolColorIcon.h"

#include <QIconEngine>
#include <QImage>
#include <QPainter>
#include <QPixmap>

namespace
{
    // Below this saturation a pixel is grey, white or black, and keeps its color.
    constexpr qreal sColoredSaturation = 0.25;

    QImage recolored(QImage image, const QColor& target)
    {
        image = image.convertToFormat(QImage::Format_ARGB32);

        // The mean lightness of the colored parts, which becomes the lightness of the target.
        qreal lightnessSum = 0;
        int coloredCount = 0;

        for (int y = 0; y < image.height(); ++y)
        {
            const QRgb* line = reinterpret_cast<const QRgb*>(image.constScanLine(y));

            for (int x = 0; x < image.width(); ++x)
            {
                const QColor pixel = QColor::fromRgba(line[x]);

                if (pixel.alpha() > 0 && pixel.hslSaturationF() >= sColoredSaturation)
                {
                    lightnessSum += pixel.lightnessF();
                    ++coloredCount;
                }
            }
        }

        if (coloredCount == 0)
        {
            return image;
        }

        const qreal meanLightness = lightnessSum / coloredCount;
        const QColor opaque = target.toHsl();
        const bool grey = opaque.hslSaturationF() < 0.1;

        for (int y = 0; y < image.height(); ++y)
        {
            QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));

            for (int x = 0; x < image.width(); ++x)
            {
                const QColor pixel = QColor::fromRgba(line[x]);

                if (pixel.alpha() == 0 || pixel.hslSaturationF() < sColoredSaturation)
                {
                    continue;
                }

                // Lighter and darker than the mean as much as the original was.
                const qreal lightness = qBound(0.0, opaque.lightnessF() + (pixel.lightnessF() - meanLightness), 1.0);
                const qreal saturation = grey ? 0.0 : qBound(0.0, opaque.hslSaturationF() * pixel.hslSaturationF() / 0.85, 1.0);
                QColor result = QColor::fromHslF(grey ? 0.0 : qMax(0.0, opaque.hslHueF()), saturation, lightness);
                result.setAlpha(pixel.alpha());
                line[x] = result.rgba();
            }
        }

        return image;
    }

    /** Draws the icon in the color at the size and resolution of the screen. */
    class ColorIconEngine : public QIconEngine
    {
    public:
        ColorIconEngine(const QIcon& icon, const QColor& color)
            : mIcon(icon)
            , mColor(color)
        {
        }

        void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
        {
            const qreal scale = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
            painter->drawPixmap(rect, scaledPixmap(rect.size(), mode, state, scale));
        }

        QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
        {
            return scaledPixmap(size, mode, state, 1.0);
        }

        QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override
        {
            const QSize physical = size * scale;
            QImage image(physical, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            {
                QPainter painter(&image);
                mIcon.paint(&painter, QRect(QPoint(0, 0), physical), Qt::AlignCenter, mode, state);
            }

            QPixmap pixmap = QPixmap::fromImage(recolored(image, mColor));
            pixmap.setDevicePixelRatio(scale);
            return pixmap;
        }

        QSize actualSize(const QSize& size, QIcon::Mode, QIcon::State) override
        {
            return size;
        }

        QIconEngine* clone() const override
        {
            return new ColorIconEngine(mIcon, mColor);
        }

    private:
        QIcon mIcon;
        QColor mColor;
    };
}

QIcon UBToolColorIcon::create(const QIcon& icon, const QColor& color)
{
    return QIcon(new ColorIconEngine(icon, color));
}
