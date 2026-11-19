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


#include "UBOcrEngine.h"

#include <cstring>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Media::Ocr;

namespace
{
    // The reader in the languages of the user, or in Spanish or English.
    OcrEngine createEngine()
    {
        OcrEngine engine = OcrEngine::TryCreateFromUserProfileLanguages();

        for (const wchar_t* tag : { L"es", L"en" })
        {
            if (engine)
            {
                break;
            }

            winrt::Windows::Globalization::Language language(tag);

            if (OcrEngine::IsLanguageSupported(language))
            {
                engine = OcrEngine::TryCreateFromLanguage(language);
            }
        }

        return engine;
    }
}

bool UBOcrEngine::isAvailable()
{
    try
    {
        return static_cast<bool>(createEngine());
    }
    catch (...)
    {
        return false;
    }
}

QList<UBOcrEngine::Word> UBOcrEngine::recognize(const QImage& source, QString* error)
{
    QList<Word> words;

    try
    {
        // The thread of the reader joins the apartment of Windows; one already
        // set is kept.
        try
        {
            winrt::init_apartment(winrt::apartment_type::multi_threaded);
        }
        catch (...)
        {
        }

        OcrEngine engine = createEngine();

        if (!engine)
        {
            if (error)
            {
                *error = QStringLiteral("no OCR language");
            }

            return words;
        }

        // Not larger than the reader accepts.
        QImage image = source;
        const int largest = int(OcrEngine::MaxImageDimension());
        qreal factor = 1;

        if (image.width() > largest || image.height() > largest)
        {
            image = image.scaled(largest, largest, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            factor = qreal(image.width()) / source.width();
        }

        // Blue, green, red, alpha in memory, as the reader wants.
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        const uint32_t size = uint32_t(image.width()) * uint32_t(image.height()) * 4;
        winrt::Windows::Storage::Streams::Buffer buffer(size);

        for (int y = 0; y < image.height(); ++y)
        {
            std::memcpy(buffer.data() + size_t(y) * image.width() * 4, image.constScanLine(y), size_t(image.width()) * 4);
        }

        buffer.Length(size);

        SoftwareBitmap bitmap(BitmapPixelFormat::Bgra8, image.width(), image.height(), BitmapAlphaMode::Premultiplied);
        bitmap.CopyFromBuffer(buffer);

        OcrResult result = engine.RecognizeAsync(bitmap).get();
        int line = 0;

        for (const OcrLine& ocrLine : result.Lines())
        {
            for (const OcrWord& ocrWord : ocrLine.Words())
            {
                const auto r = ocrWord.BoundingRect();
                Word word;
                word.text = QString::fromWCharArray(ocrWord.Text().c_str());
                word.rect = QRectF(r.X / factor, r.Y / factor, r.Width / factor, r.Height / factor);
                word.line = line;
                words << word;
            }

            ++line;
        }
    }
    catch (const winrt::hresult_error& e)
    {
        if (error)
        {
            *error = QString::fromWCharArray(e.message().c_str());
        }
    }
    catch (...)
    {
        if (error)
        {
            *error = QStringLiteral("OCR failed");
        }
    }

    return words;
}
