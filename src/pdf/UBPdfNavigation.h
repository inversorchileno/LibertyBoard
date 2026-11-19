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
class UBDocumentProxy;
class UBGraphicsPDFItem;

/**
 * @brief The index (outline) and the links of a PDF file, and the way to
 * the page of the board that shows a page of it.
 */
class UBPdfNavigation
{
public:
    explicit UBPdfNavigation(const QString& pdfFile);
    ~UBPdfNavigation();

    bool isValid() const;

    /** @brief A title of the index, the page it leads to (from 1, 0 for none) and the titles under it. */
    struct Entry
    {
        QString title;
        int page{0};
        QList<Entry> children;
    };

    QList<Entry> outline() const;

    /**
     * @brief A link of a page: where it is, in points from the top left
     * corner of the page as it is shown, and where it leads, a page of the
     * same PDF (from 1) or an address.
     */
    struct Link
    {
        QRectF rect;
        int page{0};
        QString uri;
    };

    QList<Link> links(int pageNumber) const;

    /** @brief The PDF file of the document that a page item of the board shows. */
    static QString pdfFileOf(std::shared_ptr<UBDocumentProxy> document, const UBGraphicsPDFItem* item);

    /** @brief The page of the board, from 0, that shows a page of the PDF, or -1. */
    static int sceneIndexOf(std::shared_ptr<UBDocumentProxy> document, const QString& pdfFile, int pdfPage);

    /** @brief Shows on the board the page that shows a page of the PDF. */
    static bool goToPdfPage(const QString& pdfFile, int pdfPage);

private:
    std::unique_ptr<PDFDoc> mDocument;
};
