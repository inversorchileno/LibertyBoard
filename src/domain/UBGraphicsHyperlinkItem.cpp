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

#include "UBGraphicsHyperlinkItem.h"

#include <QDesktopServices>
#include <QInputDialog>
#include <QMessageBox>
#include <QUrl>

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"

UBGraphicsHyperlinkItem::UBGraphicsHyperlinkItem(const QString& address, QGraphicsItem* parent)
    : UBGraphicsPageMarkItem(":/images/marks/hyperlink.svg", parent)
    , mAddress(address)
{
    setToolTip(address);
}

QString UBGraphicsHyperlinkItem::address() const
{
    return mAddress;
}

void UBGraphicsHyperlinkItem::open()
{
    if (!QDesktopServices::openUrl(QUrl::fromUserInput(mAddress)))
    {
        QMessageBox::warning(UBApplication::mainWindow, tr("Hyperlink"), tr("%1 could not be opened.").arg(mAddress));
    }
}

QString UBGraphicsHyperlinkItem::askAddress(QWidget* parent)
{
    // Kept above the desktop annotation layer, which stays on top of all windows.
    QInputDialog dialog(parent);
    dialog.setWindowTitle(tr("Add a hyperlink"));
    dialog.setLabelText(tr("Address of the web page:"));
    dialog.setTextValue(QStringLiteral("https://"));
    dialog.setWindowFlag(Qt::WindowStaysOnTopHint);
    dialog.resize(520, dialog.sizeHint().height());

    if (dialog.exec() != QDialog::Accepted)
    {
        return QString();
    }

    const QString address = dialog.textValue().trimmed();

    if (address.isEmpty() || address == QStringLiteral("https://"))
    {
        return QString();
    }

    return QUrl::fromUserInput(address).toString();
}
