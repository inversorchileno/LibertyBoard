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
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "board/UBPageExtractor.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"

namespace
{
    const QString sPrinterKey = QStringLiteral("App/LibertyBoardPrinter");
    const QString sPaperKey = QStringLiteral("App/LibertyBoardPrintPaper");
    const QString sScaleKey = QStringLiteral("App/LibertyBoardPrintScale");
    const QString sColorKey = QStringLiteral("App/LibertyBoardPrintColor");
    const QString sSidesKey = QStringLiteral("App/LibertyBoardPrintSides");

    // The entry of the printers that writes a PDF file instead.
    const QString sPdfFile = QStringLiteral("\u0001pdf");

    enum Range { AllPages, CurrentPage, SomePages };
}

void UBDocumentPrinter::print(QWidget* dialogParent)
{
    UBBoardController* board = UBApplication::boardController;

    if (!board || !board->selectedDocument() || board->selectedDocument()->pageCount() == 0)
    {
        return;
    }

    // What is on the page shown is in the document before printing.
    board->persistCurrentScene(false, true);

    UBPrintDialog dialog(dialogParent);
    dialog.exec();
}

UBPrintDialog::UBPrintDialog(QWidget* parent)
    : QDialog(parent)
{
    UBBoardController* board = UBApplication::boardController;
    mDocument = board->selectedDocument();
    mPageCount = mDocument->pageCount();
    mCurrentPage = board->activeSceneIndex();
    UBSettings* settings = UBSettings::settings();

    setWindowTitle(tr("Print"));

    // The printers of the system, the usual one first chosen.
    mPrinter = new QComboBox;
    const QString defaultPrinter = QPrinterInfo::defaultPrinterName();

    for (const QString& name : QPrinterInfo::availablePrinterNames())
    {
        mPrinter->addItem(name, name);
    }

    mPrinter->addItem(tr("PDF file"), sPdfFile);
    const QString chosenPrinter = settings->value(sPrinterKey, defaultPrinter).toString();
    int printerIndex = mPrinter->findData(chosenPrinter);

    if (printerIndex < 0)
    {
        printerIndex = qMax(0, mPrinter->findData(defaultPrinter));
    }

    mPrinter->setCurrentIndex(printerIndex);

    auto properties = new QPushButton(tr("Printer settings..."));
    properties->setToolTip(tr("The settings of the printer itself: tray, quality, and others"));
    auto printerRow = new QHBoxLayout;
    printerRow->addWidget(mPrinter, 1);
    printerRow->addWidget(properties);

    mCopies = new QSpinBox;
    mCopies->setRange(1, 999);
    mCollate = new QCheckBox(tr("Collate"));
    mCollate->setChecked(true);
    mCollate->setToolTip(tr("Each copy complete, one after the other"));
    auto copiesRow = new QHBoxLayout;
    copiesRow->addWidget(mCopies);
    copiesRow->addWidget(mCollate);
    copiesRow->addStretch(1);

    // Which pages, as in Extract pages.
    auto all = new QRadioButton(tr("All pages (%1)").arg(mPageCount));
    auto current = new QRadioButton(tr("Current page (%1)").arg(mCurrentPage + 1));
    auto some = new QRadioButton(tr("Pages:"));
    mPagesText = new QLineEdit;
    mPagesText->setPlaceholderText(tr("For example 1-3, 5, 8-"));
    mRange = new QButtonGroup(this);
    mRange->addButton(all, AllPages);
    mRange->addButton(current, CurrentPage);
    mRange->addButton(some, SomePages);
    all->setChecked(true);
    auto someRow = new QHBoxLayout;
    someRow->addWidget(some);
    someRow->addWidget(mPagesText, 1);
    auto pagesBox = new QVBoxLayout;
    pagesBox->addWidget(all);
    pagesBox->addWidget(current);
    pagesBox->addLayout(someRow);

    mPaper = new QComboBox;
    const QList<QPageSize::PageSizeId> papers = { QPageSize::Letter, QPageSize::Legal, QPageSize::A4, QPageSize::A3, QPageSize::A5 };

    for (QPageSize::PageSizeId id : papers)
    {
        mPaper->addItem(QPageSize::name(id), int(id));
    }

    mPaper->setCurrentIndex(qMax(0, mPaper->findData(settings->value(sPaperKey, int(QPageSize::Letter)).toInt())));

    mOrientation = new QComboBox;
    mOrientation->addItem(tr("Automatic, as each page"), Automatic);
    mOrientation->addItem(tr("Portrait"), Portrait);
    mOrientation->addItem(tr("Landscape"), Landscape);

    mScale = new QComboBox;
    mScale->addItem(tr("Fit to the sheet"), 0);
    mScale->addItem(tr("Actual size"), 1);
    mScale->setCurrentIndex(qBound(0, settings->value(sScaleKey, 0).toInt(), 1));

    mPerSheet = new QComboBox;
    mPerSheet->addItem(tr("1 page per sheet"), 1);
    mPerSheet->addItem(tr("2 pages per sheet"), 2);
    mPerSheet->addItem(tr("4 pages per sheet"), 4);

    mColor = new QComboBox;
    mColor->addItem(tr("Color"), int(QPrinter::Color));
    mColor->addItem(tr("Black and white (grayscale)"), int(QPrinter::GrayScale));
    mColor->setCurrentIndex(qMax(0, mColor->findData(settings->value(sColorKey, int(QPrinter::Color)).toInt())));

    mSides = new QComboBox;
    mSides->addItem(tr("One side"), int(QPrinter::DuplexNone));
    mSides->addItem(tr("Both sides, turning on the long edge"), int(QPrinter::DuplexLongSide));
    mSides->addItem(tr("Both sides, turning on the short edge"), int(QPrinter::DuplexShortSide));
    mSides->setCurrentIndex(qMax(0, mSides->findData(settings->value(sSidesKey, int(QPrinter::DuplexNone)).toInt())));

    mAnnotations = new QCheckBox(tr("Print the annotations"));
    mAnnotations->setChecked(true);
    mAnnotations->setToolTip(tr("Without them, only the pages of the PDF are printed, as they came"));

    auto form = new QFormLayout;
    form->addRow(tr("Printer"), printerRow);
    form->addRow(tr("Copies"), copiesRow);
    form->addRow(tr("Pages"), pagesBox);
    form->addRow(tr("Paper"), mPaper);
    form->addRow(tr("Orientation"), mOrientation);
    form->addRow(tr("Scale"), mScale);
    form->addRow(tr("Pages per sheet"), mPerSheet);
    form->addRow(tr("Color"), mColor);
    form->addRow(tr("Sides"), mSides);
    form->addRow(QString(), mAnnotations);

    mWarning = new QLabel;
    mWarning->setStyleSheet("QLabel { color: #c0392b; }");
    mWarning->setWordWrap(true);
    mWarning->hide();

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Print"));
    connect(buttons, &QDialogButtonBox::accepted, this, &UBPrintDialog::run);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto left = new QVBoxLayout;
    left->addLayout(form);
    left->addWidget(mWarning);
    left->addStretch(1);
    left->addWidget(buttons);

    // The preview, sheet by sheet.
    mPreview = new QLabel;
    mPreview->setAlignment(Qt::AlignCenter);
    mPreview->setMinimumSize(420, 520);
    mPreview->setStyleSheet("QLabel { background: #5b6170; border-radius: 4px; }");
    mPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mPrevious = new QToolButton;
    mPrevious->setText(QStringLiteral("\u2039"));
    mPrevious->setToolTip(tr("Previous sheet"));
    mNext = new QToolButton;
    mNext->setText(QStringLiteral("\u203A"));
    mNext->setToolTip(tr("Next sheet"));
    mSheetLabel = new QLabel;
    mSheetLabel->setAlignment(Qt::AlignCenter);
    mSheetLabel->setMinimumWidth(140);
    auto navigation = new QHBoxLayout;
    navigation->addStretch(1);
    navigation->addWidget(mPrevious);
    navigation->addWidget(mSheetLabel);
    navigation->addWidget(mNext);
    navigation->addStretch(1);

    auto right = new QVBoxLayout;
    right->addWidget(mPreview, 1);
    right->addLayout(navigation);

    auto layout = new QHBoxLayout(this);
    layout->addLayout(left, 0);
    layout->addSpacing(12);
    layout->addLayout(right, 1);

    connect(properties, &QPushButton::clicked, this, &UBPrintDialog::printerProperties);
    connect(mPrinter, &QComboBox::currentIndexChanged, this, [this]() {
        mDriverSettings.reset();
        // A printer that prints on both sides or in color, or not.
        const QString name = mPrinter->currentData().toString();
        const QPrinterInfo info = QPrinterInfo::printerInfo(name);
        mSides->setEnabled(name == sPdfFile ? false : info.isNull() || info.supportedDuplexModes().size() > 1);
    });
    emit mPrinter->currentIndexChanged(mPrinter->currentIndex());

    connect(mPrevious, &QToolButton::clicked, this, [this]() { showSheet(mShownSheet - 1); });
    connect(mNext, &QToolButton::clicked, this, [this]() { showSheet(mShownSheet + 1); });
    connect(mRange, &QButtonGroup::idClicked, this, [this]() { mShownSheet = 0; updatePreview(); });
    connect(mPagesText, &QLineEdit::textEdited, this, [this, some]() {
        some->setChecked(true);
        mShownSheet = 0;
        updatePreview();
    });

    for (QComboBox* box : { mPaper, mOrientation, mScale, mPerSheet, mColor })
    {
        connect(box, &QComboBox::currentIndexChanged, this, [this]() { updatePreview(); });
    }

    connect(mAnnotations, &QCheckBox::toggled, this, [this]() { updatePreview(); });

    resize(1000, 700);
    // Once the dialog has its size, so that the preview fills it.
    QTimer::singleShot(0, this, &UBPrintDialog::updatePreview);
}

