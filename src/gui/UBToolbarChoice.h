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

#include <QCoreApplication>
#include <QList>

class QAction;
class QMenu;
class QWidget;
class UBActionPalette;

/**
 * @brief The buttons each tool palette shows, as the user chose them in the
 * Customize menu: the board palette and the desktop palette each have their
 * own choice, kept in the settings. The Tools menu offers every tool, also
 * those taken out of a palette.
 */
class UBToolbarChoice
{
    Q_DECLARE_TR_FUNCTIONS(UBToolbarChoice)

public:
    enum class Palette
    {
        Board,
        Desktop
    };

    /**
     * @brief The palette of the board or of the desktop, and the actions of
     * its buttons that the user may take out (all but the button that goes
     * back to the board from the desktop).
     */
    static void registerPalette(Palette palette, UBActionPalette* actionPalette, const QList<QAction*>& choosable);

    /** @brief The actions of the buttons of a palette the user may take out. */
    static QList<QAction*> choosableActions(Palette palette);

    /** @brief Shows on the palette the buttons the user chose. */
    static void apply(Palette palette);

    /** @brief Fills the Tools menu: every tool of the board and of the desktop. */
    static void fillToolsMenu(QMenu* menu);

    /**
     * @brief Fills the Customize menu: Desktop and Board / PDF, each with a
     * box per button. The menus stay open while boxes are ticked.
     */
    static void fillCustomizeMenu(QMenu* menu);

private:
    static bool isShown(Palette palette, QAction* action);
    static void setShown(Palette palette, QAction* action, bool shown);
};
