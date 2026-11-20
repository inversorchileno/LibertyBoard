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
 * @brief A link to a web page put on a page, shown as two links of a
 * chain. A click opens the address in the browser of the system.
 */
class UBGraphicsHyperlinkItem : public UBGraphicsPageMarkItem
{
    Q_OBJECT

public:
    enum { Type = UBGraphicsItemType::HyperlinkItemType };

    explicit UBGraphicsHyperlinkItem(const QString& address, QGraphicsItem* parent = nullptr);

    int type() const override { return Type; }

    QString address() const;

    void open() override;

    /** @brief Asks for an address, with https:// added when missing; empty when given up. */
    static QString askAddress(QWidget* parent);

private:
    QString mAddress;
};
