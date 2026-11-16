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

#include "UBPagesSidebar.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QVBoxLayout>

#include "core/UBSetting.h"
#include "core/UBSettings.h"

static constexpr int sPagesWidth = 220;

UBPagesSidebar::UBPagesSidebar(QWidget* pages, QWidget* parent)
    : QWidget(parent)
    , mPages(pages)
{
    setObjectName("pagesSidebar");
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("QWidget#pagesSidebar { background: #20242b; }"
                  "QToolButton { color: #e8e8e8; background: transparent; border: none; font-size: 12px; }"
                  "QToolButton:hover { background: #3d4452; }");

    mPages->setParent(this);

    mToggleButton = new QToolButton(this);
    mToggleButton->setFixedWidth(18);
    mToggleButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    mToggleButton->setToolTip(tr("Hide or show the pages"));

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mPages);
    layout->addWidget(mToggleButton);

    connect(mToggleButton, &QToolButton::clicked, this, [this]() {
        setPagesVisible(!mPages->isVisible());
    });

    setPagesVisible(UBSettings::settings()->value("Board/PagesSidebarVisible", true).toBool());
}

void UBPagesSidebar::setPagesVisible(bool visible)
{
    mPages->setVisible(visible);
    setFixedWidth(visible ? sPagesWidth + mToggleButton->width() : mToggleButton->width());
    mToggleButton->setText(visible ? QStringLiteral("◀") : QStringLiteral("▶"));
    UBSettings::settings()->setValue("Board/PagesSidebarVisible", visible);
}
