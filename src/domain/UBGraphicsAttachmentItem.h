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

#include "core/UB.h"
#include "domain/UBGraphicsPageMarkItem.h"

/**
 * @brief A file attached to a page, shown as a paper clip. A click opens the
 * file with the program of the system for its kind, Excel for a workbook
 * for example.
 *
 * The file is copied into the document, so that the document keeps it when
 * it is copied, and the original can be moved or deleted.
 */
class UBGraphicsAttachmentItem : public UBGraphicsPageMarkItem
{
    Q_OBJECT

public:
    enum { Type = UBGraphicsItemType::AttachmentItemType };

    /** @param file The path of the copy, relative to the folder of the document. */
    explicit UBGraphicsAttachmentItem(const QString& file, QGraphicsItem* parent = nullptr);

    int type() const override { return Type; }

    QString file() const;

    void open() override;

    /**
     * @brief Copies a file into the folder of a document.
     * Returns its path relative to that folder, or an empty string on failure.
     */
    static QString copyIntoDocument(const QString& sourceFile, const QString& documentFolder, const QString& subFolder);

private:
    QString mFile;
};
