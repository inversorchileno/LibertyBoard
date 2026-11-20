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


#include "UBGifWindow.h"

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QScreen>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QImage>
#include <QWebEnginePage>
#include <QApplication>

QPointer<UBGifWindow> UBGifWindow::sWindow;

UBGifWindow::UBGifWindow(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("gifWindow");
    setStyleSheet("QWidget#gifWindow { background: #23262b; border: 1px solid #5a606a; }"
                  "QLabel { color: white; }"
                  "QToolButton { color: white; background: transparent; border: none; font-size: 18px; padding: 0 6px; }"
                  "QToolButton:hover { background: #c42b1c; }");

    // A title and the closing cross, at the top right.
    auto title = new QLabel(tr("GIFs of klipy.com: drag one to the page"), this);
    auto close = new QToolButton(this);
    close->setText(QStringLiteral("✕"));
    close->setToolTip(tr("Close the GIFs"));
    connect(close, &QToolButton::clicked, this, &QWidget::close);

    auto header = new QHBoxLayout();
    header->setContentsMargins(8, 4, 2, 4);
    header->addWidget(title, 1);
    header->addWidget(close);

    // The page of KLIPY as it is, with its search at the top.
    mPage = new QWebEngineView(this);
    mPage->load(QUrl("https://klipy.com/gifs"));

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addLayout(header);
    layout->addWidget(mPage, 1);
}

void UBGifWindow::toggle(QWidget* beside)
{
    if (sWindow)
    {
        sWindow->close();
        return;
    }

    sWindow = new UBGifWindow(beside ? beside->window() : nullptr);
    sWindow->placeBeside(beside);
    sWindow->show();
}

bool UBGifWindow::isOpen()
{
    return !sWindow.isNull();
}

UBGifWindow* UBGifWindow::instance()
{
    return sWindow;
}

void UBGifWindow::placeBeside(QWidget* beside)
{
    // A narrow column at the right of the window, or of the screen.
    QRect area = beside ? beside->window()->geometry() : QRect();
    QScreen* screen = beside ? beside->screen() : nullptr;

    if (screen && (area.isEmpty() || beside->window()->isFullScreen() || area.width() >= screen->availableGeometry().width()))
    {
        area = screen->availableGeometry();
    }

    if (area.isEmpty())
    {
        resize(380, 700);
        return;
    }

    const int width = qBound(320, area.width() / 5, 460);
    const int top = area.top() + qMin(140, area.height() / 6);
    setGeometry(area.right() - width - 60, top, width, area.bottom() - top - 20);
}

void UBGifWindow::closeEvent(QCloseEvent* event)
{
    // Closed from now on, though deleted a little later.
    if (sWindow == this)
    {
        sWindow.clear();
    }

    emit closed();
    QWidget::closeEvent(event);
}

bool UBGifWindow::isGifPage(const QUrl& url)
{
    const QString host = url.host().toLower();
    return (host == "klipy.com" || host.endsWith(".klipy.com")) && !url.path().contains('.')
           && (url.path().startsWith("/gifs/") || url.path().startsWith("/stickers/"));
}

