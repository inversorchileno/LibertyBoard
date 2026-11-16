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
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>

#include "board/UBBoardSaver.h"
#include "core/UBApplication.h"
#include "core/UBGitHubUpdater.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "gui/UBMainWindow.h"
#include "gui/UBPageIndicator.h"
#include "gui/UBToolOptionsBar.h"

UBMainMenu::UBMainMenu(UBMainWindow* mainWindow)
    : QObject(mainWindow)
    , mMainWindow(mainWindow)
{
    createMenus();
    setupDocumentShortcuts();
    arrangeBoardToolBar();
    createCornerButtons();

    // Dark like the menu bar and the tool palettes.
    const QString toolBarStyle = "QToolBar { background: #20242b; border: none; spacing: 2px; }"
                                 "QToolBar QToolButton { color: #e8e8e8; }"
                                 "QToolBar QToolButton:disabled { color: #6b7383; }";
    mMainWindow->boardToolBar->setStyleSheet(toolBarStyle);
    mMainWindow->documentToolBar->setStyleSheet(toolBarStyle);
    mMainWindow->webToolBar->setStyleSheet(toolBarStyle);
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

    mSaver = new UBBoardSaver(this);
    mUpdater = new UBGitHubUpdater(mMainWindow);

    // Home: save, settings and quit.
    QMenu* homeMenu = menuBar->addMenu(tr("&Home"));

    auto saveAction = new QAction(QIcon(":/images/toolbar/export.png"), tr("Save..."), mMainWindow);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setToolTip(tr("Save as a PDF with the annotations, or the page as a PNG or JPEG picture"));
    mMainWindow->addAction(saveAction);
    connect(saveAction, &QAction::triggered, this, [this]() { mSaver->save(mMainWindow); });
    homeMenu->addAction(saveAction);
    homeMenu->addSeparator();
    homeMenu->addAction(mMainWindow->actionPreferences);
    homeMenu->addAction(mMainWindow->actionMultiScreen);
    homeMenu->addSeparator();
    homeMenu->addAction(mMainWindow->actionPodcast);
    mMainWindow->actionPodcast->setText(tr("Podcast"));
    homeMenu->addSeparator();
    homeMenu->addAction(mMainWindow->actionQuit);

    QMenu* modeMenu = menuBar->addMenu(tr("&Mode"));
    modeMenu->addAction(mMainWindow->actionBoard);
    modeMenu->addAction(mMainWindow->actionWeb);
    modeMenu->addAction(mMainWindow->actionDocument);
    modeMenu->addAction(mMainWindow->actionDesktop);

    QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
    QActionGroup* pageSizeGroup = new QActionGroup(editMenu);
    pageSizeGroup->addAction(mMainWindow->actionWidePageSize);
    pageSizeGroup->addAction(mMainWindow->actionRegularPageSize);
    pageSizeGroup->addAction(mMainWindow->actionCustomPageSize);

    QMenu* pageSizeMenu = editMenu->addMenu(QIcon(":/images/toolbar/pageSize.png"), tr("Page Size"));
    pageSizeMenu->addAction(mMainWindow->actionWidePageSize);
    pageSizeMenu->addAction(mMainWindow->actionRegularPageSize);
    pageSizeMenu->addAction(mMainWindow->actionCustomPageSize);
    editMenu->addAction(mMainWindow->actionCut);
    editMenu->addAction(mMainWindow->actionCopy);
    editMenu->addAction(mMainWindow->actionPaste);
    editMenu->addAction(mMainWindow->actionHideApplication);
    editMenu->addAction(mMainWindow->actionSleep);

    QMenu* aboutMenu = menuBar->addMenu(tr("&About"));
    aboutMenu->addAction(mMainWindow->actionOpenTutorial);
    aboutMenu->addAction(mMainWindow->actionHintsAndTips);
    aboutMenu->addSeparator();

    auto gitHubAction = new QAction(tr("LibertyBoard on GitHub"), mMainWindow);
    connect(gitHubAction, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/noteimportaaaa/LibertyBoard"));
    });
    aboutMenu->addAction(gitHubAction);

    // Replaces the update check of OpenBoard by the one of LibertyBoard.
    mMainWindow->actionCheckUpdate->disconnect();
    mMainWindow->actionCheckUpdate->setText(tr("Update"));
    mMainWindow->actionCheckUpdate->setToolTip(tr("Download and install the latest version from GitHub"));
    mMainWindow->actionCheckUpdate->setVisible(true);
    connect(mMainWindow->actionCheckUpdate, &QAction::triggered, mUpdater, &UBGitHubUpdater::checkForUpdate);
    aboutMenu->addAction(mMainWindow->actionCheckUpdate);

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

void UBMainMenu::arrangeBoardToolBar()
{
    // Undo and redo, then the options of the selected tool, then the pages,
    // backgrounds and erase on the right.
    QToolBar* toolBar = mMainWindow->boardToolBar;
    toolBar->insertWidget(mMainWindow->actionPages, new UBToolOptionsBar(toolBar));
    toolBar->insertWidget(mMainWindow->actionForward, new UBPageIndicator(toolBar));
}

void UBMainMenu::createCornerButtons()
{
    auto corner = new QWidget(mMainWindow->menuBar());
    auto layout = new QHBoxLayout(corner);
    layout->setContentsMargins(0, 0, 4, 0);
    layout->setSpacing(4);

    const QString buttonStyle = "QToolButton { color: #f0f0f0; background: transparent; border: none; padding: 2px 8px; }"
                                "QToolButton:hover { background: #4a4a4a; }";

    // Goes to the desktop mode, like the board button of the desktop palette goes back.
    auto desktopButton = new QToolButton(corner);
    desktopButton->setDefaultAction(mMainWindow->actionDesktop);
    desktopButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    desktopButton->setIconSize(QSize(18, 18));
    desktopButton->setStyleSheet(buttonStyle);
    layout->addWidget(desktopButton);

    mToggleToolBarButton = new QToolButton(corner);
    mToggleToolBarButton->setStyleSheet(buttonStyle);
    mToggleToolBarButton->setToolTip(tr("Hide or show the toolbar"));
    layout->addWidget(mToggleToolBarButton);

    connect(mToggleToolBarButton, &QToolButton::clicked, this, [this]() {
        setToolBarsHidden(!mToolBarsHidden);
    });

    setToolBarsHidden(false);
    mMainWindow->menuBar()->setCornerWidget(corner, Qt::TopRightCorner);
}

QToolBar* UBMainMenu::currentToolBar() const
{
    switch (mMode)
    {
        case UBApplicationController::Document:
            return mMainWindow->documentToolBar;
        case UBApplicationController::Internet:
            return mMainWindow->webToolBar;
        default:
            return mMainWindow->boardToolBar;
    }
}

void UBMainMenu::setToolBarsHidden(bool hidden)
{
    mToolBarsHidden = hidden;
    mToggleToolBarButton->setText(hidden ? QStringLiteral("\u25BC") : QStringLiteral("\u25B2"));

    if (mMode == UBApplicationController::Board
            || mMode == UBApplicationController::Document
            || mMode == UBApplicationController::Internet)
    {
        currentToolBar()->setVisible(!hidden);
    }
}

void UBMainMenu::onMainModeChanged(UBApplicationController::MainMode mode)
{
    mMode = mode;

    // The application controller shows the toolbar of each mode: keep it
    // hidden if the user hid it.
    if (mToolBarsHidden)
    {
        QMetaObject::invokeMethod(this, [this]() { setToolBarsHidden(true); }, Qt::QueuedConnection);
    }
}
