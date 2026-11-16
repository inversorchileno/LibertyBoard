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

static bool hasPdfPage(UBGraphicsScene* scene)
{
    for (QGraphicsItem* item : scene->items())
    {
        if (item->type() == UBGraphicsItemType::PDFItemType)
        {
            return true;
        }
    }

    return false;
}

UBBoardSaver::UBBoardSaver(QObject* parent)
    : QObject(parent)
{
}

void UBBoardSaver::save(QWidget* dialogParent)
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBDocumentProxy> document = board ? board->selectedDocument() : nullptr;
    std::shared_ptr<UBGraphicsScene> scene = board ? board->activeScene() : nullptr;

    if (!document || !scene)
    {
        return;
    }

    // What the user wrote last must be in the document before it is exported.
    board->persistCurrentScene(false, true);

    const QString pdfFilter = tr("PDF with annotations (*.pdf)");
    const QString pngFilter = tr("PNG image of the current page (*.png)");
    const QString jpegFilter = tr("JPEG image of the current page (*.jpg *.jpeg)");

    const bool fromPdf = hasPdfPage(scene.get());
    const QStringList filters = fromPdf ? QStringList{ pdfFilter, pngFilter, jpegFilter }
                                        : QStringList{ pngFilter, jpegFilter, pdfFilter };

    const QString folder = UBSettings::settings()->value(sLastFolderKey,
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString name = document->name();
    name.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");

    if (!fromPdf)
    {
        name += QString(" - %1").arg(board->activeSceneIndex() + 1);
    }

    QString selectedFilter = filters.first();
    const QString suggested = folder + "/" + name + (fromPdf ? ".pdf" : ".png");
    QString filename = QFileDialog::getSaveFileName(dialogParent, tr("Save"), suggested,
                                                    filters.join(";;"), &selectedFilter);

    if (filename.isEmpty())
    {
        return;
    }

    UBSettings::settings()->setValue(sLastFolderKey, QFileInfo(filename).absolutePath());

    const QString suffix = QFileInfo(filename).suffix().toLower();
    bool saved = false;

    if (suffix == "png" || (suffix.isEmpty() && selectedFilter == pngFilter))
    {
        saved = saveAsImage(suffix.isEmpty() ? filename + ".png" : filename, "PNG");
    }
    else if (suffix == "jpg" || suffix == "jpeg" || (suffix.isEmpty() && selectedFilter == jpegFilter))
    {
        saved = saveAsImage(suffix.isEmpty() ? filename + ".jpg" : filename, "JPEG");
    }
    else
    {
        saved = saveAsPdf(suffix == "pdf" ? filename : filename + ".pdf");
    }

    if (saved)
    {
        UBApplication::showMessage(tr("Saved to %1").arg(QFileInfo(filename).fileName()));
    }
    else
    {
        UBApplication::mainWindow->warning(tr("Save"), tr("The file could not be saved. Check that the folder can be written."));
    }
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
