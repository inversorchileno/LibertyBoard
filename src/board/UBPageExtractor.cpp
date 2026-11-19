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


#include "UBPageExtractor.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <QVBoxLayout>

#include "adaptors/UBExportFullPDF.h"
#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"

static const QString sFolderKey = QStringLiteral("App/LibertyBoardExtractFolder");
static const QString sFormatKey = QStringLiteral("App/LibertyBoardExtractFormat");

enum Range
{
    CurrentPage,
    AllPages,
    SomePages
};

void UBPageExtractor::extract(QWidget* parent)
{
    UBBoardController* board = UBApplication::boardController;

    if (!board || !board->selectedDocument() || board->selectedDocument()->pageCount() == 0)
    {
        return;
    }

    // What the user wrote last must be in the document before it is extracted.
    board->persistCurrentScene(false, true);

    UBPageExtractor dialog(parent);
    dialog.exec();
}

QList<int> UBPageExtractor::parsePages(const QString& text, int pageCount)
{
    QList<int> pages;
    static const QRegularExpression part("^\\s*(\\d*)\\s*(-?)\\s*(\\d*)\\s*$");

    for (const QString& piece : text.split(QRegularExpression("[,;]"), Qt::SkipEmptyParts))
    {
        const auto match = part.match(piece);

        if (!match.hasMatch() || (match.captured(1).isEmpty() && match.captured(3).isEmpty()))
        {
            return {};
        }

        const bool isRange = !match.captured(2).isEmpty();
        const int first = match.captured(1).isEmpty() ? 1 : match.captured(1).toInt();
        int last = first;

        if (isRange)
        {
            // "8-" goes to the end, "-3" starts at the beginning.
            last = match.captured(3).isEmpty() ? pageCount : match.captured(3).toInt();
        }
        else if (match.captured(1).isEmpty())
        {
            return {};
        }

        if (first < 1 || last > pageCount || first > last)
        {
            return {};
        }

        for (int page = first; page <= last; ++page)
        {
            if (!pages.contains(page - 1))
            {
                pages << page - 1;
            }
        }
    }

    return pages;
}

UBPageExtractor::UBPageExtractor(QWidget* parent)
    : QDialog(parent)
{
    UBBoardController* board = UBApplication::boardController;
    auto document = board->selectedDocument();
    mPageCount = document->pageCount();
    mCurrentPage = board->activeSceneIndex();

    setWindowTitle(tr("Extract pages"));

    // Which pages.
    auto current = new QRadioButton(tr("Current page (%1)").arg(mCurrentPage + 1));
    auto all = new QRadioButton(tr("All pages (%1)").arg(mPageCount));
    auto some = new QRadioButton(tr("Pages:"));
    mPagesText = new QLineEdit;
    mPagesText->setPlaceholderText(tr("For example 1-3, 5, 8-"));
    mPagesText->setToolTip(tr("Page numbers and ranges, separated by commas. \"8-\" goes to the last page."));

    mRange = new QButtonGroup(this);
    mRange->addButton(current, CurrentPage);
    mRange->addButton(all, AllPages);
    mRange->addButton(some, SomePages);
    current->setChecked(true);
    // Writing pages chooses them.
    connect(mPagesText, &QLineEdit::textEdited, some, [some]() { some->setChecked(true); });

    auto someRow = new QHBoxLayout;
    someRow->addWidget(some);
    someRow->addWidget(mPagesText, 1);

    auto pagesBox = new QVBoxLayout;
    pagesBox->addWidget(current);
    pagesBox->addWidget(all);
    pagesBox->addLayout(someRow);

    // How.
    mFormat = new QComboBox;
    mFormat->addItem(tr("PDF, all the pages in one file"), OnePdf);
    mFormat->addItem(tr("PDF, one file for each page"), PdfPerPage);
    mFormat->addItem(tr("PNG pictures, one for each page"), PngPerPage);
    mFormat->addItem(tr("JPEG pictures, one for each page"), JpegPerPage);
    mFormat->setCurrentIndex(qMax(0, mFormat->findData(UBSettings::settings()->value(sFormatKey, OnePdf).toInt())));

    // Where.
    QString name = document->name();
    name.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");
    mName = new QLineEdit(tr("%1 - pages").arg(name));

    const QString folder = UBSettings::settings()->value(sFolderKey,
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    mFolder = new QLineEdit(QDir::toNativeSeparators(folder));
    auto browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &UBPageExtractor::browse);

    auto folderRow = new QHBoxLayout;
    folderRow->addWidget(mFolder, 1);
    folderRow->addWidget(browseButton);

    mOpenWhenDone = new QCheckBox(tr("Open when done"));
    mOpenWhenDone->setChecked(true);

    auto form = new QFormLayout;
    form->addRow(tr("Pages"), pagesBox);
    form->addRow(tr("Format"), mFormat);
    form->addRow(tr("File name"), mName);
    form->addRow(tr("Folder"), folderRow);
    form->addRow(QString(), mOpenWhenDone);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Extract"));
    connect(buttons, &QDialogButtonBox::accepted, this, &UBPageExtractor::run);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    resize(520, sizeHint().height());
}

