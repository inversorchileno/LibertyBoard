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
#include <QScreen>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWebEngineView>

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