UBPrintDialog::~UBPrintDialog() = default;

QList<int> UBPrintDialog::chosenPages() const
{
    switch (mRange->checkedId())
    {
    case CurrentPage:
        return { mCurrentPage };

    case SomePages:
        return UBPageExtractor::parsePages(mPagesText->text(), mPageCount);

    default:
    {
        QList<int> pages;

        for (int page = 0; page < mPageCount; ++page)
        {
            pages << page;
        }

        return pages;
    }
    }
}

int UBPrintDialog::pagesPerSheet() const
{
    return mPerSheet->currentData().toInt();
}

int UBPrintDialog::sheetCount() const
{
    const int pages = chosenPages().size();
    return (pages + pagesPerSheet() - 1) / pagesPerSheet();
}

QPageSize UBPrintDialog::paperSize() const
{
    return QPageSize(QPageSize::PageSizeId(mPaper->currentData().toInt()));
}

QPageLayout::Orientation UBPrintDialog::sheetOrientation(int sheet) const
{
    const int chosen = mOrientation->currentData().toInt();

    if (chosen == Portrait)
    {
        return QPageLayout::Portrait;
    }

    if (chosen == Landscape)
    {
        return QPageLayout::Landscape;
    }

    // Two pages side by side fit a landscape sheet; one or four, the page decides.
    if (pagesPerSheet() == 2)
    {
        return QPageLayout::Landscape;
    }

    const QList<int> pages = chosenPages();
    const int first = sheet * pagesPerSheet();

    if (first >= pages.size())
    {
        return QPageLayout::Portrait;
    }

    std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(mDocument, pages.at(first));
    const QRectF rect = scene ? scene->normalizedSceneRect() : QRectF();
    return rect.width() > rect.height() ? QPageLayout::Landscape : QPageLayout::Portrait;
}

