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

#include "UBNoteDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

UBNoteDialog::UBNoteDialog(const QString& text, bool isNewNote, QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    setObjectName("noteDialog");
    setAttribute(Qt::WA_StyledBackground);
    setMinimumWidth(460);
    setStyleSheet(
        "QDialog#noteDialog { background: #20242b; border: 1px solid #3a404b; border-radius: 10px; }"
        "QLabel { color: #f0f0f0; font-size: 18px; font-weight: bold; }"
        "QPlainTextEdit { background: #20242b; color: #f0f0f0; border: 1px solid #5a6170; border-radius: 4px; padding: 6px; font-size: 14px; }"
        "QToolButton { color: #c8ccd4; background: transparent; border: none; font-size: 16px; }"
        "QPushButton { color: #f0f0f0; background: transparent; border: none; padding: 8px 18px; font-size: 14px; font-weight: bold; }"
        "QPushButton#confirm { background: #5b6b8c; border-radius: 18px; }"
        "QPushButton#confirm:disabled { background: #3d4452; color: #8a90a0; }"
        "QPushButton:hover { color: #ffffff; }");

    auto title = new QLabel(isNewNote ? tr("Add note") : tr("Edit note"), this);

    auto closeButton = new QToolButton(this);
    closeButton->setText(QStringLiteral("✕"));
    connect(closeButton, &QToolButton::clicked, this, &QDialog::reject);

    auto titleRow = new QHBoxLayout();
    titleRow->addWidget(title);
    titleRow->addStretch();
    titleRow->addWidget(closeButton);

    mText = new QPlainTextEdit(text, this);
    mText->setPlaceholderText(tr("Type the note here"));
    mText->setFixedHeight(100);

    auto secondary = new QPushButton(isNewNote ? tr("Cancel") : tr("Delete"), this);
    mConfirm = new QPushButton(isNewNote ? tr("Save") : tr("Update"), this);
    mConfirm->setObjectName("confirm");
    mConfirm->setDefault(true);

    auto buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(secondary);
    buttons->addWidget(mConfirm);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 18, 24, 18);
    layout->setSpacing(16);
    layout->addLayout(titleRow);
    layout->addWidget(mText);
    layout->addLayout(buttons);

    const auto updateConfirm = [this]() {
        mConfirm->setEnabled(!mText->toPlainText().trimmed().isEmpty());
    };
    connect(mText, &QPlainTextEdit::textChanged, this, updateConfirm);
    updateConfirm();

    connect(mConfirm, &QPushButton::clicked, this, [this]() {
        mResult = Result::Saved;
        accept();
    });
    connect(secondary, &QPushButton::clicked, this, [this, isNewNote]() {
        mResult = isNewNote ? Result::Cancelled : Result::Deleted;
        accept();
    });

    mText->setFocus();
}

UBNoteDialog::Result UBNoteDialog::ask()
{
    mResult = Result::Cancelled;
    exec();
    return mResult;
}

QString UBNoteDialog::text() const
{
    return mText->toPlainText().trimmed();
}
