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

class QWidget;

/**
 * @brief Reads the text of the scanned pages of the document of the board,
 * page by page, with a progress dialog, so that the search and the copy of
 * the text work on them and a saved PDF holds their text.
 */
namespace UBOcrRunner
{
    /** @brief Home > Recognize text: reads every page of the document that has no text. */
    void run(QWidget* parent);

    /** @brief After a PDF is opened: offers to read it when its first pages are pictures only. */
    void offerIfScanned(QWidget* parent);
}
