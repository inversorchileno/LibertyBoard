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

#include <QObject>

class QTextToSpeech;

/**
 * @brief Reads the page aloud with the voices of the system.
 *
 * It reads the texts selected on the page, or else the text of the PDF
 * page and the texts written on it, from the top.
 */
class UBReadAloud : public QObject
{
    Q_OBJECT

public:
    explicit UBReadAloud(QObject* parent = nullptr);

    bool isReading() const;

    /** @brief Reads the page shown, or stops when it is reading. */
    void readOrStop();

    /** @brief The text that would be read on the page shown. */
    static QString pageText();

signals:
    void readingChanged(bool reading);

private:
    QTextToSpeech* mSpeech{nullptr};
    QString mPendingText;
};
