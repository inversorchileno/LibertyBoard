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


#include "UBBoardSaver.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QRegularExpression>
#include <QStandardPaths>

#include "adaptors/UBExportFullPDF.h"
#include "board/UBBoardController.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBMainWindow.h"

static const QString sLastFolderKey = QStringLiteral("App/LibertyBoardLastSaveFolder");

UBBoardSaver::UBBoardSaver(QObject* parent)
    : QObject(parent)
{
}

bool UBBoardSaver::isFromPdf()
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBGraphicsScene> scene = board ? board->activeScene() : nullptr;

    if (!scene)
    {
        return false;
    }

    for (QGraphicsItem* item : scene->items())
    {
        if (item->type() == UBGraphicsItemType::PDFItemType)
        {
            return true;
        }
    }

    return false;
}

bool UBBoardSaver::saveTo(const QString& filename)
{
    UBBoardController* board = UBApplication::boardController;

    if (!board || !board->selectedDocument() || !board->activeScene())
    {
        return false;
    }

    // What the user wrote last must be in the document before it is exported.
    board->persistCurrentScene(false, true);

    // Written beside the file first, so that a failure leaves the file as it was.
    // It keeps the extension: the PDF export derives other names from it.
    const QFileInfo target(filename);
    const QString suffix = target.suffix().toLower();
    const QString partial = target.path() + "/" + target.completeBaseName() + ".saving." + target.suffix();
    QFile::remove(partial);

    bool written = false;

    if (suffix == "png")
    {
        written = saveAsImage(partial, "PNG");
    }
    else if (suffix == "jpg" || suffix == "jpeg")
    {
        written = saveAsImage(partial, "JPEG");
    }
    else
    {
        written = saveAsPdf(partial);
    }

    if (!written)
    {
        QFile::remove(partial);
        return false;
    }

    if (QFile::exists(filename) && !QFile::remove(filename))
    {
        // For example a PDF still open in another program.
        QFile::remove(partial);
        return false;
    }

    return QFile::rename(partial, filename);
}

QString UBBoardSaver::saveAs(QWidget* dialogParent, const QString& suggestedFile)
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBDocumentProxy> document = board ? board->selectedDocument() : nullptr;

    if (!document)
    {
        return QString();
    }

    const QString pdfFilter = tr("PDF with annotations (*.pdf)");
    const QString pngFilter = tr("PNG image of the current page (*.png)");
    const QString jpegFilter = tr("JPEG image of the current page (*.jpg *.jpeg)");

    const bool fromPdf = isFromPdf();
    const QStringList filters = fromPdf ? QStringList{ pdfFilter, pngFilter, jpegFilter }
                                        : QStringList{ pngFilter, jpegFilter, pdfFilter };

    QString suggested = suggestedFile;

    if (suggested.isEmpty())
    {
        const QString folder = UBSettings::settings()->value(sLastFolderKey,
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        QString name = document->name();
        name.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");

        if (!fromPdf)
        {
            name += QString(" - %1").arg(board->activeSceneIndex() + 1);
        }

        suggested = folder + "/" + name + (fromPdf ? ".pdf" : ".png");
    }

    QString selectedFilter = filters.first();
    const QString suggestedSuffix = QFileInfo(suggested).suffix().toLower();

    if (suggestedSuffix == "pdf")
    {
        selectedFilter = pdfFilter;
    }
    else if (suggestedSuffix == "jpg" || suggestedSuffix == "jpeg")
    {
        selectedFilter = jpegFilter;
    }
    else if (suggestedSuffix == "png")
    {
        selectedFilter = pngFilter;
    }

    QString filename = QFileDialog::getSaveFileName(dialogParent, tr("Save as"), suggested,
                                                    filters.join(";;"), &selectedFilter);

    if (filename.isEmpty())
    {
        return QString();
    }

    UBSettings::settings()->setValue(sLastFolderKey, QFileInfo(filename).absolutePath());

    if (QFileInfo(filename).suffix().isEmpty())
    {
        filename += selectedFilter == pngFilter ? ".png" : selectedFilter == jpegFilter ? ".jpg" : ".pdf";
    }

    if (!saveTo(filename))
    {
        UBApplication::mainWindow->warning(tr("Save"), tr("The file could not be saved. Check that the folder can be written."));
        return QString();
    }

    return filename;
}

bool UBBoardSaver::saveAsPdf(const QString& filename)
{
    UBExportFullPDF exporter;
    exporter.setVerbose(false);
    return exporter.persistsDocument(UBApplication::boardController->selectedDocument(), filename);
}

bool UBBoardSaver::saveAsImage(const QString& filename, const char* format)
{
    std::shared_ptr<UBGraphicsScene> scene = UBApplication::boardController->activeScene();
    const QRectF pageRect = scene->normalizedSceneRect();

    // Twice the size of the page, sharp enough to print or zoom.
    QImage image((pageRect.size() * 2).toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    {
        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        // Without the selection frames and other helpers of the screen.
        scene->setRenderingContext(UBGraphicsScene::NonScreen);
        scene->render(&painter, QRectF(image.rect()), pageRect);
        scene->setRenderingContext(UBGraphicsScene::Screen);
    }

    // The default compression for PNG, a good quality for JPEG.
    return image.save(filename, format, qstrcmp(format, "JPEG") == 0 ? 92 : -1);
}
