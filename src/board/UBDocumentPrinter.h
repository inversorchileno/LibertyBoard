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


#pragma once

#include <memory>

#include <QDialog>
#include <QImage>
#include <QList>
#include <QPageLayout>
#include <QPageSize>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPainter;
class QPrinter;
class QSpinBox;
class QToolButton;
class UBDocumentProxy;

/**
 * @brief Prints the document of the board, through a dialog of its own with
 * a preview of each sheet: the printer, the copies, the pages, the paper,
 * the orientation, the scale, the pages on each sheet, color or gray, both
 * sides, and whether the annotations are printed.
 */
namespace UBDocumentPrinter
{
    void print(QWidget* dialogParent);
}

class UBPrintDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UBPrintDialog(QWidget* parent);
    ~UBPrintDialog() override;

private:
    enum Orientation { Automatic, Portrait, Landscape };

    QList<int> chosenPages() const;
    int pagesPerSheet() const;
    int sheetCount() const;
    QPageSize paperSize() const;
    QPageLayout::Orientation sheetOrientation(int sheet) const;
    void drawSheet(QPainter* painter, const QRectF& area, int sheet) const;
    void drawPage(QPainter* painter, const QRectF& cell, int page) const;
    void updatePreview();
    void showSheet(int sheet);
    void setUpPrinter(QPrinter* printer) const;
    void printerProperties();
    void run();
    void remember() const;

    std::shared_ptr<UBDocumentProxy> mDocument;
    int mPageCount{0};
    int mCurrentPage{0};
    int mShownSheet{0};

    QComboBox* mPrinter{nullptr};
    QSpinBox* mCopies{nullptr};
    QCheckBox* mCollate{nullptr};
    QButtonGroup* mRange{nullptr};
    QLineEdit* mPagesText{nullptr};
    QComboBox* mPaper{nullptr};
    QComboBox* mOrientation{nullptr};
    QComboBox* mScale{nullptr};
    QComboBox* mPerSheet{nullptr};
    QComboBox* mColor{nullptr};
    QComboBox* mSides{nullptr};
    QCheckBox* mAnnotations{nullptr};
    QLabel* mPreview{nullptr};
    QLabel* mSheetLabel{nullptr};
    QToolButton* mPrevious{nullptr};
    QToolButton* mNext{nullptr};
    QLabel* mWarning{nullptr};
    std::unique_ptr<QPrinter> mDriverSettings;
};
