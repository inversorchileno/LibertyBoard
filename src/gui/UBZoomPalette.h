/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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

#include "UBFloatingPalette.h"

class QLineEdit;
class QToolButton;
class UBBoardController;

/**
 * @brief A small fixed box at the bottom right of the board with zoom out,
 * the zoom in percent, which the user can type, and zoom in.
 */
class UBZoomPalette : public UBFloatingPalette
{
    Q_OBJECT

public:
    explicit UBZoomPalette(QWidget* parent);

    /** @brief Puts the box at the bottom right of the area, left of what it must not cover. */
    void placeIn(const QRect& area, const QRect& avoid);

public slots:
    void refreshPalette();

protected:
    // The box stays where it is placed.
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void zoomTo(qreal zoom);
    void stepZoom(bool in);
    void onPercentEdited();

    UBBoardController* mBoardController{nullptr};
    QToolButton* mZoomOutButton{nullptr};
    QLineEdit* mPercent{nullptr};
    QToolButton* mZoomInButton{nullptr};
};