QList<int> UBPageExtractor::chosenPages() const
{
    switch (mRange->checkedId())
    {
    case CurrentPage:
        return { mCurrentPage };

    case AllPages:
    {
        QList<int> pages;

        for (int page = 0; page < mPageCount; ++page)
        {
            pages << page;
        }

        return pages;
    }

    default:
        return parsePages(mPagesText->text(), mPageCount);
    }
}

QStringList UBPageExtractor::filesFor(const QList<int>& pages) const
{
    const QString base = QDir(QDir::fromNativeSeparators(mFolder->text().trimmed())).filePath(mName->text().trimmed());
    const int format = mFormat->currentData().toInt();

    if (format == OnePdf)
    {
        return { base + ".pdf" };
    }

    const QString suffix = format == PdfPerPage ? "pdf" : format == PngPerPage ? "png" : "jpg";
    QStringList files;

    for (int page : pages)
    {
        files << tr("%1 - page %2").arg(base).arg(page + 1) + "." + suffix;
    }

    return files;
}

void UBPageExtractor::browse()
{
    const QString folder = QFileDialog::getExistingDirectory(this, tr("Folder"), QDir::fromNativeSeparators(mFolder->text()));

    if (!folder.isEmpty())
    {
        mFolder->setText(QDir::toNativeSeparators(folder));
    }
}

void UBPageExtractor::run()
{
    const QList<int> pages = chosenPages();

    if (pages.isEmpty())
    {
        QMessageBox::warning(this, windowTitle(), tr("Write the pages as numbers from 1 to %1, for example 1-3, 5.").arg(mPageCount));
        mPagesText->setFocus();
        return;
    }

    QString name = mName->text().trimmed();
    const QString folder = QDir::fromNativeSeparators(mFolder->text().trimmed());

    if (name.isEmpty() || name.contains(QRegularExpression("[\\\\/:*?\"<>|]")))
    {
        QMessageBox::warning(this, windowTitle(), tr("Write a file name without the signs \\ / : * ? \" < > |."));
        mName->setFocus();
        return;
    }

    if (!QDir().mkpath(folder))
    {
        QMessageBox::warning(this, windowTitle(), tr("The folder could not be created. Choose another one."));
        return;
    }

    const QStringList files = filesFor(pages);
    QStringList existing;

    for (const QString& file : files)
    {
        if (QFile::exists(file))
        {
            existing << QFileInfo(file).fileName();
        }
    }

    if (!existing.isEmpty())
    {
        const QString shown = existing.size() > 5 ? tr("%1 files").arg(existing.size()) : existing.join("\n");
        const auto answer = QMessageBox::question(this, windowTitle(), tr("Replace the files that already exist?\n\n%1").arg(shown));

        if (answer != QMessageBox::Yes)
        {
            return;
        }
    }

    UBSettings::settings()->setValue(sFolderKey, folder);
    UBSettings::settings()->setValue(sFormatKey, mFormat->currentData());

    // Each file is written apart first, and replaces the old one only when complete.
    QTemporaryDir work;
    bool written = work.isValid();
    const int format = mFormat->currentData().toInt();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    for (int index = 0; written && index < files.size(); ++index)
    {
        const QString partial = work.filePath(QString("page%1.%2").arg(index).arg(QFileInfo(files.at(index)).suffix()));

        switch (format)
        {
        case OnePdf:
            written = writePdf(pages, partial);
            break;
        case PdfPerPage:
            written = writePdf({ pages.at(index) }, partial);
            break;
        case PngPerPage:
            written = writeImage(pages.at(index), partial, "PNG");
            break;
        default:
            written = writeImage(pages.at(index), partial, "JPEG");
            break;
        }

        if (written)
        {
            QFile::remove(files.at(index));
            written = QFile::copy(partial, files.at(index));
        }
    }

    QApplication::restoreOverrideCursor();
    UBApplication::showMessage(QString());

    if (!written)
    {
        QMessageBox::warning(this, windowTitle(), tr("The pages could not be extracted. Check that the folder can be written and that the files are not open in another program."));
        return;
    }

    if (mOpenWhenDone->isChecked())
    {
        // One file opens; several show their folder.
        const QString shown = files.size() == 1 ? files.first() : folder;
        QDesktopServices::openUrl(QUrl::fromLocalFile(shown));
    }

    accept();
}

bool UBPageExtractor::writePdf(const QList<int>& pages, const QString& filename) const
{
    UBExportFullPDF exporter;
    exporter.setVerbose(false);
    exporter.setPages(pages);
    return exporter.persistsDocument(UBApplication::boardController->selectedDocument(), filename) && QFile::exists(filename);
}

bool UBPageExtractor::writeImage(int page, const QString& filename, const char* format) const
{
    std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(
        UBApplication::boardController->selectedDocument(), page);

    if (!scene)
    {
        return false;
    }

    const QRectF pageRect = scene->normalizedSceneRect();

    // Twice the size of the page, as Save as does.
    QImage image((pageRect.size() * 2).toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    {
        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        scene->setRenderingContext(UBGraphicsScene::NonScreen);
        scene->render(&painter, QRectF(image.rect()), pageRect);
        scene->setRenderingContext(UBGraphicsScene::Screen);
    }

    return image.save(filename, format, qstrcmp(format, "JPEG") == 0 ? 92 : -1);
}
