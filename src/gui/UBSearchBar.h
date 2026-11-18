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

#include <QFrame>
#include <QList>
#include <QPointer>
#include <QRectF>
#include <QString>

class QGraphicsPathItem;
class QLabel;
class QLineEdit;
class UBDocumentProxy;
class UBGraphicsPDFItem;
class UBGraphicsScene;

/**
 * @brief The search box of the board: finds a text in the PDF shown and
 * goes from one place to the next, marking each place on its page.
 *
 * It floats at the top right of the main window, under the menu.
 */
class UBSearchBar : public QFrame
{
    Q_OBJECT

public:
    explicit UBSearchBar(QWidget* parent);

public slots:
    /** @brief Shows the box, or a message when the board shows no PDF. */
    void open();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    struct Place
    {
        int pdfPage{0};
        QRectF rect;
    };

    void search();
    void showPlace(int index);
    void step(int direction);
    void clearMark();
    void updateCount();
    void placeInWindow();
    int sceneIndexOf(int pdfPage) const;
    static UBGraphicsPDFItem* pdfItemOf(UBGraphicsScene* scene);

    QLineEdit* mText{nullptr};
    QLabel* mCount{nullptr};
    QList<Place> mPlaces;
    int mCurrent{-1};
    QString mSearchedText;
    std::shared_ptr<UBDocumentProxy> mDocument;
    QString mPdfFile;
    QGraphicsPathItem* mMark{nullptr};
    std::weak_ptr<UBGraphicsScene> mMarkScene;
};
