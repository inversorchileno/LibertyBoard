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

class QWidget;

/**
 * @brief Saves what the user sees on the board to a file of their choice.
 *
 * A document made from a PDF is saved as a PDF with the annotations; a
 * blank board is saved as a PNG or JPEG picture of the current page.
 */
class UBBoardSaver : public QObject
{
    Q_OBJECT

public:
    explicit UBBoardSaver(QObject* parent = nullptr);

    /** @brief Asks where to save, then saves. */
    void save(QWidget* dialogParent);

private:
    bool saveAsPdf(const QString& filename);
    bool saveAsImage(const QString& filename, const char* format);
};