void UBPrintDialog::drawSheet(QPainter* painter, const QRectF& area, int sheet) const
{
    const QList<int> pages = chosenPages();
    const int perSheet = pagesPerSheet();
    QList<QRectF> cells;

    if (perSheet == 1)
    {
        cells << area;
    }
    else if (perSheet == 2)
    {
        // Side by side on a wide sheet, one above the other on a tall one.
        if (area.width() >= area.height())
        {
            const qreal half = area.width() / 2;
            cells << QRectF(area.left(), area.top(), half, area.height()) << QRectF(area.left() + half, area.top(), half, area.height());
        }
        else
        {
            const qreal half = area.height() / 2;
            cells << QRectF(area.left(), area.top(), area.width(), half) << QRectF(area.left(), area.top() + half, area.width(), half);
        }
    }
    else
    {
        const qreal w = area.width() / 2;
        const qreal h = area.height() / 2;
        cells << QRectF(area.left(), area.top(), w, h) << QRectF(area.left() + w, area.top(), w, h)
              << QRectF(area.left(), area.top() + h, w, h) << QRectF(area.left() + w, area.top() + h, w, h);
    }

    for (int i = 0; i < cells.size(); ++i)
    {
        const int index = sheet * perSheet + i;

        if (index < pages.size())
        {
            // A small gap between the pages of a sheet.
            const qreal gap = perSheet == 1 ? 0 : qMin(cells.at(i).width(), cells.at(i).height()) * 0.03;
            drawPage(painter, cells.at(i).adjusted(gap, gap, -gap, -gap), pages.at(index));
        }
    }
}

