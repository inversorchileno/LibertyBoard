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

#include <QFrame>

class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * @brief The index of the PDF shown on the board, as its author wrote it:
 * a click on a title goes to its page. It floats on the right of the board.
 */
class UBPdfOutlinePanel : public QFrame
{
    Q_OBJECT

public:
    explicit UBPdfOutlinePanel(QWidget* parent);

public slots:
    /** @brief Shows the index, or a message when there is none. */
    void open();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void goTo(QTreeWidgetItem* item);
    void placeInWindow();

    QLabel* mTitle{nullptr};
    QTreeWidget* mTree{nullptr};
    QString mPdfFile;
};
