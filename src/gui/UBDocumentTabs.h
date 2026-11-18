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

#include <memory>

#include <QList>
#include <QMap>
#include <QString>
#include <QWidget>

class QTabBar;
class UBBoardSaver;
class UBDocumentProxy;

/**
 * @brief The tabs of the documents open on the board, like a PDF reader.
 *
 * Each tab remembers its page and the file it was opened from or saved to,
 * so that Save writes to that file without asking. Clicking a tab shows its
 * document, the cross closes it and the plus button opens a new board.
 */
class UBDocumentTabs : public QWidget
{
    Q_OBJECT

public:
    explicit UBDocumentTabs(QWidget* parent = nullptr);

    /** @brief The number of open documents. */
    int count() const;

    /** @brief Shows the file in its tab, opening it in a new tab if needed. */
    void openFile(const QString& filePath);

    /** @brief Saves to the file of the tab, or asks for one the first time. */
    bool saveCurrent();

    /** @brief Asks for a file with the dialog of the system and saves to it. */
    bool saveCurrentAs();

public slots:
    /** @brief Opens a new blank board in a new tab. */
    void newDocument();

    /** @brief Closes the current tab, offering to save it first. */
    void closeCurrent();

signals:
    /** @brief The document was written to its file. */
    void saved();

    /** @brief A document was opened or closed. */
    void countChanged(int count);

private:
    struct OpenDocument
    {
        std::shared_ptr<UBDocumentProxy> document;
        int page{0};
        QString pdfFile;
        QMap<int, QString> pageFiles;
        bool modified{false};
    };

    int indexOf(const std::shared_ptr<UBDocumentProxy>& document) const;
    int currentIndex() const;
    QString fileOfCurrentPage() const;
    void rememberSavedFile(const QString& filename);
    void showDocument(int index);
    void closeTab(int index);
    bool confirmClose(int index);
    bool isBlank(const OpenDocument& open) const;
    void updateTabText(int index);

    void onActiveSceneChanged();
    void onFileImported(std::shared_ptr<UBDocumentProxy> document, const QString& filePath);
    void onTabClicked(int index);
    void onUndoStackChanged();

    QTabBar* mTabBar{nullptr};
    UBBoardSaver* mSaver{nullptr};
    QList<OpenDocument> mDocuments;
    bool mSwitching{false};
};
