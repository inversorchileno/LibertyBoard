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

#include <QVector>
#include <QWidget>

class QAudioSource;
class QIODevice;

/**
 * @brief The wave of the sound coming from the microphone while recording:
 * a flat line in silence, curves when there is sound.
 */
class UBSoundWave : public QWidget
{
    Q_OBJECT

public:
    explicit UBSoundWave(QWidget* parent = nullptr);
    ~UBSoundWave() override;

    /** @brief Listens to the microphone and draws its sound. */
    void start();

    /** @brief Stops listening; the line becomes flat. */
    void stop();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void readSound();

    QAudioSource* mSource{nullptr};
    QIODevice* mInput{nullptr};
    QVector<qreal> mLevels;
};
