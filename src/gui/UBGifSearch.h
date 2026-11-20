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
#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

/**
 * @brief The GIF search of KLIPY, with the key of the user: each user makes
 * a free account once and pastes the key in LibertyBoard, so that no
 * account is shared by all.
 *
 * It searches, downloads the small previews, and downloads the GIF chosen
 * among the data of LibertyBoard.
 */
class UBGifSearch : public QObject
{
    Q_OBJECT

public:
    struct Gif
    {
        QString id;
        QUrl preview;   // a small GIF, for the list
        QUrl full;      // the GIF put on the page
    };

    explicit UBGifSearch(QObject* parent = nullptr);

    /** @brief Searches GIFs with these words, with the key of the user. */
    void search(const QString& words, const QString& key);

    /** @brief Downloads a preview, given back by previewReady. */
    void downloadPreview(const Gif& gif);

    /** @brief Downloads the GIF, kept among the data of LibertyBoard, given back by gifReady. */
    void downloadGif(const Gif& gif);

signals:
    void found(const QList<UBGifSearch::Gif>& gifs);
    void failed();
    void previewReady(const QString& id, const QByteArray& data);
    void gifReady(const QString& file);

private:
    void onSearchFinished(QNetworkReply* reply, const QString& words, const QString& key, bool firstTry);
    static QList<Gif> gifsOf(const QByteArray& json);

    QNetworkAccessManager* mNetwork{nullptr};
    QNetworkReply* mSearch{nullptr};
};
