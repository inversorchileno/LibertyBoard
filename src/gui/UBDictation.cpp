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


#include "UBDictation.h"

#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QPointer>
#include <QTextCursor>
#include <QTimer>

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsTextItem.h"
#include "frameworks/UBPlatformUtils.h"
#include "gui/UBMainWindow.h"

// A new text box in the middle of the board shown, as the text tool makes one.
static QGraphicsTextItem* newTextBox()
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBGraphicsScene> scene = board ? board->activeScene() : nullptr;

    if (!scene || !board->controlView())
    {
        return nullptr;
    }

    QGraphicsView* view = board->controlView();
    const qreal width = scene->nominalSize().width() / board->currentZoom() / 4.;
    const QPointF center = view->mapToScene(view->viewport()->rect().center());
    UBGraphicsTextItem* textBox = scene->addTextHtml("", center - QPointF(width / 2, 0));

    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    textBox->setTextWidth(width);
    textBox->setSelected(true);
    return textBox;
}

void UBDictation::start(QGraphicsTextItem* textBox)
{
    if (!textBox)
    {
        textBox = UBApplication::boardController ? UBApplication::boardController->currentTextBox() : nullptr;
    }

    if (!textBox)
    {
        textBox = newTextBox();
    }

    if (!textBox)
    {
        return;
    }

    // The cursor at the end of the text, in the view, so that Windows writes there.
    textBox->setTextInteractionFlags(Qt::TextEditorInteraction);
    QTextCursor cursor = textBox->textCursor();
    cursor.movePosition(QTextCursor::End);
    textBox->setTextCursor(cursor);

    if (textBox->scene() && !textBox->scene()->views().isEmpty())
    {
        QGraphicsView* view = textBox->scene()->views().first();
        view->activateWindow();
        view->setFocus();
    }

    textBox->setFocus();

    // After the click, once the text box has the cursor.
    QPointer<QGraphicsTextItem> box(textBox);
    QTimer::singleShot(200, UBApplication::mainWindow, [box]() {
        if (box)
        {
            box->setFocus();
        }

        if (!UBPlatformUtils::startDictation())
        {
            UBApplication::mainWindow->information(tr("Dictate"), tr("The dictation of the system is not available here. "
                                                                    "On Windows, press Windows + H in a text."));
        }
    });
}
