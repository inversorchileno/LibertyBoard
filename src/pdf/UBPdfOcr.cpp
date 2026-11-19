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


#include "UBPdfOcr.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

// PDFDoc first: it brings the objects of poppler in the order they need.
#include <poppler/PDFDoc.h>
#include <poppler/GlobalParams.h>
#include <poppler/SplashOutputDev.h>
#include <poppler/TextOutputDev.h>
#include <poppler/cpp/poppler-version.h>
#include <splash/SplashBitmap.h>

namespace
{
    QString sidecar(const QString& pdfFile)
    {
        return pdfFile + QStringLiteral(".ocr.json");
    }

    QJsonObject read(const QString& pdfFile)
    {
        QFile file(sidecar(pdfFile));

        if (!file.open(QIODevice::ReadOnly))
        {
            return QJsonObject();
        }

        return QJsonDocument::fromJson(file.readAll()).object();
    }

    bool write(const QString& pdfFile, const QJsonObject& object)
    {
        QSaveFile file(sidecar(pdfFile));

        if (!file.open(QIODevice::WriteOnly))
        {
            return false;
        }

        file.write(QJsonDocument(object).toJson(QJsonDocument::Compact));
        return file.commit();
    }

    std::unique_ptr<PDFDoc> open(const QString& pdfFile)
    {
        if (!globalParams)
        {
            return nullptr;
        }

#if POPPLER_VERSION_MAJOR > 25 || (POPPLER_VERSION_MAJOR == 25 && POPPLER_VERSION_MINOR >= 12)
        auto document = std::make_unique<PDFDoc>(std::make_unique<GooString>(static_cast<std::string_view>(pdfFile.toLocal8Bit())));
#elif POPPLER_VERSION_MAJOR > 22 || (POPPLER_VERSION_MAJOR == 22 && POPPLER_VERSION_MINOR >= 3)
        auto document = std::make_unique<PDFDoc>(std::make_unique<GooString>(pdfFile.toLocal8Bit()));
#else
        auto document = std::make_unique<PDFDoc>(new GooString(pdfFile.toLocal8Bit()), nullptr, nullptr, nullptr);
#endif
        return document->isOk() ? std::move(document) : nullptr;
    }
}

UBPdfOcr::Pages UBPdfOcr::load(const QString& pdfFile)
{
    Pages pages;
    const QJsonObject words = read(pdfFile).value(QStringLiteral("pages")).toObject();

    for (auto it = words.constBegin(); it != words.constEnd(); ++it)
    {
        QList<Word> list;

        for (const QJsonValue& value : it.value().toArray())
        {
            // text, x, y, width, height, line
            const QJsonArray item = value.toArray();

            if (item.size() < 6)
            {
                continue;
            }

            Word word;
            word.text = item.at(0).toString();
            word.rect = QRectF(item.at(1).toDouble(), item.at(2).toDouble(), item.at(3).toDouble(), item.at(4).toDouble());
            word.line = item.at(5).toInt();
            list << word;
        }

        pages.insert(it.key().toInt(), list);
    }

    return pages;
}

bool UBPdfOcr::save(const QString& pdfFile, const Pages& pages)
{
    QJsonObject object = read(pdfFile);
    QJsonObject words;

    for (auto it = pages.constBegin(); it != pages.constEnd(); ++it)
    {
        QJsonArray list;

        for (const Word& word : it.value())
        {
            list.append(QJsonArray{ word.text, qRound(word.rect.x() * 10) / 10.0, qRound(word.rect.y() * 10) / 10.0,
                                    qRound(word.rect.width() * 10) / 10.0, qRound(word.rect.height() * 10) / 10.0, word.line });
        }

        words.insert(QString::number(it.key()), list);
    }

    object.insert(QStringLiteral("pages"), words);
    return write(pdfFile, object);
}

bool UBPdfOcr::wasDeclined(const QString& pdfFile)
{
    return read(pdfFile).value(QStringLiteral("declined")).toBool();
}

void UBPdfOcr::setDeclined(const QString& pdfFile)
{
    QJsonObject object = read(pdfFile);
    object.insert(QStringLiteral("declined"), true);
    write(pdfFile, object);
}

QList<int> UBPdfOcr::pagesWithoutText(const QString& pdfFile, int firstPages)
{
    QList<int> pages;
    std::unique_ptr<PDFDoc> document = open(pdfFile);

    if (!document)
    {
        return pages;
    }

    const int last = firstPages > 0 ? qMin(firstPages, document->getNumPages()) : document->getNumPages();

    for (int page = 1; page <= last; ++page)
    {
        TextOutputDev output(nullptr, false, 0, false, false);

        if (!output.isOk())
        {
            break;
        }

        document->displayPage(&output, page, 72, 72, 0, true, false, false);
        auto words = output.makeWordList();

        // A few letters, a page number for example, are not a text.
        int letters = 0;

        for (int i = 0; words && i < words->getLength() && letters < 20; ++i)
        {
            letters += words->get(i)->getLength();
        }

        if (letters < 20)
        {
            pages << page;
        }
    }

    return pages;
}

int UBPdfOcr::pageCount(const QString& pdfFile)
{
    std::unique_ptr<PDFDoc> document = open(pdfFile);
    return document ? document->getNumPages() : 0;
}

QImage UBPdfOcr::renderPage(const QString& pdfFile, int page, int dpi)
{
    std::unique_ptr<PDFDoc> document = open(pdfFile);

    if (!document || page < 1 || page > document->getNumPages())
    {
        return QImage();
    }

    SplashColor paper = { 0xFF, 0xFF, 0xFF };
#if POPPLER_VERSION_MAJOR > 26 || (POPPLER_VERSION_MAJOR == 26 && POPPLER_VERSION_MINOR >= 2)
    SplashOutputDev output(splashModeRGB8, 1, paper);
#else
    SplashOutputDev output(splashModeRGB8, 1, false, paper);
#endif
    output.startDoc(document.get());

    // The page as the search sees it: its media box, turned as it is shown.
    document->displayPage(&output, page, dpi, dpi, 0, true, false, false);
    SplashBitmap* bitmap = output.getBitmap();

    if (!bitmap)
    {
        return QImage();
    }

    return QImage(bitmap->getDataPtr(), bitmap->getWidth(), bitmap->getHeight(), bitmap->getRowSize(), QImage::Format_RGB888).copy();
}
