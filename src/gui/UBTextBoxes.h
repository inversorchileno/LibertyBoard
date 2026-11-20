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

#include <QCoreApplication>

class QWidget;
class UBBoardView;
class UBGraphicsTextItem;

/**
 * @brief The text boxes of the board, or of the desktop in the desktop
 * mode: the one being written, a new one, and the tables put in them.
 */
class UBTextBoxes
{
    Q_DECLARE_TR_FUNCTIONS(UBTextBoxes)

public:
    /** @brief The view drawn on: the board, or the desktop in the desktop mode. */
    static UBBoardView* activeView();

    /** @brief The text box being written: the one with the cursor, or the one selected; or none. */
    static UBGraphicsTextItem* current();

    /** @brief A new text box in the middle of the view, ready to be written. */
    static UBGraphicsTextItem* create();

    /** @brief Asks how many rows and columns, then puts a table in the text box being written, or in a new one. */
    static void insertTable(QWidget* parent);
};
