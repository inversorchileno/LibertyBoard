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

#include <QDialog>
#include <QString>

class QAudioInput;
class QAudioOutput;
class QLabel;
class QMediaCaptureSession;
class QMediaPlayer;
class QMediaRecorder;
class QProgressBar;
class QPushButton;
class QToolButton;

/**
 * @brief Gets the sound of a page: recorded with the microphone, or taken
 * from an audio file, and listened to before it is accepted.
 */
class UBSoundDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UBSoundDialog(QWidget* parent = nullptr);
    ~UBSoundDialog() override;

    /** @brief The sound accepted, recorded or chosen; empty when there is none. */
    QString soundFile() const;

private:
    void play();
    void stop();
    void record();
    void browse();
    void setSoundFile(const QString& file);
    void updateButtons();

    QString mFile;
    QString mRecordingFile;
    QMediaPlayer* mPlayer{nullptr};
    QAudioOutput* mOutput{nullptr};
    QMediaCaptureSession* mSession{nullptr};
    QAudioInput* mInput{nullptr};
    QMediaRecorder* mRecorder{nullptr};
    QProgressBar* mProgress{nullptr};
    QLabel* mStatus{nullptr};
    QToolButton* mPlay{nullptr};
    QToolButton* mStop{nullptr};
    QToolButton* mRecord{nullptr};
    QPushButton* mAccept{nullptr};
};
