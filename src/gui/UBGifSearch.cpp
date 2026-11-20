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


#include "UBGifSearch.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QUrlQuery>

#include "core/UBSettings.h"

namespace
{
    // The search in the form of Tenor, which KLIPY took over, and its own form.
    QUrl tenorSearch(const QString& words, const QString& key)
    {
        QUrl url("https://api.klipy.com/v2/search");
        QUrlQuery query;
        query.addQueryItem("q", words);
        query.addQueryItem("key", key);
        query.addQueryItem("limit", "24");
        query.addQueryItem("media_filter", "gif,tinygif");
        query.addQueryItem("client_key", "libertyboard");
        url.setQuery(query);
        return url;
    }

    QUrl klipySearch(const QString& words, const QString& key)
    {
        QUrl url("https://api.klipy.com/api/v1/" + QString::fromLatin1(QUrl::toPercentEncoding(key)) + "/gifs/search");
        QUrlQuery query;
        query.addQueryItem("q", words);
        query.addQueryItem("per_page", "24");
        query.addQueryItem("customer_id", "libertyboard");
        url.setQuery(query);
        return url;
    }

    QUrl urlAt(const QJsonObject& object, const QStringList& path)
    {
        QJsonValue value = object;

        for (const QString& key : path)
        {
            value = value.toObject().value(key);
        }

        return QUrl(value.toString());
    }
}

UBGifSearch::UBGifSearch(QObject* parent)
    : QObject(parent)
    , mNetwork(new QNetworkAccessManager(this))
{
}

void UBGifSearch::search(const QString& words, const QString& key)
{
    if (mSearch)
    {
        mSearch->abort();
    }

    QNetworkReply* reply = mNetwork->get(QNetworkRequest(tenorSearch(words, key)));
    mSearch = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, words, key]() { onSearchFinished(reply, words, key, true); });
}

void UBGifSearch::onSearchFinished(QNetworkReply* reply, const QString& words, const QString& key, bool firstTry)
{
    reply->deleteLater();

    if (reply != mSearch)
    {
        return;
    }

    mSearch = nullptr;

    if (reply->error() == QNetworkReply::OperationCanceledError)
    {
        return;
    }

    const QList<Gif> gifs = reply->error() == QNetworkReply::NoError ? gifsOf(reply->readAll()) : QList<Gif>();

    // Nothing in the form of Tenor: the form of KLIPY.
    if (gifs.isEmpty() && firstTry)
    {
        QNetworkReply* second = mNetwork->get(QNetworkRequest(klipySearch(words, key)));
        mSearch = second;
        connect(second, &QNetworkReply::finished, this, [this, second, words, key]() { onSearchFinished(second, words, key, false); });
        return;
    }

    if (reply->error() != QNetworkReply::NoError && gifs.isEmpty())
    {
        qWarning() << "GIF search failed:" << reply->errorString();
        emit failed();
        return;
    }

    emit found(gifs);
}

QList<UBGifSearch::Gif> UBGifSearch::gifsOf(const QByteArray& json)
{
    QList<Gif> gifs;
    const QJsonObject root = QJsonDocument::fromJson(json).object();

    // The form of Tenor: results[].media_formats.{tinygif,gif}.url
    for (const QJsonValue& value : root.value("results").toArray())
    {
        const QJsonObject result = value.toObject();
        Gif gif{ result.value("id").toVariant().toString(), urlAt(result, { "media_formats", "tinygif", "url" }),
                 urlAt(result, { "media_formats", "gif", "url" }) };

        if (gif.full.isValid() && !gif.full.isEmpty())
        {
            gifs << (gif.preview.isEmpty() ? Gif{ gif.id, gif.full, gif.full } : gif);
        }
    }

    // The form of KLIPY: data.data[].file.{xs,sm,md}.gif.url
    for (const QJsonValue& value : root.value("data").toObject().value("data").toArray())
    {
        const QJsonObject result = value.toObject();
        QUrl preview = urlAt(result, { "file", "xs", "gif", "url" });

        if (preview.isEmpty())
        {
            preview = urlAt(result, { "file", "sm", "gif", "url" });
        }

        const QUrl full = urlAt(result, { "file", "md", "gif", "url" });

        if (!full.isEmpty())
        {
            gifs << Gif{ result.value("id").toVariant().toString(), preview.isEmpty() ? full : preview, full };
        }
    }

    return gifs;
}

void UBGifSearch::downloadPreview(const Gif& gif)
{
    QNetworkReply* reply = mNetwork->get(QNetworkRequest(gif.preview));
    const QString id = gif.id;

    connect(reply, &QNetworkReply::finished, this, [this, reply, id]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError)
        {
            emit previewReady(id, reply->readAll());
        }
    });
}

void UBGifSearch::downloadGif(const Gif& gif)
{
    QNetworkReply* reply = mNetwork->get(QNetworkRequest(gif.full));
    QString name = gif.id;
    name.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");

    connect(reply, &QNetworkReply::finished, this, [this, reply, name]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
        {
            emit failed();
            return;
        }

        // Kept, so that the GIF can be put again without the internet.
        const QString folder = UBSettings::userDataDirectory() + "/gifs";
        QDir().mkpath(folder);
        const QString path = folder + "/" + (name.isEmpty() ? QStringLiteral("gif") : name) + ".gif";
        QFile file(path);

        if (!file.open(QIODevice::WriteOnly) || file.write(reply->readAll()) < 0)
        {
            emit failed();
            return;
        }

        file.close();
        emit gifReady(path);
    });
}
