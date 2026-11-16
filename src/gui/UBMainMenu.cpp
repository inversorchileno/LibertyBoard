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

#include "UBMainMenu.h"

#include <QActionGroup>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "gui/UBMainWindow.h"

UBMainMenu::UBMainMenu(UBMainWindow* mainWindow)
    : QObject(mainWindow)
    , mMainWindow(mainWindow)
{
    createMenus();
    setupDocumentShortcuts();
    spreadBoardToolBar();
}

QMenu* UBMainMenu::createApplicationMenu(UBMainWindow* mainWindow, QWidget* parent)
{
    QMenu* menu = new QMenu(parent);

    QActionGroup* pageSizeGroup = new QActionGroup(menu);
    pageSizeGroup->addAction(mainWindow->actionWidePageSize);
    pageSizeGroup->addAction(mainWindow->actionRegularPageSize);
    pageSizeGroup->addAction(mainWindow->actionCustomPageSize);

    QMenu* documentSizeMenu = menu->addMenu(QIcon(":/images/toolbar/pageSize.png"), tr("Page Size"));
    documentSizeMenu->addAction(mainWindow->actionWidePageSize);
    documentSizeMenu->addAction(mainWindow->actionRegularPageSize);
    documentSizeMenu->addAction(mainWindow->actionCustomPageSize);
    menu->addAction(mainWindow->actionCut);
    menu->addAction(mainWindow->actionCopy);
    menu->addAction(mainWindow->actionPaste);
    menu->addAction(mainWindow->actionHideApplication);
    menu->addAction(mainWindow->actionSleep);

    menu->addSeparator();
    menu->addAction(mainWindow->actionOpenTutorial);
    menu->addAction(mainWindow->actionHintsAndTips);
    menu->addSeparator();
    menu->addAction(mainWindow->actionPreferences);
    menu->addAction(mainWindow->actionMultiScreen);

    if (!UBSettings::settings()->appHideCheckForSoftwareUpdate->get().toBool())
    {
        menu->addAction(mainWindow->actionCheckUpdate);
    }

    menu->addSeparator();
    menu->addAction(mainWindow->actionPodcast);
    mainWindow->actionPodcast->setText(tr("Podcast"));

    menu->addSeparator();
    menu->addAction(mainWindow->actionQuit);

    return menu;
}

void UBMainMenu::createMenus()
{
    QMenuBar* menuBar = mMainWindow->menuBar();
    menuBar->setNativeMenuBar(false);

    QMenu* viewMenu = menuBar->addMenu(tr("&View"));
    viewMenu->addAction(mMainWindow->actionBoard);
    viewMenu->addAction(mMainWindow->actionWeb);
    viewMenu->addAction(mMainWindow->actionDocument);
    viewMenu->addAction(mMainWindow->actionDesktop);

    mDocumentMenu = menuBar->addMenu(tr("&Document"));
    mDocumentMenu->addAction(mMainWindow->actionNewDocument);
    mDocumentMenu->addAction(mMainWindow->actionNewFolder);
    mDocumentMenu->addSeparator();
    mDocumentMenu->addAction(mMainWindow->actionImport);
    mDocumentMenu->addAction(mMainWindow->actionExport);
    mDocumentMenu->addSeparator();
    mDocumentMenu->addAction(mMainWindow->actionRename);
    mDocumentMenu->addAction(mMainWindow->actionAddDocumentToFavorites);
    mDocumentMenu->addAction(mMainWindow->actionDuplicate);
    mDocumentMenu->addAction(mMainWindow->actionDelete);
    mDocumentMenu->addSeparator();
    mDocumentMenu->addAction(mMainWindow->actionOpen);
    mDocumentMenu->addAction(mMainWindow->actionAddToWorkingDocument);
    mDocumentMenu->addAction(mMainWindow->actionDocumentAdd);
    mDocumentMenu->menuAction()->setVisible(false);

    QMenu* applicationMenu = createApplicationMenu(mMainWindow, menuBar);
    applicationMenu->setTitle(QStringLiteral("&LibertyBoard"));
    menuBar->addMenu(applicationMenu);

    // The same dark gray as the tool palettes.
    menuBar->setStyleSheet(
        "QMenuBar { background: #323232; color: #f0f0f0; }"
        "QMenuBar::item { padding: 4px 10px; background: transparent; }"
        "QMenuBar::item:selected { background: #4a4a4a; }");
}

void UBMainMenu::setupDocumentShortcuts()
{
    const QList<QPair<QAction*, QKeySequence>> shortcuts = {
        { mMainWindow->actionNewDocument, QKeySequence(Qt::CTRL | Qt::Key_N) },
        { mMainWindow->actionNewFolder, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N) },
        { mMainWindow->actionImport, QKeySequence(Qt::CTRL | Qt::Key_I) },
        { mMainWindow->actionRename, QKeySequence(Qt::Key_F2) },
        { mMainWindow->actionDuplicate, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D) },
    };

    for (const auto& shortcut : shortcuts)
    {
        // Only in the Documents mode, where these actions apply.
        shortcut.first->setShortcut(shortcut.second);
        shortcut.first->setShortcutContext(Qt::WidgetWithChildrenShortcut);

        if (QWidget* documents = mMainWindow->documentsWidget())
        {
            documents->addAction(shortcut.first);
        }
    }
}

void UBMainMenu::spreadBoardToolBar()
{
    // Undo and redo on the left, the pages in the middle, backgrounds and
    // erase on the right.
    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->boardToolBar, mMainWindow->actionPages);
    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->boardToolBar, mMainWindow->actionBackgrounds);
}

void UBMainMenu::onMainModeChanged(UBApplicationController::MainMode mode)
{
    mDocumentMenu->menuAction()->setVisible(mode == UBApplicationController::Document);
}
