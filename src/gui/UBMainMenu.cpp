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
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>

#include "core/UBApplication.h"
#include "core/UBDocumentManager.h"
#include "core/UBGitHubUpdater.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "gui/UBDocumentTabs.h"
#include "gui/UBMainWindow.h"
#include "gui/UBPageIndicator.h"
#include "gui/UBSearchBar.h"
#include "gui/UBToolOptionsBar.h"

UBMainMenu::UBMainMenu(UBMainWindow* mainWindow)
    : QObject(mainWindow)
    , mMainWindow(mainWindow)
{
    createDocumentTabs();
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

    mUpdater = new UBGitHubUpdater(mMainWindow);

    // Home: new, open, save, settings and quit.
    QMenu* homeMenu = menuBar->addMenu(tr("&Home"));

    auto newAction = new QAction(mMainWindow->actionNewDocument->icon(), mMainWindow->actionNewDocument->text(), mMainWindow);
    newAction->setShortcut(QKeySequence::New);
    mMainWindow->addAction(newAction);
    connect(newAction, &QAction::triggered, mDocumentTabs, &UBDocumentTabs::newDocument);

    auto openAction = new QAction(QIcon(":/images/toolbar/documents.png"), tr("Open/Import"), mMainWindow);
    openAction->setShortcut(QKeySequence::Open);
    mMainWindow->addAction(openAction);
    connect(openAction, &QAction::triggered, this, &UBMainMenu::openFiles);

    auto saveAction = new QAction(QIcon(":/images/toolbar/export.png"), tr("Save"), mMainWindow);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setToolTip(tr("Save the annotations in the PDF, or the page as a picture"));
    mMainWindow->addAction(saveAction);
    connect(saveAction, &QAction::triggered, mDocumentTabs, &UBDocumentTabs::saveCurrent);

    auto saveAsAction = new QAction(tr("Save as..."), mMainWindow);
    saveAsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    saveAsAction->setToolTip(tr("Save a copy as a PDF, PNG or JPEG file"));
    mMainWindow->addAction(saveAsAction);
    connect(saveAsAction, &QAction::triggered, mDocumentTabs, &UBDocumentTabs::saveCurrentAs);

    homeMenu->addAction(newAction);
    homeMenu->addAction(openAction);
    homeMenu->addAction(saveAction);
    homeMenu->addAction(saveAsAction);
    homeMenu->addSeparator();
    homeMenu->addAction(mMainWindow->actionPreferences);
    homeMenu->addAction(mMainWindow->actionMultiScreen);
    homeMenu->addSeparator();
    homeMenu->addAction(mMainWindow->actionPodcast);
    mMainWindow->actionPodcast->setText(tr("Podcast"));
    homeMenu->addSeparator();

    auto closeAction = new QAction(QIcon(":/images/toolbar/closeDocument.svg"), tr("Close document"), mMainWindow);
    closeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F4));
    mMainWindow->addAction(closeAction);
    connect(closeAction, &QAction::triggered, mDocumentTabs, &UBDocumentTabs::closeCurrent);
    homeMenu->addAction(closeAction);
    homeMenu->addAction(mMainWindow->actionQuit);

    QMenu* modeMenu = menuBar->addMenu(tr("&Mode"));
    modeMenu->addAction(mMainWindow->actionBoard);
    modeMenu->addAction(mMainWindow->actionWeb);
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

    // "Saved", for a few seconds after each save.
    // Its place is kept, so that the buttons beside it do not move.
    mSavedNotice = new QLabel(corner);
    mSavedNotice->setStyleSheet("QLabel { color: #8fe3a0; font-weight: bold; padding: 0 10px; }");
    mSavedNotice->setAlignment(Qt::AlignCenter);
    QFont bold = mSavedNotice->font();
    bold.setBold(true);
    mSavedNotice->setMinimumWidth(QFontMetrics(bold).horizontalAdvance(tr("Saved")) + 24);
    layout->addWidget(mSavedNotice);
    connect(mDocumentTabs, &UBDocumentTabs::saved, this, &UBMainMenu::showSavedNotice);

    // Finds a text in the PDF shown on the board.
    auto searchBar = new UBSearchBar(mMainWindow);
    auto searchAction = new QAction(QIcon(":/images/toolbar/search.svg"), tr("Search..."), mMainWindow);
    searchAction->setShortcut(QKeySequence::Find);
    // Ctrl+F is for the search, as in other programs, not for the selector.
    mMainWindow->actionSelector->setShortcut(QKeySequence());
    searchAction->setToolTip(tr("Find words in the PDF"));
    mMainWindow->addAction(searchAction);
    connect(searchAction, &QAction::triggered, searchBar, &UBSearchBar::open);

    auto searchButton = new QToolButton(corner);
    searchButton->setDefaultAction(searchAction);
    searchButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    searchButton->setIconSize(QSize(18, 18));
    searchButton->setStyleSheet(buttonStyle);
    layout->addWidget(searchButton);

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

void UBMainMenu::createDocumentTabs()
{
    // A row of its own under the toolbar of the board.
    mDocumentTabs = new UBDocumentTabs(mMainWindow);
    mDocumentTabsToolBar = new QToolBar(mMainWindow);
    mDocumentTabsToolBar->setObjectName("documentTabsToolBar");
    mDocumentTabsToolBar->setMovable(false);
    mDocumentTabsToolBar->setFloatable(false);
    mDocumentTabsToolBar->toggleViewAction()->setVisible(false);
    mDocumentTabsToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    mDocumentTabsToolBar->setStyleSheet("QToolBar { background: #20242b; border: none; padding: 0; }");
    mDocumentTabsToolBar->addWidget(mDocumentTabs);

    mMainWindow->insertToolBar(mMainWindow->boardToolBar, mDocumentTabsToolBar);
    mMainWindow->insertToolBarBreak(mMainWindow->boardToolBar);

    connect(mDocumentTabs, &UBDocumentTabs::countChanged, this, &UBMainMenu::updateDocumentTabsVisibility);
    updateDocumentTabsVisibility();
}

void UBMainMenu::updateDocumentTabsVisibility()
{
    // Only useful to switch between documents: hidden with a single one.
    mDocumentTabsToolBar->setVisible(mMode == UBApplicationController::Board && mDocumentTabs->count() > 1);
}

void UBMainMenu::openFiles()
{
    static const QString lastFolderKey = QStringLiteral("App/LibertyBoardLastOpenFolder");
    const QString folder = UBSettings::settings()->value(lastFolderKey,
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();

    const QStringList files = QFileDialog::getOpenFileNames(mMainWindow, tr("Open"), folder,
        UBDocumentManager::documentManager()->importFileFilter(true));

    if (files.isEmpty())
    {
        return;
    }

    UBSettings::settings()->setValue(lastFolderKey, QFileInfo(files.first()).absolutePath());

    for (const QString& file : files)
    {
        mDocumentTabs->openFile(file);
    }
}

void UBMainMenu::showSavedNotice()
{
    mSavedNotice->setText(tr("Saved"));
    QTimer::singleShot(3000, mSavedNotice, &QLabel::clear);
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
    updateDocumentTabsVisibility();

    // The application controller shows the toolbar of each mode: keep it
    // hidden if the user hid it.
    if (mToolBarsHidden)
    {
        QMetaObject::invokeMethod(this, [this]() { setToolBarsHidden(true); }, Qt::QueuedConnection);
    }
}
