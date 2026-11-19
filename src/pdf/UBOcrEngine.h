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

#include <QImage>
#include <QList>
#include <QRectF>
#include <QString>

/**
 * @brief Reads the words of a picture (optical character recognition).
 *
 * On Windows it uses the reader of Windows itself (Windows.Media.Ocr),
 * in the languages of the user; elsewhere the tesseract program, when it
 * is installed.
 */
namespace UBOcrEngine
{
    struct Word
    {
        QString text;

        /** @brief Where the word is, in pixels of the picture. */
        QRectF rect;

        /** @brief The line of the word, counted from 0 in reading order. */
        int line{0};
    };

    /** @brief True when there is a reader on this computer. */
    bool isAvailable();

    /**
     * @brief The words of the picture, in reading order. Can run in a thread
     * of its own. On failure the list is empty and the error says why.
     */
    QList<Word> recognize(const QImage& image, QString* error);
}
