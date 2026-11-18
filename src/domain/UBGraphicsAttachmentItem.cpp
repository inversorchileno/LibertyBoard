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


#include "UBGraphicsAttachmentItem.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrl>
#include <QUuid>

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"

UBGraphicsAttachmentItem::UBGraphicsAttachmentItem(const QString& file, QGraphicsItem* parent)
    : UBGraphicsPageMarkItem(":/images/marks/attachment.svg", parent)
    , mFile(file)
{
    setToolTip(QFileInfo(file).fileName());
}

QString UBGraphicsAttachmentItem::file() const
{
    return mFile;
}

void UBGraphicsAttachmentItem::open()
{
    const QString path = documentFolder() + "/" + mFile;

    if (!QFileInfo::exists(path) || !QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
    {
        QMessageBox::warning(UBApplication::mainWindow, tr("Attached file"),
                             tr("%1 could not be opened.").arg(QFileInfo(mFile).fileName()));
    }
}

QString UBGraphicsAttachmentItem::copyIntoDocument(const QString& sourceFile, const QString& documentFolder,
                                                   const QString& subFolder)
{
    if (documentFolder.isEmpty())
    {
        return QString();
    }

    // A folder of its own keeps the name of the file, which the program that opens it shows.
    const QString relativeFolder = subFolder + "/" + QUuid::createUuid().toString(QUuid::WithoutBraces);

    if (!QDir().mkpath(documentFolder + "/" + relativeFolder))
    {
        return QString();
    }

    const QString relativeFile = relativeFolder + "/" + QFileInfo(sourceFile).fileName();
    return QFile::copy(sourceFile, documentFolder + "/" + relativeFile) ? relativeFile : QString();
}
