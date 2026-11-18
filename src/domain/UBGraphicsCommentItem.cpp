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


#include "UBGraphicsCommentItem.h"

#include <QGraphicsScene>

#include "core/UBSettings.h"
#include "gui/UBCommentPopup.h"

static const QString sShownKey = QStringLiteral("Board/LibertyBoardCommentsShown");

UBGraphicsCommentItem::UBGraphicsCommentItem(const QString& text, const QDateTime& date, QGraphicsItem* parent)
    : UBGraphicsPageMarkItem(":/images/marks/comment.svg", parent)
    , mDate(date.isValid() ? date : QDateTime::currentDateTime())
{
    setText(text);
    setVisible(areShown());
}

QString UBGraphicsCommentItem::text() const
{
    return mText;
}

void UBGraphicsCommentItem::setText(const QString& text)
{
    if (text == mText)
    {
        return;
    }

    mText = text;
    setToolTip(text);
    markDocumentChanged();
}

QDateTime UBGraphicsCommentItem::date() const
{
    return mDate;
}

void UBGraphicsCommentItem::open()
{
    UBCommentPopup::showFor(this);
}

bool UBGraphicsCommentItem::areShown()
{
    return UBSettings::settings()->value(sShownKey, true).toBool();
}

void UBGraphicsCommentItem::setShown(bool shown)
{
    UBSettings::settings()->setValue(sShownKey, shown);
}

void UBGraphicsCommentItem::removeIfEmpty()
{
    if (mText.trimmed().isEmpty() && scene())
    {
        markDocumentChanged();
        scene()->removeItem(this);
        deleteLater();
    }
}
