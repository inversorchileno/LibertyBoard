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

#include <QWidget>

class QToolButton;

/**
 * @brief The page thumbnails at the left of the board, which a click on a
 * small triangle hides or shows, like the page panel of Drawboard PDF.
 */
class UBPagesSidebar : public QWidget
{
    Q_OBJECT

public:
    UBPagesSidebar(QWidget* pages, QWidget* parent = nullptr);

private:
    void setPagesVisible(bool visible);

    QWidget* mPages{nullptr};
    QToolButton* mToggleButton{nullptr};
};
