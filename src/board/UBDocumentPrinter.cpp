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


#include "UBDocumentPrinter.h"

#include <QApplication>
#include <QPageLayout>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"

void UBDocumentPrinter::print(QWidget* dialogParent)
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBDocumentProxy> document = board ? board->selectedDocument() : nullptr;

    if (!document || document->pageCount() == 0)
    {
        return;
    }

    // What is on the page shown is in the document before printing.
    board->persistCurrentScene(false, true);

    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(document->name());
    printer.setFromTo(1, document->pageCount());

    QPrintDialog dialog(&printer, dialogParent);
    dialog.setOptions(QAbstractPrintDialog::PrintPageRange | QAbstractPrintDialog::PrintCurrentPage
                      | QAbstractPrintDialog::PrintToFile | QAbstractPrintDialog::PrintShowPageSize);
    dialog.setMinMax(1, document->pageCount());

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    int first = 1;
    int last = document->pageCount();

    if (printer.printRange() == QPrinter::PageRange)
    {
        first = qMax(1, printer.fromPage());
        last = qMin(document->pageCount(), printer.toPage());
    }
    else if (printer.printRange() == QPrinter::CurrentPage)
    {
        first = last = board->activeSceneIndex() + 1;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QPainter painter;
    bool started = false;

    for (int page = first; page <= last; ++page)
    {
        std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(document, page - 1);

        if (!scene)
        {
            continue;
        }

        const QRectF source = scene->normalizedSceneRect();
        printer.setPageOrientation(source.width() > source.height() ? QPageLayout::Landscape : QPageLayout::Portrait);

        if (!started)
        {
            if (!painter.begin(&printer))
            {
                break;
            }

            started = true;
        }
        else
        {
            printer.newPage();
        }

        // The page fills the sheet, keeping its proportions, centered.
        const QRectF sheet = painter.viewport();
        QSizeF size = source.size();
        size.scale(sheet.size(), Qt::KeepAspectRatio);
        const QRectF target(sheet.left() + (sheet.width() - size.width()) / 2,
                            sheet.top() + (sheet.height() - size.height()) / 2, size.width(), size.height());

        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        scene->setRenderingContext(UBGraphicsScene::NonScreen);
        scene->render(&painter, target, source);
        scene->setRenderingContext(UBGraphicsScene::Screen);
    }

    if (started)
    {
        painter.end();
    }

    QApplication::restoreOverrideCursor();
}
