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

    mOcr = UBPdfOcr::load(pdfFile);
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

// Older versions of poppler give the list of words to delete.
[[maybe_unused]] static void releaseList(TextWordList* words)
{
    delete words;
}

[[maybe_unused]] static void releaseList(std::unique_ptr<TextWordList>& words)
{
    words.reset();
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

QString UBPdfTextSearch::textIn(int pageNumber, const QRectF& rect) const
{
    if (!isValid() || pageNumber < 1 || pageNumber > pageCount() || rect.isEmpty())
    {
        return QString();
    }

    if (mOcr.contains(pageNumber))
    {
        return ocrTextIn(pageNumber, rect);
    }

    // In reading order, as the words were written.
    TextOutputDev output(nullptr, false, 0, false, false);

    if (!output.isOk())
    {
        return QString();
    }

    mDocument->displayPage(&output, pageNumber, 72, 72, 0, true, false, false);

    // A word touched by the rectangle is taken whole, not cut in its letters.
    QRectF area = rect;
    auto words = output.makeWordList();

    for (int i = 0; words && i < words->getLength(); ++i)
    {
        double xMin = 0;
        double yMin = 0;
        double xMax = 0;
        double yMax = 0;
        words->get(i)->getBBox(&xMin, &yMin, &xMax, &yMax);
        const QRectF word(QPointF(xMin, yMin), QPointF(xMax, yMax));

        // Most of its height in the rectangle: a line only grazed is left out.
        const QRectF common = word.intersected(rect);

        if (!common.isEmpty() && common.height() > word.height() / 2)
        {
            area |= word;
        }
    }

    releaseList(words);

    QString text = takeText(output.getText(area.left(), area.top(), area.right(), area.bottom()));

    // Lines end as on the system, without a blank one at the end.
    text.replace("\r\n", "\n");
    return text.trimmed();
}

QList<UBPdfTextSearch::Found> UBPdfTextSearch::findWithLines(int pageNumber, const QString& text) const
{
    QList<Found> places;

    if (!isValid() || text.isEmpty() || pageNumber < 1 || pageNumber > pageCount())
    {
        return places;
    }

    if (mOcr.contains(pageNumber))
    {
        return findInOcr(pageNumber, text);
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

QList<UBPdfTextSearch::Found> UBPdfTextSearch::findInOcr(int pageNumber, const QString& text) const
{
    QList<Found> places;
    const QList<UBPdfOcr::Word> words = mOcr.value(pageNumber);

    // The words one after the other, each letter knowing its word.
    QString all;
    QList<int> wordOf;

    for (int i = 0; i < words.size(); ++i)
    {
        if (!all.isEmpty())
        {
            all += QLatin1Char(' ');
            wordOf << -1;
        }

        all += words.at(i).text;

        for (int c = 0; c < words.at(i).text.size(); ++c)
        {
            wordOf << i;
        }
    }

    const QString wanted = text.simplified();
    int from = 0;

    while (!wanted.isEmpty())
    {
        const int at = all.indexOf(wanted, from, Qt::CaseInsensitive);

        if (at < 0)
        {
            break;
        }

        QRectF rect;
        int line = -1;

        for (int c = at; c < at + wanted.size(); ++c)
        {
            const int word = wordOf.at(c);

            if (word >= 0)
            {
                rect |= words.at(word).rect;
                line = line < 0 ? words.at(word).line : line;
            }
        }

        QStringList lineWords;

        for (const UBPdfOcr::Word& word : words)
        {
            if (word.line == line)
            {
                lineWords << word.text;
            }
        }

        places << Found{ rect, lineWords.join(QLatin1Char(' ')) };
        from = at + wanted.size();
    }

    return places;
}

QString UBPdfTextSearch::ocrTextIn(int pageNumber, const QRectF& rect) const
{
    QStringList lines;
    int line = -1;

    for (const UBPdfOcr::Word& word : mOcr.value(pageNumber))
    {
        // Most of the height of the word in the rectangle, as for the words of a PDF.
        const QRectF common = word.rect.intersected(rect);

        if (common.isEmpty() || common.height() <= word.rect.height() / 2)
        {
            continue;
        }

        if (word.line != line || lines.isEmpty())
        {
            lines << word.text;
            line = word.line;
        }
        else
        {
            lines.last() += QLatin1Char(' ') + word.text;
        }
    }

    return lines.join(QLatin1Char('\n'));
}
