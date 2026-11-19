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

#include <memory>

#include <QList>
#include <QRectF>
#include <QString>

class PDFDoc;

/**
 * @brief Finds a text in the pages of a PDF file, as the words written in
 * it: a scanned page, which is only a picture, has no text to find.
 */
class UBPdfTextSearch
{
public:
    explicit UBPdfTextSearch(const QString& pdfFile);
    ~UBPdfTextSearch();

    bool isValid() const;
    int pageCount() const;

    /**
     * @brief The places of the text on the page, numbered from 1, in points
     * from the top left corner of the page as it is shown. The case of the
     * letters does not matter.
     */
    QList<QRectF> find(int pageNumber, const QString& text) const;

    /** @brief A place of the text and the line where it is. */
    struct Found
    {
        QRectF rect;
        QString line;
    };

    /** @brief As find, with the line of each place, to show it in a list. */
    QList<Found> findWithLines(int pageNumber, const QString& text) const;

private:
    std::unique_ptr<PDFDoc> mDocument;
};
