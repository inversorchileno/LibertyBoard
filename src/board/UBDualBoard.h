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

#include <QGraphicsView>
#include <QObject>
#include <QPointer>
#include <QTransform>

class QSplitter;
class UBDocumentProxy;
class UBDocumentTabs;
class UBGraphicsScene;

/**
 * @brief The side of the double board that is not active: it shows its
 * document as it was left, can be scrolled, and asks to become the active
 * side when it is clicked.
 */
class UBOtherBoardView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit UBOtherBoardView(QWidget* parent = nullptr);

signals:
    void activationRequested();
    void resized();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void tabletEvent(QTabletEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

/**
 * @brief The double board: two documents side by side, two PDFs, two boards
 * or a PDF and a board for notes.
 *
 * Only one side is the board at a time, the active one, on which the tools,
 * pages and saving work, with a dark frame. The other side shows its own
 * document; a click on it makes it the active side, without moving it.
 */
class UBDualBoard : public QObject
{
    Q_OBJECT

public:
    explicit UBDualBoard(UBDocumentTabs* tabs, QObject* parent = nullptr);

    bool isEnabled() const;

signals:
    /** @brief The double board was turned on or off, also by itself when one of its documents closes. */
    void enabledChanged(bool enabled);

public slots:
    /** @brief Shows two sides, the current document and a new blank board, or one side again. */
    void setEnabled(bool enabled);

    /** @brief Makes the other side the active one. */
    void activateOtherSide();

private:
    void showOtherSide();
    void fitOtherSide();

    /** @brief Keeps each side between a fifth and four fifths of the width. */
    void limitSides();
    void placeSides();

    /**
     * @brief Asks which documents to close so that only two stay open,
     * keeping the one given, and closes them. Returns false when the user
     * does not close enough of them.
     */
    bool keepTwoDocuments(const std::shared_ptr<UBDocumentProxy>& kept);

    void onDocumentClosing(std::shared_ptr<UBDocumentProxy> document);
    void onDocumentCountChanged(int count);

    UBDocumentTabs* mTabs{nullptr};
    QPointer<UBOtherBoardView> mOtherView;
    QPointer<QSplitter> mSplitter;
    std::shared_ptr<UBDocumentProxy> mOtherDocument;
    std::shared_ptr<UBGraphicsScene> mOtherScene;
    int mOtherPage{0};
    QTransform mOtherTransform;
    QPointF mOtherCenter;
    bool mActiveOnLeft{true};
    bool mFitPending{false};
};
