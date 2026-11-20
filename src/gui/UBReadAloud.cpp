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

#include "UBReadAloud.h"

#include <QTextToSpeech>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsTextItem.h"
#include "gui/UBMainWindow.h"
#include "pdf/UBPdfTextSearch.h"

UBReadAloud::UBReadAloud(QObject* parent)
    : QObject(parent)
{
}

bool UBReadAloud::isReading() const
{
    return mSpeech && mSpeech->state() == QTextToSpeech::Speaking;
}

void UBReadAloud::readOrStop()
{
    if (isReading())
    {
        mSpeech->stop();
        return;
    }

    const QString text = pageText();

    if (text.isEmpty())
    {
        UBApplication::showMessage(tr("There is no text to read on this page: write or select a text, or open a PDF with text."));
        return;
    }

    // The voice of the system, created when first needed.
    if (!mSpeech)
    {
        mSpeech = new QTextToSpeech(this);
        connect(mSpeech, &QTextToSpeech::stateChanged, this, [this](QTextToSpeech::State state) {
            // A voice that was still starting reads once it is ready.
            if (state == QTextToSpeech::Ready && !mPendingText.isEmpty())
            {
                mSpeech->say(mPendingText);
                mPendingText.clear();
                return;
            }

            emit readingChanged(state == QTextToSpeech::Speaking);
        });
    }

    if (mSpeech->state() == QTextToSpeech::Error)
    {
        UBApplication::mainWindow->warning(tr("Read aloud"), tr("The voices of the system could not be used to read aloud."));
        return;
    }

    if (mSpeech->state() != QTextToSpeech::Ready)
    {
        mPendingText = text;
        return;
    }

    mSpeech->say(text);
}

QString UBReadAloud::pageText()
{
    std::shared_ptr<UBGraphicsScene> scene = UBApplication::boardController ? UBApplication::boardController->activeScene() : nullptr;

    if (!scene)
    {
        return QString();
    }

    // The texts selected, when there are some.
    QStringList selected;
    QList<UBGraphicsTextItem*> texts;

    for (QGraphicsItem* item : scene->items(Qt::AscendingOrder))
    {
        if (auto text = dynamic_cast<UBGraphicsTextItem*>(item))
        {
            texts << text;

            if (text->isSelected())
            {
                selected << text->toPlainText();
            }
        }
    }

    if (!selected.isEmpty())
    {
        return selected.join("\n").trimmed();
    }

    // Else the text of the PDF page, then the texts written, from the top.
    QStringList parts;
    auto pdf = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());
    std::shared_ptr<UBDocumentProxy> document = scene->document();

    if (pdf && document)
    {
        const QString pdfFile = document->persistencePath() + "/" + UBPersistenceManager::objectDirectory + "/"
                                + pdf->fileUuid().toString() + ".pdf";
        const QSizeF size = pdf->pageSize();
        parts << UBPdfTextSearch(pdfFile).textIn(pdf->pageNumber(), QRectF(QPointF(0, 0), size));
    }

    std::sort(texts.begin(), texts.end(), [](UBGraphicsTextItem* a, UBGraphicsTextItem* b) {
        return a->scenePos().y() < b->scenePos().y();
    });

    for (UBGraphicsTextItem* text : std::as_const(texts))
    {
        parts << text->toPlainText();
    }

    parts.removeAll(QString());
    return parts.join("\n").trimmed();
}
