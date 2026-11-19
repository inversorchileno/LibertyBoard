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

#include <QDateTime>
#include <QList>
#include <QPointF>
#include <QString>

/**
 * @brief Writes into a PDF file the quick notes and the files of the pages
 * as annotations of the PDF: a note that other readers show as a comment,
 * and a file attached to the page, which they can open or save.
 */
namespace UBPdfAnnotations
{
    struct Mark
    {
        /** @brief The page of the PDF, from 1. */
        int page{0};

        /** @brief Where on the page, from 0 to 1 across and down the page as it is shown. */
        QPointF place;

        /** @brief The text of a quick note; empty for a file. */
        QString text;
        QDateTime date;

        /** @brief The file attached, a sound or any other; empty for a quick note. */
        QString file;
    };

    /** @brief Adds the marks to the PDF file. Returns false when the file could not be written. */
    bool write(const QString& pdfFile, const QList<Mark>& marks);
}
