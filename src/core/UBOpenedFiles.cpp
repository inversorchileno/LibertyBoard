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


#include "UBOpenedFiles.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QVariantMap>

#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"

static const QString sDocumentsKey = QStringLiteral("App/LibertyBoardFileDocuments");
static const QString sRecentKey = QStringLiteral("App/LibertyBoardRecentFiles");
static constexpr int sRecentCount = 10;

/** What tells that a file did not change: its date and its size. */
static QString stampOf(const QFileInfo& file)
{
    return QString("%1/%2").arg(file.lastModified().toMSecsSinceEpoch()).arg(file.size());
}

void UBOpenedFiles::remember(const QString& filePath, const std::shared_ptr<UBDocumentProxy>& document)
{
    const QFileInfo file(filePath);

    if (!document || !file.exists())
    {
        return;
    }

    const QString path = file.absoluteFilePath();
    UBSettings* settings = UBSettings::settings();

    QVariantMap documents = settings->value(sDocumentsKey).toMap();
    documents.insert(path, QStringList{ document->documentFolderName(), stampOf(file) });
    settings->setValue(sDocumentsKey, documents);

    QStringList recent = settings->value(sRecentKey).toStringList();
    recent.removeAll(path);
    recent.prepend(path);

    while (recent.size() > sRecentCount)
    {
        recent.removeLast();
    }

    settings->setValue(sRecentKey, recent);
}

std::shared_ptr<UBDocumentProxy> UBOpenedFiles::documentOf(const QString& filePath)
{
    const QFileInfo file(filePath);
    const QStringList known = UBSettings::settings()->value(sDocumentsKey).toMap().value(file.absoluteFilePath()).toStringList();

    if (known.size() != 2 || !file.exists() || known.at(1) != stampOf(file))
    {
        return nullptr;
    }

    UBPersistenceManager* persistence = UBPersistenceManager::persistenceManager();
    std::shared_ptr<UBDocumentProxy> document = persistence->mDocumentTreeStructureModel->findDocumentByFolderName(known.at(0));

    // Only a document still in the library, not in the trash.
    if (!document || !QDir(document->persistencePath()).exists()
            || persistence->mDocumentTreeStructureModel->inTrash(persistence->mDocumentTreeStructureModel->indexForProxy(document)))
    {
        return nullptr;
    }

    return document;
}

QStringList UBOpenedFiles::recentFiles()
{
    QStringList files;

    for (const QString& file : UBSettings::settings()->value(sRecentKey).toStringList())
    {
        if (QFileInfo::exists(file))
        {
            files << file;
        }
    }

    return files;
}
