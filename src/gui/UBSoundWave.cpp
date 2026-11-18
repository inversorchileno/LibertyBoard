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


#include "UBSoundWave.h"

#include <QAudioFormat>
#include <QAudioSource>
#include <QMediaDevices>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

// The values drawn, one for each few milliseconds of sound, newest at the right.
static constexpr int sLevelCount = 240;
static constexpr int sSamplesPerLevel = 160;

UBSoundWave::UBSoundWave(QWidget* parent)
    : QWidget(parent)
    , mLevels(sLevelCount, 0.0)
{
    setMinimumHeight(56);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

UBSoundWave::~UBSoundWave()
{
    stop();
}

void UBSoundWave::start()
{
    stop();

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    mSource = new QAudioSource(QMediaDevices::defaultAudioInput(), format, this);
    mInput = mSource->start();

    if (mInput)
    {
        connect(mInput, &QIODevice::readyRead, this, &UBSoundWave::readSound);
    }
}

void UBSoundWave::stop()
{
    if (mSource)
    {
        mSource->stop();
        mSource->deleteLater();
    }

    mSource = nullptr;
    mInput = nullptr;
    mLevels.fill(0.0);
    update();
}

void UBSoundWave::readSound()
{
    const QByteArray data = mInput->readAll();
    const auto samples = reinterpret_cast<const qint16*>(data.constData());
    const int count = int(data.size() / sizeof(qint16));

    // Each value is the sound of a few milliseconds, with its sign, so that
    // the line goes up and down like a wave.
    for (int start = 0; start + sSamplesPerLevel <= count; start += sSamplesPerLevel)
    {
        qreal strongest = 0;

        for (int i = start; i < start + sSamplesPerLevel; ++i)
        {
            const qreal value = samples[i] / 32768.0;

            if (qAbs(value) > qAbs(strongest))
            {
                strongest = value;
            }
        }

        // Quiet sounds are made visible, loud ones stay inside.
        const qreal shown = (strongest < 0 ? -1 : 1) * qMin(1.0, qSqrt(qAbs(strongest)) * 1.4);
        mLevels.removeFirst();
        mLevels.append(shown);
    }

    update();
}

void UBSoundWave::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#13161b"));
    painter.drawRoundedRect(rect(), 6, 6);

    const qreal middle = height() / 2.0;
    const qreal amplitude = height() / 2.0 - 5;
    const qreal step = qreal(width() - 12) / (sLevelCount - 1);

    QPainterPath wave;
    wave.moveTo(6, middle - mLevels.first() * amplitude);

    for (int i = 1; i < sLevelCount; ++i)
    {
        // Every other value goes the other way, for the look of a sound wave.
        const qreal value = (i % 2 == 0 ? 1 : -1) * qAbs(mLevels.at(i));
        wave.lineTo(6 + i * step, middle - value * amplitude);
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(mSource ? "#3fd47a" : "#5a6270"), 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(wave);
}
