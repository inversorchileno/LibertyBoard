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


#include "UBPageRotation.h"

#include <QGraphicsItem>
#include <QTransform>
#include <QUndoStack>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBItem.h"
#include "domain/UBUndoCommand.h"

namespace
{
    /** Turning back is turning the other way. */
    class RotationUndoCommand : public UBUndoCommand
    {
    public:
        RotationUndoCommand(std::shared_ptr<UBGraphicsScene> scene, int degrees)
            : mScene(scene)
            , mDegrees(degrees)
        {
        }

        void undo() override
        {
            UBPageRotation::rotate(mScene, -mDegrees);
        }

        void redo() override
        {
            UBPageRotation::rotate(mScene, mDegrees);
        }

    private:
        std::shared_ptr<UBGraphicsScene> mScene;
        int mDegrees;
    };
}

void UBPageRotation::rotateActivePage(int degrees)
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBGraphicsScene> scene = board ? board->activeScene() : nullptr;

    if (!scene)
    {
        return;
    }

    // The stack runs the command when it is pushed.
    if (UBApplication::undoStack)
    {
        UBApplication::undoStack->push(new RotationUndoCommand(scene, degrees));
    }
    else
    {
        rotate(scene, degrees);
    }
}

void UBPageRotation::rotate(const std::shared_ptr<UBGraphicsScene>& scene, int degrees)
{
    if (!scene)
    {
        return;
    }

    // The page is centered on the origin of the scene: turning around the
    // origin keeps it in place. Each item keeps its whole placement in its
    // transform, as when it is read from the document.
    QTransform turn;
    turn.rotate(degrees);

    const QList<QGraphicsItem*> items = scene->items();

    for (QGraphicsItem* item : items)
    {
        if (item->parentItem() || !dynamic_cast<UBItem*>(item))
        {
            continue;
        }

        const QTransform placement = item->sceneTransform() * turn;
        item->setRotation(0);
        item->setScale(1);
        item->setPos(0, 0);
        item->setTransform(placement);
    }

    if (degrees % 180 != 0)
    {
        // Only this page turns: the size of new pages stays the same.
        std::shared_ptr<UBDocumentProxy> document = scene->document();
        const QSize defaultSize = document ? document->defaultDocumentSize() : QSize();

        scene->setNominalSize(scene->nominalSize().transposed());

        if (document)
        {
            document->setDefaultDocumentSize(defaultSize);
        }
    }

    scene->setModified(true);

    UBBoardController* board = UBApplication::boardController;

    if (board && board->activeScene() == scene)
    {
        board->zoomRestore();
    }
}
