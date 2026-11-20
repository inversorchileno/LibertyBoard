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
#include <QString>
#include <QStringList>

/**
 * @brief The emojis of the emoji tool: the emojis offered, the one chosen
 * to put on the page, and its size there.
 *
 * An emoji is put on the page as a picture drawn with the color emoji
 * font of the system, so that it is saved, exported and printed like a
 * picture.
 */
class UBEmojis
{
public:
    /** @brief The emojis offered, those a teacher uses most first. */
    static QStringList all();

    /** @brief The emoji drawn in color on a square picture of this side. */
    static QImage image(const QString& emoji, int side);

    /** @brief The emoji chosen. */
    static QString emoji();

    /** @brief The width on the page of an emoji put, in points of the page. */
    static int size();
};
