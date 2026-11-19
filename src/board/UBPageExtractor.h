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

#include <QDialog>
#include <QList>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;

/**
 * @brief The dialog of Home > Extract pages: writes some pages of the
 * document shown, with their annotations, to a new PDF or to pictures.
 */
class UBPageExtractor : public QDialog
{
    Q_OBJECT

public:
    /** @brief Shows the dialog for the document of the board. */
    static void extract(QWidget* parent);

    /**
     * @brief The pages, counted from 0, of a text like "1-3, 5, 8-" for a
     * document of pageCount pages. Empty when the text is not valid.
     */
    static QList<int> parsePages(const QString& text, int pageCount);

private:
    explicit UBPageExtractor(QWidget* parent);

    enum Format
    {
        OnePdf,
        PdfPerPage,
        PngPerPage,
        JpegPerPage
    };

    QList<int> chosenPages() const;
    QStringList filesFor(const QList<int>& pages) const;
    void browse();
    void run();
    bool writePdf(const QList<int>& pages, const QString& filename) const;
    bool writeImage(int page, const QString& filename, const char* format) const;

    int mPageCount{0};
    int mCurrentPage{0};
    QButtonGroup* mRange{nullptr};
    QLineEdit* mPagesText{nullptr};
    QComboBox* mFormat{nullptr};
    QLineEdit* mName{nullptr};
    QLineEdit* mFolder{nullptr};
    QCheckBox* mOpenWhenDone{nullptr};
};