void UBPrintDialog::drawPage(QPainter* painter, const QRectF& cell, int page) const
{
    std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(mDocument, page);

    if (!scene)
    {
        return;
    }

    const QRectF source = scene->normalizedSceneRect();
    QSizeF size = source.size();

    if (mScale->currentData().toInt() == 1 && pagesPerSheet() == 1)
    {
        // The size of the page itself: the board counts in dots of the document.
        const qreal dpi = mDocument->pageDpi() > 0 ? mDocument->pageDpi() : 96;
        const qreal deviceDpi = painter->device() ? painter->device()->logicalDpiX() : 96;
        size = source.size() * deviceDpi / dpi;

        // Never larger than the sheet.
        if (size.width() > cell.width() || size.height() > cell.height())
        {
            size.scale(cell.size(), Qt::KeepAspectRatio);
        }
    }
    else
    {
        size.scale(cell.size(), Qt::KeepAspectRatio);
    }

    const QRectF target(cell.left() + (cell.width() - size.width()) / 2, cell.top() + (cell.height() - size.height()) / 2,
                        size.width(), size.height());

    // Without the annotations, only the page of the PDF, or the background.
    QList<QGraphicsItem*> hidden;

    if (!mAnnotations->isChecked())
    {
        QGraphicsItem* background = scene->backgroundObject();

        for (QGraphicsItem* item : scene->items())
        {
            if (item != background && item->isVisible() && !item->parentItem())
            {
                item->setVisible(false);
                hidden << item;
            }
        }
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    scene->setRenderingContext(UBGraphicsScene::NonScreen);
    scene->render(painter, target, source);
    scene->setRenderingContext(UBGraphicsScene::Screen);
    painter->restore();

    for (QGraphicsItem* item : std::as_const(hidden))
    {
        item->setVisible(true);
    }
}

void UBPrintDialog::updatePreview()
{
    const int sheets = sheetCount();

    if (sheets == 0)
    {
        mWarning->setText(tr("Write the pages as numbers from 1 to %1, for example 1-3, 5.").arg(mPageCount));
        mWarning->show();
        mPreview->clear();
        mSheetLabel->clear();
        mPrevious->setEnabled(false);
        mNext->setEnabled(false);
        return;
    }

    mWarning->hide();
    showSheet(qBound(0, mShownSheet, sheets - 1));
}

void UBPrintDialog::showSheet(int sheet)
{
    const int sheets = sheetCount();

    if (sheets == 0)
    {
        return;
    }

    mShownSheet = qBound(0, sheet, sheets - 1);
    mPrevious->setEnabled(mShownSheet > 0);
    mNext->setEnabled(mShownSheet < sheets - 1);
    mSheetLabel->setText(tr("Sheet %1 of %2").arg(mShownSheet + 1).arg(sheets));

    // The paper, in its orientation, with the margins of the printer.
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const QPageLayout layout(paperSize(), sheetOrientation(mShownSheet), QMarginsF(12, 12, 12, 12), QPageLayout::Point);
    const QRectF paper = layout.fullRectPoints();
    const QRectF printable = layout.paintRectPoints();

    QSizeF shown = paper.size();
    shown.scale(QSizeF(mPreview->size()) - QSizeF(24, 24), Qt::KeepAspectRatio);
    const qreal scale = shown.width() / paper.width();
    const qreal ratio = devicePixelRatioF();

    QImage image((shown * ratio).toSize(), QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(ratio);
    image.fill(Qt::white);

    {
        QPainter painter(&image);
        const QRectF area(printable.left() * scale, printable.top() * scale, printable.width() * scale, printable.height() * scale);
        drawSheet(&painter, area, mShownSheet);
        painter.setPen(QPen(QColor(0, 0, 0, 60), 0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(QPointF(0, 0), shown).adjusted(0, 0, -1, -1));
    }

    if (mColor->currentData().toInt() == int(QPrinter::GrayScale))
    {
        image = image.convertToFormat(QImage::Format_Grayscale8);
    }

    mPreview->setPixmap(QPixmap::fromImage(image));
    QApplication::restoreOverrideCursor();
}

void UBPrintDialog::setUpPrinter(QPrinter* printer) const
{
    const QString name = mPrinter->currentData().toString();

    if (name == sPdfFile)
    {
        printer->setOutputFormat(QPrinter::PdfFormat);
    }
    else
    {
        printer->setPrinterName(name);
    }

    printer->setDocName(mDocument->name());
    printer->setCopyCount(mCopies->value());
    printer->setCollateCopies(mCollate->isChecked());
    printer->setColorMode(QPrinter::ColorMode(mColor->currentData().toInt()));

    if (mSides->isEnabled())
    {
        printer->setDuplex(QPrinter::DuplexMode(mSides->currentData().toInt()));
    }

    printer->setPageSize(paperSize());
}

void UBPrintDialog::printerProperties()
{
    // The dialog of the system, for the settings of the printer itself.
    if (mPrinter->currentData().toString() == sPdfFile)
    {
        return;
    }

    if (!mDriverSettings)
    {
        mDriverSettings = std::make_unique<QPrinter>(QPrinter::HighResolution);
    }

    setUpPrinter(mDriverSettings.get());
    QPrintDialog dialog(mDriverSettings.get(), this);
    dialog.setOptions(QAbstractPrintDialog::PrintShowPageSize);
    dialog.setWindowTitle(tr("Printer settings"));

    if (dialog.exec() == QDialog::Accepted)
    {
        // What was chosen there shows here too.
        mCopies->setValue(mDriverSettings->copyCount());
        const int paper = mPaper->findData(int(mDriverSettings->pageLayout().pageSize().id()));

        if (paper >= 0)
        {
            mPaper->setCurrentIndex(paper);
        }
    }
}

void UBPrintDialog::run()
{
    const QList<int> pages = chosenPages();

    if (pages.isEmpty())
    {
        mPagesText->setFocus();
        return;
    }

    QPrinter printer(QPrinter::HighResolution);

    if (mDriverSettings && mDriverSettings->printerName() == mPrinter->currentData().toString())
    {
        // Keeps what was set in the settings of the printer.
        printer.setPrinterName(mDriverSettings->printerName());
        printer.setPageLayout(mDriverSettings->pageLayout());
    }

    setUpPrinter(&printer);

    if (printer.outputFormat() == QPrinter::PdfFormat)
    {
        const QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const QString file = QFileDialog::getSaveFileName(this, tr("PDF file"), folder + "/" + mDocument->name() + ".pdf", tr("PDF (*.pdf)"));

        if (file.isEmpty())
        {
            return;
        }

        printer.setOutputFileName(file);
    }

    remember();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QPainter painter;
    bool started = false;
    const int sheets = sheetCount();

    for (int sheet = 0; sheet < sheets; ++sheet)
    {
        printer.setPageOrientation(sheetOrientation(sheet));

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

        drawSheet(&painter, QRectF(painter.viewport()), sheet);
    }

    if (started)
    {
        painter.end();
    }

    QApplication::restoreOverrideCursor();

    if (!started)
    {
        QMessageBox::warning(this, windowTitle(), tr("The printer could not be used. Check that it is on and connected."));
        return;
    }

    accept();
}

void UBPrintDialog::remember() const
{
    UBSettings* settings = UBSettings::settings();
    settings->setValue(sPrinterKey, mPrinter->currentData());
    settings->setValue(sPaperKey, mPaper->currentData());
    settings->setValue(sScaleKey, mScale->currentData());
    settings->setValue(sColorKey, mColor->currentData());
    settings->setValue(sSidesKey, mSides->currentData());
}
