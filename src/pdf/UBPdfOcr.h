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

#include <QHash>
#include <QImage>
#include <QList>
#include <QString>

#include "pdf/UBOcrEngine.h"

/**
 * @brief The text read on the scanned pages of a PDF of a document.
 *
 * The words of each page are kept in a file beside the PDF, its name with
 * ".ocr.json" added, in points from the top left corner of the page as it
 * is shown, so that the search and the copy of the text use them.
 */
namespace UBPdfOcr
{
    using Word = UBOcrEngine::Word;
    using Pages = QHash<int, QList<Word>>;

    /** @brief The words read on the pages of the PDF, by page from 1. */
    Pages load(const QString& pdfFile);

    bool save(const QString& pdfFile, const Pages& pages);

    /** @brief True when the user said no to reading this PDF. */
    bool wasDeclined(const QString& pdfFile);
    void setDeclined(const QString& pdfFile);

    /** @brief The pages, from 1, without any text of their own: pictures of paper. */
    QList<int> pagesWithoutText(const QString& pdfFile, int firstPages = -1);

    int pageCount(const QString& pdfFile);

    /** @brief The page as a picture, at this many dots per inch. */
    QImage renderPage(const QString& pdfFile, int page, int dpi);
}
