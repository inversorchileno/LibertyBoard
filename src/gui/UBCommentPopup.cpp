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


#include "UBCommentPopup.h"

#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPlainTextEdit>
#include <QScreen>
#include <QShortcut>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "domain/UBGraphicsCommentItem.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBMainWindow.h"

void UBCommentPopup::showFor(UBGraphicsCommentItem* comment)
{
    static QPointer<UBCommentPopup> popup;

    if (!popup)
    {
        popup = new UBCommentPopup(UBApplication::mainWindow);
    }

    popup->hide();
    popup->setComment(comment);

    // Beside the icon, on the screen where it is.
    QPoint position = QCursor::pos() + QPoint(24, -12);
    UBBoardView* view = UBApplication::boardController ? UBApplication::boardController->controlView() : nullptr;

    if (view && view->scene() && comment->scene() == static_cast<QGraphicsScene*>(view->scene().get()))
    {
        position = view->viewport()->mapToGlobal(view->mapFromScene(comment->scenePos())) + QPoint(24, -12);
    }

    popup->adjustSize();
    const QScreen* screen = QGuiApplication::screenAt(position);
    const QRect available = screen ? screen->availableGeometry() : QRect(position, popup->size());
    position.setX(qMin(position.x(), available.right() - popup->width()));
    position.setY(qBound(available.top(), position.y(), available.bottom() - popup->height()));

    popup->move(position);
    popup->show();
    popup->raise();
    popup->activateWindow();
    popup->mText->setFocus();
}

UBCommentPopup::UBCommentPopup(QWidget* parent)
    : QFrame(parent, Qt::Tool | Qt::FramelessWindowHint)
{
    setObjectName("commentPopup");
    setStyleSheet(
        "#commentPopup { background: #fff8b8; border: 1px solid #c9a800; border-radius: 4px; }"
        "QLabel { color: #3a3200; }"
        "QLabel#title { font-weight: bold; }"
        "QPlainTextEdit { background: #fffde8; color: #1e1e1e; border: 1px solid #d8c24a; }"
        "QToolButton { border: none; color: #6b5c00; font-size: 14px; padding: 0 4px; }"
        "QToolButton:hover { background: #f3e27a; }");

    auto icon = new QLabel(this);
    icon->setPixmap(QIcon(":/images/marks/comment.svg").pixmap(QSize(22, 22)));

    auto title = new QLabel(tr("Quick note"), this);
    title->setObjectName("title");

    mDate = new QLabel(this);

    auto close = new QToolButton(this);
    close->setText(QStringLiteral("\u2715"));
    close->setToolTip(tr("Close"));

    auto header = new QHBoxLayout();
    header->setSpacing(6);
    header->addWidget(icon);
    header->addWidget(title);
    header->addWidget(mDate, 1);
    header->addWidget(close);

    mText = new QPlainTextEdit(this);
    mText->setMinimumSize(280, 110);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 8);
    layout->setSpacing(6);
    layout->addLayout(header);
    layout->addWidget(mText);

    connect(close, &QToolButton::clicked, this, &QWidget::hide);
    connect(new QShortcut(QKeySequence(Qt::Key_Escape), this), &QShortcut::activated, this, &QWidget::hide);

    // The window belongs to the page shown: it closes when another one is shown.
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &QWidget::hide);
    connect(mText, &QPlainTextEdit::textChanged, this, [this]() {
        if (mComment)
        {
            mComment->setText(mText->toPlainText());
        }
    });
}

void UBCommentPopup::setComment(UBGraphicsCommentItem* comment)
{
    mComment = nullptr;
    mText->setPlainText(comment->text());
    mDate->setText(QLocale().toString(comment->date(), QLocale::ShortFormat));
    mComment = comment;
}

void UBCommentPopup::hideEvent(QHideEvent* event)
{
    if (mComment)
    {
        mComment->removeIfEmpty();
    }

    mComment = nullptr;
    QFrame::hideEvent(event);
}
