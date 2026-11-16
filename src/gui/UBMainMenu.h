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

#include <QObject>

#include "core/UBApplicationController.h"

class QMenu;
class UBMainWindow;

/**
 * @brief The menu bar of the main window, like most desktop programs: the
 * modes, the document actions and the application settings.
 *
 * The toolbars keep only the actions used most often; every other action
 * of a mode is in the menus, with its keyboard shortcut.
 */
class UBMainMenu : public QObject
{
    Q_OBJECT

public:
    explicit UBMainMenu(UBMainWindow* mainWindow);

    /** @brief The application menu: page size, clipboard, help, preferences and quit. */
    static QMenu* createApplicationMenu(UBMainWindow* mainWindow, QWidget* parent);

    void onMainModeChanged(UBApplicationController::MainMode mode);

private:
    void createMenus();
    void setupDocumentShortcuts();
    void spreadBoardToolBar();

    UBMainWindow* mMainWindow{nullptr};
    QMenu* mDocumentMenu{nullptr};
};
