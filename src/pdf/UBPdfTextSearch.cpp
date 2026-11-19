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


#include "UBPdfTextSearch.h"

#include <QVector>

#include <poppler/GlobalParams.h>
#include <poppler/PDFDoc.h>
#include <poppler/TextOutputDev.h>
#include <poppler/cpp/poppler-version.h>

UBPdfTextSearch::UBPdfTextSearch(const QString& pdfFile)
{
    // The renderer of the board has set up the global parameters of poppler.
    if (!globalParams)
    {
        return;
    }

#if POPPLER_VERSION_MAJOR > 25 || (POPPLER_VERSION_MAJOR == 25 && POPPLER_VERSION_MINOR >= 12)
    mDocument = std::make_unique<PDFDoc>(std::make_unique<GooString>(static_cast<std::string_view>(pdfFile.toLocal8Bit())));
#elif POPPLER_VERSION_MAJOR > 22 || (POPPLER_VERSION_MAJOR == 22 && POPPLER_VERSION_MINOR >= 3)
    mDocument = std::make_unique<PDFDoc>(std::make_unique<GooString>(pdfFile.toLocal8Bit()));
#else
    mDocument = std::make_unique<PDFDoc>(new GooString(pdfFile.toLocal8Bit()), nullptr, nullptr, nullptr);
#endif
}

UBPdfTextSearch::~UBPdfTextSearch() = default;

bool UBPdfTextSearch::isValid() const
{
    return mDocument && mDocument->isOk();
}

int UBPdfTextSearch::pageCount() const
{
    return isValid() ? mDocument->getNumPages() : 0;
}

// The versions of poppler give the text in different ways.
static QString takeText(GooString* text)
{
    const QString result = text ? QString::fromUtf8(text->c_str()) : QString();
    delete text;
    return result;
}

[[maybe_unused]] static QString takeText(std::unique_ptr<GooString> text)
{
    return text ? QString::fromUtf8(text->c_str()) : QString();
}

[[maybe_unused]] static QString takeText(const GooString& text)
{
    return QString::fromUtf8(text.c_str());
}

QList<QRectF> UBPdfTextSearch::find(int pageNumber, const QString& text) const
{
    QList<QRectF> places;

    for (const Found& found : findWithLines(pageNumber, text))
    {
        places << found.rect;
    }

    return places;
}

QList<UBPdfTextSearch::Found> UBPdfTextSearch::findWithLines(int pageNumber, const QString& text) const
{
    QList<Found> places;

    if (!isValid() || text.isEmpty() || pageNumber < 1 || pageNumber > pageCount())
    {
        return places;
    }

    TextOutputDev output(nullptr, true, 0, false, false);

    if (!output.isOk())
    {
        return places;
    }

    // At 72 dots per inch, the places are in points, as the page is shown.
    mDocument->displayPage(&output, pageNumber, 72, 72, 0, true, false, false);

    QVector<Unicode> letters;

    for (const uint letter : text.toUcs4())
    {
        letters << Unicode(letter);
    }

    double xMin = 0;
    double yMin = 0;
    double xMax = 0;
    double yMax = 0;
    bool first = true;

    // Each search starts after the place found before.
    while (output.findText(letters.data(), letters.size(), first, true, !first, false,
                           false, false, false, &xMin, &yMin, &xMax, &yMax))
    {
        // The words in line with the place, across the page: a text that
        // reads upwards or downwards, on a turned page, is taller than wide.
        QString line;

        if (yMax - yMin > (xMax - xMin) * 1.5 && letters.size() > 1)
        {
            const double middle = (xMin + xMax) / 2;
            const double half = qMax(1.0, (xMax - xMin) / 4);
            line = takeText(output.getText(middle - half, -1e5, middle + half, 1e5));
        }
        else
        {
            const double middle = (yMin + yMax) / 2;
            const double half = qMax(1.0, (yMax - yMin) / 4);
            line = takeText(output.getText(-1e5, middle - half, 1e5, middle + half));
        }

        line = line.simplified();

        places << Found{ QRectF(QPointF(xMin, yMin), QPointF(xMax, yMax)), line };
        first = false;
    }

    return places;
}
