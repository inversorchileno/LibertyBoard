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


#include "UBTextBoxes.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTextCursor>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "desktop/UBDesktopAnnotationController.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBTextTables.h"

// The keyboard writes in the text box: its view has the focus, and the box the cursor.
static void giveCursor(UBGraphicsTextItem* textBox)
{
    textBox->setTextInteractionFlags(Qt::TextEditorInteraction);
    textBox->setSelected(true);

    if (UBBoardView* view = UBTextBoxes::activeView())
    {
        view->activateWindow();
        view->setFocus();
    }

    textBox->setFocus();
}

UBBoardView* UBTextBoxes::activeView()
{
    if (UBDrawingController::drawingController()->isInDesktopMode() && UBApplication::applicationController
        && UBApplication::applicationController->uninotesController())
    {
        return UBApplication::applicationController->uninotesController()->drawingView();
    }

    return UBApplication::boardController ? UBApplication::boardController->controlView() : nullptr;
}

UBGraphicsTextItem* UBTextBoxes::current()
{
    UBBoardView* view = activeView();
    std::shared_ptr<UBGraphicsScene> scene = view ? view->scene() : nullptr;

    if (!scene)
    {
        return nullptr;
    }

    if (auto focused = dynamic_cast<UBGraphicsTextItem*>(scene->focusItem()))
    {
        return focused;
    }

    for (QGraphicsItem* item : scene->selectedItems())
    {
        if (auto text = dynamic_cast<UBGraphicsTextItem*>(item))
        {
            return text;
        }
    }

    return nullptr;
}

UBGraphicsTextItem* UBTextBoxes::create()
{
    UBBoardView* view = activeView();
    std::shared_ptr<UBGraphicsScene> scene = view ? view->scene() : nullptr;

    if (!scene)
    {
        return nullptr;
    }

    // As wide as the text tool makes them: a quarter of the page on the screen.
    const qreal zoom = view->transform().m11() > 0 ? view->transform().m11() : 1.0;
    const qreal width = view->viewport()->width() / zoom / 4.;
    const QPointF center = view->mapToScene(view->viewport()->rect().center());
    UBGraphicsTextItem* textBox = scene->addTextHtml("", center - QPointF(width / 2, 0));

    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    textBox->setTextWidth(width);
    giveCursor(textBox);
    return textBox;
}

void UBTextBoxes::insertTable(QWidget* parent)
{
    // The size of the table, as in note-taking applications.
    QDialog dialog(parent);
    dialog.setWindowTitle(tr("Table"));
    auto layout = new QFormLayout(&dialog);
    auto title = new QLabel(tr("How many rows and columns?"), &dialog);
    layout->addRow(title);

    auto rows = new QSpinBox(&dialog);
    rows->setRange(1, 30);
    rows->setValue(2);
    layout->addRow(tr("Rows"), rows);

    auto columns = new QSpinBox(&dialog);
    columns->setRange(1, 12);
    columns->setValue(3);
    layout->addRow(tr("Columns"), columns);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Insert"));
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    UBGraphicsTextItem* textBox = current();

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    // In the text box being written, at its cursor; or in a new one, twice as wide.
    if (!textBox)
    {
        textBox = create();

        if (!textBox)
        {
            return;
        }

        textBox->setTextWidth(textBox->textWidth() * 2);
    }

    QTextCursor cursor = textBox->textCursor();
    UBTextTables::insert(cursor, rows->value(), columns->value());
    textBox->setTextCursor(cursor);
    giveCursor(textBox);
}
