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


#include "UBPdfAnnotations.h"

#include <QFile>
#include <QFileInfo>
#include <QTransform>

// PDFDoc first: it brings the objects of poppler in the order they need.
#include <poppler/PDFDoc.h>
#include <poppler/Array.h>
#include <poppler/Catalog.h>
#include <poppler/Dict.h>
#include <poppler/GfxState.h>
#include <poppler/GlobalParams.h>
#include <poppler/Object.h>
#include <poppler/Page.h>
#include <poppler/Stream.h>
#include <poppler/XRef.h>
#include <poppler/cpp/poppler-version.h>
#include <poppler/goo/gmem.h>

namespace
{
    // A text of the PDF, in UTF-16 with its mark, for any language.
    GooString* pdfText(const QString& text)
    {
        QByteArray bytes("\xFE\xFF", 2);

        for (const QChar c : text)
        {
            bytes.append(char(c.unicode() >> 8));
            bytes.append(char(c.unicode() & 0xFF));
        }

        return new GooString(bytes.constData(), size_t(bytes.size()));
    }

    GooString* pdfDate(const QDateTime& date)
    {
        const QDateTime utc = (date.isValid() ? date : QDateTime::currentDateTime()).toUTC();
        return new GooString(("D:" + utc.toString("yyyyMMddHHmmss") + "Z").toLatin1().constData());
    }

    Object numbers(XRef* xref, std::initializer_list<double> values)
    {
        auto array = new Array(xref);

        for (double value : values)
        {
            array->add(Object(value));
        }

        return Object(array);
    }

    // Recent versions of poppler ask whether to compress the stream; older ones do not.
    template<class X>
    auto addStream(X* xref, Dict* dict, char* buffer, Goffset size, int) -> decltype(xref->addStreamObject(dict, buffer, size, {}))
    {
        return xref->addStreamObject(dict, buffer, size, {});
    }

    template<class X>
    Ref addStream(X* xref, Dict* dict, char* buffer, Goffset size, long)
    {
        return xref->addStreamObject(dict, buffer, size);
    }

    // The file as a stream of the PDF.
    Ref embedFile(XRef* xref, const QString& path)
    {
        QFile file(path);

        if (!file.open(QIODevice::ReadOnly))
        {
            return Ref::INVALID();
        }

        const QByteArray content = file.readAll();
        char* buffer = static_cast<char*>(gmalloc(size_t(qMax<qsizetype>(1, content.size()))));
        memcpy(buffer, content.constData(), size_t(content.size()));

        auto dict = new Dict(xref);
        dict->add("Type", Object(objName, "EmbeddedFile"));

        auto params = new Dict(xref);
        params->add("Size", Object(int(content.size())));
        params->add("ModDate", Object(pdfDate(QFileInfo(path).lastModified())));
        dict->add("Params", Object(params));

        // The stream frees the buffer.
        return addStream(xref, dict, buffer, content.size(), 0);
    }
}

bool UBPdfAnnotations::write(const QString& pdfFile, const QList<Mark>& marks)
{
    if (marks.isEmpty())
    {
        return true;
    }

    if (!globalParams)
    {
        return false;
    }

    // Written beside the file, then put in its place.
    const QString partial = pdfFile + ".notes";

    {
#if POPPLER_VERSION_MAJOR > 25 || (POPPLER_VERSION_MAJOR == 25 && POPPLER_VERSION_MINOR >= 12)
        PDFDoc document(std::make_unique<GooString>(static_cast<std::string_view>(pdfFile.toLocal8Bit())));
#elif POPPLER_VERSION_MAJOR > 22 || (POPPLER_VERSION_MAJOR == 22 && POPPLER_VERSION_MINOR >= 3)
        PDFDoc document(std::make_unique<GooString>(pdfFile.toLocal8Bit()));
#else
        PDFDoc document(new GooString(pdfFile.toLocal8Bit()), nullptr, nullptr, nullptr);
#endif

        if (!document.isOk())
        {
            return false;
        }

        XRef* xref = document.getXRef();
        QMap<int, QList<Ref>> annotationsOfPage;

        for (const Mark& mark : marks)
        {
            Page* page = mark.page >= 1 && mark.page <= document.getNumPages() ? document.getPage(mark.page) : nullptr;

            if (!page)
            {
                continue;
            }

            // From the place on the page as it is shown to the coordinates of the PDF.
            GfxState state(72, 72, page->getMediaBox(), page->getRotate(), true);
            const double* ctm = state.getCTM();
            const QTransform toShown(ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
            const QPointF shown(mark.place.x() * state.getPageWidth(), mark.place.y() * state.getPageHeight());
            const QPointF onPdf = toShown.inverted().map(shown);
            const double half = 10;

            auto annotation = new Dict(xref);
            annotation->add("Type", Object(objName, "Annot"));
            annotation->add("Rect", numbers(xref, { onPdf.x() - half, onPdf.y() - half, onPdf.x() + half, onPdf.y() + half }));
            annotation->add("F", Object(4)); // printed
            annotation->add("M", Object(pdfDate(mark.date)));
            annotation->add("T", Object(pdfText(QStringLiteral("LibertyBoard"))));

            if (mark.file.isEmpty())
            {
                annotation->add("Subtype", Object(objName, "Text"));
                annotation->add("Name", Object(objName, "Comment"));
                annotation->add("Contents", Object(pdfText(mark.text)));
                annotation->add("C", numbers(xref, { 1.0, 0.82, 0.2 }));
                annotation->add("Open", Object(false));
            }
            else
            {
                const Ref stream = embedFile(xref, mark.file);

                if (stream == Ref::INVALID())
                {
                    delete annotation;
                    continue;
                }

                const QString name = QFileInfo(mark.file).fileName();
                auto embedded = new Dict(xref);
                embedded->add("F", Object(stream));

                auto specification = new Dict(xref);
                specification->add("Type", Object(objName, "Filespec"));
                specification->add("F", Object(pdfText(name)));
                specification->add("UF", Object(pdfText(name)));
                specification->add("EF", Object(embedded));

                annotation->add("Subtype", Object(objName, "FileAttachment"));
                annotation->add("Name", Object(objName, "PushPin"));
                annotation->add("Contents", Object(pdfText(name)));
                annotation->add("FS", Object(specification));
                annotation->add("C", numbers(xref, { 0.24, 0.48, 0.85 }));
            }

            annotationsOfPage[mark.page] << xref->addIndirectObject(Object(annotation));
        }

        // Each page lists its annotations, the old ones and the new ones.
        for (auto it = annotationsOfPage.constBegin(); it != annotationsOfPage.constEnd(); ++it)
        {
            Ref* pageRef = document.getCatalog()->getPageRef(it.key());

            if (!pageRef)
            {
                continue;
            }

            Object pageObject = xref->fetch(*pageRef);

            if (!pageObject.isDict())
            {
                continue;
            }

            auto annotations = new Array(xref);
            Object existing = pageObject.dictLookup("Annots");

            if (existing.isArray())
            {
                for (int i = 0; i < existing.arrayGetLength(); ++i)
                {
                    annotations->add(existing.arrayGetNF(i).copy());
                }
            }

            for (const Ref& annotation : it.value())
            {
                annotations->add(Object(annotation));
            }

            pageObject.dictSet("Annots", Object(annotations));
            xref->setModifiedObject(&pageObject, *pageRef);
        }

        QFile::remove(partial);

        if (document.saveAs(GooString(partial.toLocal8Bit().constData())) != errNone)
        {
            QFile::remove(partial);
            return false;
        }
    }

    return QFile::remove(pdfFile) && QFile::rename(partial, pdfFile);
}
