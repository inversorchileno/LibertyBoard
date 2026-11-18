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

#include <QList>
#include <QPointer>

class QWidget;
class UBGraphicsCommentItem;

/**
 * @brief The quick notes of the document on the board, taken all together:
 * going from one to the next across the pages, listing them, hiding them
 * and merging them into the pages.
 */
namespace UBComments
{
    struct Entry
    {
        int page{0};
        QPointer<UBGraphicsCommentItem> comment;
    };

    /** @brief The quick notes of the document, page by page, from the top of each page. */
    QList<Entry> all();

    /** @brief Shows the next quick note (or the previous one when direction is negative) and opens it. */
    void step(int direction, QWidget* parent);

    /** @brief Shows the page of the note, brings the note to the center and opens it. */
    void show(const Entry& entry);

    /** @brief A list of the quick notes, to go to one of them. */
    void showList(QWidget* parent);

    /** @brief Shows or hides the quick notes of all the pages. */
    void setShown(bool shown);

    /**
     * @brief After asking, replaces each quick note by a picture of its
     * window with its date and text, which becomes part of the page.
     */
    void merge(QWidget* parent);
}
