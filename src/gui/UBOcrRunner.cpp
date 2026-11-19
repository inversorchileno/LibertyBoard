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


#include "UBOcrRunner.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtConcurrent>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsScene.h"
#include "pdf/UBOcrEngine.h"
#include "pdf/UBPdfNavigation.h"
#include "pdf/UBPdfOcr.h"

namespace
{
    // The PDF files of the document: one for each PDF imported in it.
    QStringList pdfFilesOf(std::shared_ptr<UBDocumentProxy> document)
    {
        QStringList files;
        QDir folder(document->persistencePath() + "/" + UBPersistenceManager::objectDirectory);

        for (const QString& name : folder.entryList({ QStringLiteral("*.pdf") }, QDir::Files))
        {
            files << folder.filePath(name);
        }

        return files;
    }

    // The words of one page, in points of the page as it is shown.
    QList<UBOcrEngine::Word> readPage(const QString& pdfFile, int page, QString* error)
    {
        // Sharp enough for small print, not larger than the reader likes.
        int dpi = 300;
        QImage image = UBPdfOcr::renderPage(pdfFile, page, dpi);

        if (!image.isNull() && qMax(image.width(), image.height()) > 4200)
        {
            dpi = qMax(72, dpi * 4200 / qMax(image.width(), image.height()));
            image = UBPdfOcr::renderPage(pdfFile, page, dpi);
        }

        if (image.isNull())
        {
            return {};
        }

        // The reader works in a thread of its own; the window stays alive.
        QFutureWatcher<QList<UBOcrEngine::Word>> watcher;
        QEventLoop loop;
        QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);
        watcher.setFuture(QtConcurrent::run([image, error]() { return UBOcrEngine::recognize(image, error); }));
        loop.exec(QEventLoop::ExcludeUserInputEvents);

        QList<UBOcrEngine::Word> words = watcher.result();
        const qreal toPoints = 72.0 / dpi;

        for (UBOcrEngine::Word& word : words)
        {
            word.rect = QRectF(word.rect.topLeft() * toPoints, word.rect.size() * toPoints);
        }

        return words;
    }
}

void UBOcrRunner::run(QWidget* parent)
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBDocumentProxy> document = board ? board->selectedDocument() : nullptr;
    const QString title = QCoreApplication::translate("UBOcrRunner", "Recognize text");

    if (!document)
    {
        return;
    }

    if (!UBOcrEngine::isAvailable())
    {
        QMessageBox::information(parent, title, QCoreApplication::translate("UBOcrRunner",
            "This computer has no text reader. On Windows, the reader comes with the languages of Windows: "
            "check in the settings of Windows, Time and language, that Spanish is installed."));
        return;
    }

    // The pages without text that were not read yet.
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<QPair<QString, int>> todo;

    for (const QString& pdfFile : pdfFilesOf(document))
    {
        const UBPdfOcr::Pages done = UBPdfOcr::load(pdfFile);

        for (int page : UBPdfOcr::pagesWithoutText(pdfFile))
        {
            if (!done.contains(page))
            {
                todo << qMakePair(pdfFile, page);
            }
        }
    }

    QApplication::restoreOverrideCursor();

    if (todo.isEmpty())
    {
        QMessageBox::information(parent, title, QCoreApplication::translate("UBOcrRunner",
            "Every page of this document already has its text: there is nothing to recognize."));
        return;
    }

    QProgressDialog progress(QCoreApplication::translate("UBOcrRunner", "Recognizing the text of the scanned pages..."),
                             QCoreApplication::translate("UBOcrRunner", "Stop"), 0, todo.size(), parent);
    progress.setWindowTitle(title);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);

    QHash<QString, UBPdfOcr::Pages> results;
    int read = 0;
    int withWords = 0;
    QString lastError;

    for (int i = 0; i < todo.size() && !progress.wasCanceled(); ++i)
    {
        const QString pdfFile = todo.at(i).first;
        const int page = todo.at(i).second;
        progress.setLabelText(QCoreApplication::translate("UBOcrRunner", "Recognizing the text: page %1 of %2").arg(i + 1).arg(todo.size()));

        if (!results.contains(pdfFile))
        {
            results.insert(pdfFile, UBPdfOcr::load(pdfFile));
        }

        QString error;
        const QList<UBOcrEngine::Word> words = readPage(pdfFile, page, &error);

        if (!error.isEmpty())
        {
            lastError = error;
        }

        // A page read without words is kept too, so as not to read it again.
        if (error.isEmpty())
        {
            results[pdfFile].insert(page, words);
            // Kept at each page: a stop or a crash keeps what was read.
            UBPdfOcr::save(pdfFile, results.value(pdfFile));
            ++read;
            withWords += words.isEmpty() ? 0 : 1;
        }

        progress.setValue(i + 1);
    }

    progress.close();

    if (read == 0 && !lastError.isEmpty())
    {
        QMessageBox::warning(parent, title, QCoreApplication::translate("UBOcrRunner", "The text could not be recognized: %1").arg(lastError));
        return;
    }

    QMessageBox::information(parent, title, QCoreApplication::translate("UBOcrRunner",
        "The text of %1 pages was recognized. The search and the copy of text now work on them, "
        "and a saved PDF keeps their text.").arg(withWords));
}

void UBOcrRunner::offerIfScanned(QWidget* parent)
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBGraphicsScene> scene = board ? board->activeScene() : nullptr;
    auto item = scene ? qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject()) : nullptr;

    if (!item || !UBOcrEngine::isAvailable())
    {
        return;
    }

    const QString pdfFile = UBPdfNavigation::pdfFileOf(board->selectedDocument(), item);

    if (UBPdfOcr::wasDeclined(pdfFile) || !UBPdfOcr::load(pdfFile).isEmpty())
    {
        return;
    }

    // Its first pages are pictures only: a scanned document.
    const int first = qMin(3, UBPdfOcr::pageCount(pdfFile));

    if (first == 0 || UBPdfOcr::pagesWithoutText(pdfFile, first).size() < first)
    {
        return;
    }

    const auto answer = QMessageBox::question(parent, QCoreApplication::translate("UBOcrRunner", "Recognize text"),
        QCoreApplication::translate("UBOcrRunner", "This PDF looks scanned: its pages are pictures, without text to search or copy. "
                                               "Recognize their text now?\n\nIt can also be done later with Home > Recognize text."));

    if (answer == QMessageBox::Yes)
    {
        run(parent);
    }
    else
    {
        UBPdfOcr::setDeclined(pdfFile);
    }
}
