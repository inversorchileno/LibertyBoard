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


#include "UBPdfNavigation.h"

#include <poppler/Annot.h>
#include <poppler/GfxState.h>
#include <poppler/GlobalParams.h>
#include <poppler/Link.h>
#include <poppler/Outline.h>
#include <poppler/PDFDoc.h>
#include <poppler/Page.h>
#include <poppler/cpp/poppler-version.h>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsScene.h"

namespace
{
    // The title of an entry of the index: a list of letters in recent
    // versions of poppler, letters and their number in older ones.
    template<class Item>
    auto titleOf(const Item* item, int) -> decltype(item->getTitleLength(), QString())
    {
        QString title;

        for (int i = 0; i < item->getTitleLength(); ++i)
        {
            const char32_t letter = item->getTitle()[i];
            title += QString::fromUcs4(&letter, 1);
        }

        return title;
    }

    template<class Item>
    QString titleOf(const Item* item, long)
    {
        QString title;

        for (const auto letter : item->getTitle())
        {
            const char32_t code = letter;
            title += QString::fromUcs4(&code, 1);
        }

        return title;
    }
}

UBPdfNavigation::UBPdfNavigation(const QString& pdfFile)
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

UBPdfNavigation::~UBPdfNavigation() = default;

bool UBPdfNavigation::isValid() const
{
    return mDocument && mDocument->isOk();
}

// The page a link or an entry of the index leads to, from 1, or 0.
static int pageOf(PDFDoc* document, const LinkAction* action)
{
    if (!action || action->getKind() != actionGoTo)
    {
        return 0;
    }

    auto goTo = static_cast<const LinkGoTo*>(action);
    std::unique_ptr<LinkDest> named;
    const LinkDest* destination = goTo->getDest();

    if (!destination && goTo->getNamedDest())
    {
        named = document->findDest(goTo->getNamedDest());
        destination = named.get();
    }

    if (!destination || !destination->isOk())
    {
        return 0;
    }

    const int page = destination->isPageRef() ? document->findPage(destination->getPageRef()) : destination->getPageNum();
    return page >= 1 && page <= document->getNumPages() ? page : 0;
}

static QList<UBPdfNavigation::Entry> entriesOf(PDFDoc* document, const std::vector<OutlineItem*>* items, int depth)
{
    QList<UBPdfNavigation::Entry> entries;

    // A broken file could loop: the index stops at some depth.
    if (!items || depth > 12)
    {
        return entries;
    }

    for (OutlineItem* item : *items)
    {
        UBPdfNavigation::Entry entry;
        entry.title = titleOf(item, 0).simplified();
        entry.page = pageOf(document, item->getAction());

        if (item->hasKids())
        {
            item->open();
            entry.children = entriesOf(document, item->getKids(), depth + 1);
        }

        entries << entry;
    }

    return entries;
}

QList<UBPdfNavigation::Entry> UBPdfNavigation::outline() const
{
    if (!isValid() || !mDocument->getOutline())
    {
        return {};
    }

    return entriesOf(mDocument.get(), mDocument->getOutline()->getItems(), 0);
}

QList<UBPdfNavigation::Link> UBPdfNavigation::links(int pageNumber) const
{
    QList<Link> result;

    if (!isValid() || pageNumber < 1 || pageNumber > mDocument->getNumPages())
    {
        return result;
    }

    Page* page = mDocument->getPage(pageNumber);
    std::unique_ptr<Links> links = page ? page->getLinks() : nullptr;

    if (!links)
    {
        return result;
    }

    // From the coordinates of the PDF to points of the page as it is shown,
    // as the search finds the words.
    GfxState state(72, 72, page->getMediaBox(), page->getRotate(), true);

    for (AnnotLink* annotation : links->getLinks())
    {
        const LinkAction* action = annotation ? annotation->getAction() : nullptr;

        if (!action)
        {
            continue;
        }

        Link link;

        if (action->getKind() == actionURI)
        {
            link.uri = QString::fromStdString(static_cast<const LinkURI*>(action)->getURI());
        }
        else
        {
            link.page = pageOf(mDocument.get(), action);
        }

        if (link.uri.isEmpty() && link.page == 0)
        {
            continue;
        }

        double x1 = 0;
        double y1 = 0;
        double x2 = 0;
        double y2 = 0;
        annotation->getRect(&x1, &y1, &x2, &y2);

        double tx1 = 0;
        double ty1 = 0;
        double tx2 = 0;
        double ty2 = 0;
        state.transform(x1, y1, &tx1, &ty1);
        state.transform(x2, y2, &tx2, &ty2);
        link.rect = QRectF(QPointF(tx1, ty1), QPointF(tx2, ty2)).normalized();

        result << link;
    }

    return result;
}

QString UBPdfNavigation::pdfFileOf(std::shared_ptr<UBDocumentProxy> document, const UBGraphicsPDFItem* item)
{
    if (!document || !item)
    {
        return QString();
    }

    return document->persistencePath() + "/" + UBPersistenceManager::objectDirectory + "/"
           + item->fileUuid().toString() + ".pdf";
}

int UBPdfNavigation::sceneIndexOf(std::shared_ptr<UBDocumentProxy> document, const QString& pdfFile, int pdfPage)
{
    if (!document)
    {
        return -1;
    }

    // An imported PDF has one page of the board for each page of the PDF,
    // in the same order: that page is tried first.
    QList<int> candidates{ pdfPage - 1 };

    for (int i = 0; i < document->pageCount(); ++i)
    {
        if (i != pdfPage - 1)
        {
            candidates << i;
        }
    }

    for (int index : std::as_const(candidates))
    {
        if (index < 0 || index >= document->pageCount())
        {
            continue;
        }

        std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(document, index);
        auto item = scene ? qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject()) : nullptr;

        if (item && item->pageNumber() == pdfPage && pdfFileOf(document, item) == pdfFile)
        {
            return index;
        }
    }

    return -1;
}

bool UBPdfNavigation::goToPdfPage(const QString& pdfFile, int pdfPage)
{
    UBBoardController* board = UBApplication::boardController;
    const int index = board ? sceneIndexOf(board->selectedDocument(), pdfFile, pdfPage) : -1;

    if (index < 0)
    {
        return false;
    }

    if (index != board->activeSceneIndex())
    {
        board->persistCurrentScene(false, true);
        board->setActiveDocumentScene(index);
    }

    return true;
}
