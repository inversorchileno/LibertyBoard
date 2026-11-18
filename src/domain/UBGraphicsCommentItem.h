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

#include <QDateTime>

#include "core/UB.h"
#include "domain/UBGraphicsPageMarkItem.h"

/**
 * @brief A quick note on a page: a yellow speech bubble that opens a small
 * window with its date and its text, like the comments of a PDF reader.
 *
 * All the quick notes can be hidden or shown at once.
 */
class UBGraphicsCommentItem : public UBGraphicsPageMarkItem
{
    Q_OBJECT

public:
    enum { Type = UBGraphicsItemType::CommentItemType };

    explicit UBGraphicsCommentItem(const QString& text = QString(), const QDateTime& date = QDateTime::currentDateTime(),
                                   QGraphicsItem* parent = nullptr);

    int type() const override { return Type; }

    QString text() const;
    void setText(const QString& text);

    QDateTime date() const;

    /** @brief Opens the window of the note. */
    void open() override;

    /** @brief True when the quick notes are shown on the pages. */
    static bool areShown();

    /** @brief Shows or hides the quick notes of the pages loaded. */
    static void setShown(bool shown);

    /** @brief Takes the note off the page without asking, when it was left empty. */
    void removeIfEmpty();

private:
    QString mText;
    QDateTime mDate;
};
