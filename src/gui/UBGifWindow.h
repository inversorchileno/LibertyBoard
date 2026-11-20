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

#include <QPointer>
#include <QUrl>

#include <functional>
#include <QWidget>

class QWebEngineView;

/**
 * @brief A narrow window at the right of the board with the web page of
 * KLIPY, to search GIFs without an account and drag them to the page,
 * where they move. A button at its top right closes it.
 */
class UBGifWindow : public QWidget
{
    Q_OBJECT

public:
    /** @brief Opens the window, or closes it when it is open. */
    static void toggle(QWidget* beside);

    static bool isOpen();

    /** @brief The one window, while it is open; for its signal closed. */
    static UBGifWindow* instance();

    /** @brief Whether this is the address of the page of a GIF of klipy.com, rather than of the GIF. */
    static bool isGifPage(const QUrl& url);

    /**
     * @brief Finds the GIF of a page of klipy.com, dragged from the window,
     * and downloads it; then calls found with its data, or with empty data
     * when it could not be found.
     */
    static void findGif(const QUrl& page, QObject* context, const std::function<void(const QByteArray&)>& found);

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    explicit UBGifWindow(QWidget* parent = nullptr);
    void placeBeside(QWidget* beside);

    QWebEngineView* mPage{nullptr};
    static QPointer<UBGifWindow> sWindow;
};
