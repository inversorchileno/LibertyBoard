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
#include <QString>

class QWidget;

/**
 * @brief Writes the document of the board to a file of the user: a PDF with
 * the annotations, or the current page as a PNG or JPEG picture.
 */
class UBBoardSaver : public QObject
{
    Q_OBJECT

public:
    explicit UBBoardSaver(QObject* parent = nullptr);

    /** @brief True when the current page shows a page of an imported PDF. */
    static bool isFromPdf();

    /**
     * @brief Writes the file, in the format of its extension (pdf, png, jpg).
     * An existing file is only replaced once the new one is complete.
     */
    bool saveTo(const QString& filename);

    /**
     * @brief Asks for a file with the dialog of the system, then saves to it.
     * Returns the file written, or an empty string when the user cancels or
     * saving fails.
     */
    QString saveAs(QWidget* dialogParent, const QString& suggestedFile);

private:
    bool saveAsPdf(const QString& filename);
    bool saveAsImage(const QString& filename, const char* format);
};