// The addresses of pictures written in a page, its shared picture first.
static QStringList picturesInPage(QString html)
{
    // The addresses written in the data of the page have their slashes escaped.
    html.replace("\\/", "/");

    static const QRegularExpression shared(
        "<meta[^>]+(?:property|name)=[\"'](?:og:image|twitter:image|og:video)[\"'][^>]+content=[\"']([^\"']+)[\"']",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression sharedFirst(
        "<meta[^>]+content=[\"']([^\"']+)[\"'][^>]+(?:property|name)=[\"'](?:og:image|twitter:image|og:video)[\"']",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression anyPicture("https://[^\"'\\s<>]+?\\.(?:gif|webp|mp4|webm)(?:\\?[^\"'\\s<>]*)?",
                                               QRegularExpression::CaseInsensitiveOption);
    QStringList addresses;

    for (const QRegularExpression& pattern : { shared, sharedFirst })
    {
        QRegularExpressionMatchIterator it = pattern.globalMatch(html);

        while (it.hasNext())
        {
            addresses << it.next().captured(1).replace("&amp;", "&");
        }
    }

    QRegularExpressionMatchIterator it = anyPicture.globalMatch(html);

    while (it.hasNext())
    {
        addresses << it.next().captured(0).replace("&amp;", "&");
    }

    return addresses;
}

namespace
{
    // Tries the addresses one after the other, until one gives a picture.
    class GifFinder : public QObject
    {
    public:
        GifFinder(const QUrl& page, QObject* context, const std::function<void(const QByteArray&)>& found)
            : QObject(context)
            , mPage(page)
            , mFound(found)
        {
        }

        void addCandidates(const QStringList& addresses)
        {
            for (const QString& address : addresses)
            {
                const QUrl url(address);

                if (!url.isValid() || url.scheme() != "https")
                {
                    continue;
                }

                // A GIF first: the same file is often there as a GIF, a WebP and a video.
                const QString path = url.path();
                const int dot = path.lastIndexOf('.');

                if (dot > 0 && !path.endsWith(".gif", Qt::CaseInsensitive))
                {
                    QUrl gif = url;
                    gif.setPath(path.left(dot) + ".gif");
                    add(gif);
                }

                if (!path.endsWith(".mp4", Qt::CaseInsensitive) && !path.endsWith(".webm", Qt::CaseInsensitive))
                {
                    add(url);
                }
            }
        }

        void start()
        {
            // Then what the page itself names, asked for as a browser does.
            QNetworkReply* reply = network()->get(request(mPage));

            connect(reply, &QNetworkReply::finished, this, [this, reply]() {
                reply->deleteLater();

                if (reply->error() == QNetworkReply::NoError)
                {
                    addCandidates(picturesInPage(QString::fromUtf8(reply->readAll())));
                }

                tryNext();
            });
        }

    private:
        static QNetworkAccessManager* network()
        {
            static QNetworkAccessManager* manager = new QNetworkAccessManager(qApp);
            return manager;
        }

        QNetworkRequest request(const QUrl& url) const
        {
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                                                                "(KHTML, like Gecko) Chrome/120.0 Safari/537.36");
            request.setRawHeader("Referer", "https://klipy.com/");
            request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
            return request;
        }

        void add(const QUrl& url)
        {
            if (!mCandidates.contains(url) && !mTried.contains(url))
            {
                mCandidates << url;
            }
        }

        void tryNext()
        {
            if (mCandidates.isEmpty())
            {
                mFound(QByteArray());
                deleteLater();
                return;
            }

            const QUrl url = mCandidates.takeFirst();
            mTried << url;
            QNetworkReply* reply = network()->get(request(url));

            connect(reply, &QNetworkReply::finished, this, [this, reply]() {
                reply->deleteLater();
                const QByteArray data = reply->error() == QNetworkReply::NoError ? reply->readAll() : QByteArray();

                if (!QImage::fromData(data).isNull())
                {
                    mFound(data);
                    deleteLater();
                    return;
                }

                tryNext();
            });
        }

        QUrl mPage;
        std::function<void(const QByteArray&)> mFound;
        QList<QUrl> mCandidates;
        QList<QUrl> mTried;
    };
}

void UBGifWindow::findGif(const QUrl& page, QObject* context, const std::function<void(const QByteArray&)>& found)
{
    auto finder = new GifFinder(page, context, found);

    if (!sWindow)
    {
        finder->start();
        return;
    }

    // First the pictures of the GIF dragged, as the window shows them:
    // the page of KLIPY makes them when it runs, so they are not in the
    // page as it is downloaded.
    const QString slug = page.path().section('/', -1);
    const QString script = QStringLiteral(R"((function() {
        var found = [];
        var links = document.querySelectorAll('a[href*="%1"]');
        links.forEach(function(link) {
            link.querySelectorAll('img, video, source').forEach(function(media) {
                [media.currentSrc, media.src, media.poster, media.getAttribute('data-src')].forEach(function(address) {
                    if (address) { found.push(address); }
                });
                (media.srcset || '').split(',').forEach(function(part) {
                    var address = part.trim().split(' ')[0];
                    if (address) { found.push(address); }
                });
            });
        });
        return found;
    })())").arg(slug);

    sWindow->mPage->page()->runJavaScript(script, [finder](const QVariant& result) {
        finder->addCandidates(result.toStringList());
        finder->start();
    });
}
