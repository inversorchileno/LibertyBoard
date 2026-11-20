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
#include <QTimer>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsTextItem.h"
#include "gui/UBMainWindow.h"
#include "pdf/UBPdfTextSearch.h"

UBReadAloud* UBReadAloud::instance()
{
    static UBReadAloud* voice = new UBReadAloud(qApp);
    return voice;
}

UBReadAloud::UBReadAloud(QObject* parent)
    : QObject(parent)
{
}

bool UBReadAloud::isReading() const
{
    return mSpeech && mSpeech->state() == QTextToSpeech::Speaking;
}

bool UBReadAloud::isPaused() const
{
    return mSpeech && mSpeech->state() == QTextToSpeech::Paused;
}

void UBReadAloud::readOrStop()
{
    if (isReading() || isPaused())
    {
        stop();
        return;
    }

    mContinuous = false;
    say(pageText(true));
}

void UBReadAloud::play()
{
    if (isPaused())
    {
        mSpeech->resume();
        return;
    }

    if (isReading())
    {
        return;
    }

    mContinuous = true;
    say(pageText(false));
}

void UBReadAloud::pause()
{
    if (isReading())
    {
        mSpeech->pause();
    }
}

void UBReadAloud::stop()
{
    mContinuous = false;
    mPendingText.clear();
    setReadingArea(QRectF());

    if (mSpeech && (isReading() || isPaused()))
    {
        mStopping = true;
        mSpeech->stop();
    }
}

void UBReadAloud::nextPage()
{
    UBBoardController* board = UBApplication::boardController;

    if (!board || board->activeSceneIndex() + 1 >= board->selectedDocument()->pageCount())
    {
        UBApplication::showMessage(tr("This is the last page."));
        return;
    }

    const bool wasReading = isReading() || isPaused();
    stop();
    board->nextScene();

    if (wasReading)
    {
        // Once the page is shown.
        QTimer::singleShot(300, this, &UBReadAloud::play);
    }
}

void UBReadAloud::readArea(const QRectF& sceneRect)
{
    stop();
    mContinuous = false;

    const QString text = areaText(sceneRect);

    if (text.isEmpty())
    {
        UBApplication::showMessage(tr("There is no text in this rectangle."));
        return;
    }

    // Marked while it is read, so that one sees what is read.
    setReadingArea(sceneRect);
    say(text);
}

void UBReadAloud::readFrom(const QPointF& scenePoint)
{
    stop();
    mContinuous = true;
    say(textFrom(scenePoint));
}

void UBReadAloud::setReadingArea(const QRectF& sceneRect)
{
    if (sceneRect == mReadingArea)
    {
        return;
    }

    mReadingArea = sceneRect;
    emit readingAreaChanged();
}

void UBReadAloud::say(const QString& text)
{
    if (text.isEmpty())
    {
        UBApplication::showMessage(tr("There is no text to read on this page: write or select a text, or open a PDF with text."));

        // Reading on: the next page may have text.
        if (mContinuous)
        {
            QTimer::singleShot(0, this, [this]() {
                UBBoardController* board = UBApplication::boardController;

                if (board && board->activeSceneIndex() + 1 < board->selectedDocument()->pageCount())
                {
                    board->nextScene();
                    QTimer::singleShot(300, this, [this]() { say(pageText(false)); });
                }
                else
                {
                    mContinuous = false;
                }
            });
        }

        return;
    }

    // The voice of the system, created when first needed.
    if (!mSpeech)
    {
        mSpeech = new QTextToSpeech(this);
        connect(mSpeech, &QTextToSpeech::stateChanged, this, &UBReadAloud::onStateChanged);
    }

    if (mSpeech->state() == QTextToSpeech::Error)
    {
        mContinuous = false;
        setReadingArea(QRectF());
        UBApplication::mainWindow->warning(tr("Read aloud"), tr("The voices of the system could not be used to read aloud."));
        return;
    }

    // A voice still starting reads once it is ready.
    if (mSpeech->state() != QTextToSpeech::Ready)
    {
        mPendingText = text;
        return;
    }

    mStopping = false;
    mSpeech->say(text);
}

void UBReadAloud::onStateChanged()
{
    const QTextToSpeech::State state = mSpeech->state();

    // The voice could not start: say so instead of reading nothing.
    if (state == QTextToSpeech::Error && (!mPendingText.isEmpty() || mContinuous))
    {
        mPendingText.clear();
        mContinuous = false;
        setReadingArea(QRectF());
        UBApplication::mainWindow->warning(tr("Read aloud"), tr("The voices of the system could not be used to read aloud."));
        emit readingChanged(false);
        return;
    }

    if (state == QTextToSpeech::Ready && !mPendingText.isEmpty())
    {
        const QString text = mPendingText;
        mPendingText.clear();
        mSpeech->say(text);
        return;
    }

    if (state == QTextToSpeech::Speaking)
    {
        mWasSpeaking = true;
    }

    // The page was read to its end: the next one, when reading on.
    if (state == QTextToSpeech::Ready && mWasSpeaking)
    {
        mWasSpeaking = false;
        setReadingArea(QRectF());

        if (mContinuous && !mStopping)
        {
            UBBoardController* board = UBApplication::boardController;

            if (board && board->activeSceneIndex() + 1 < board->selectedDocument()->pageCount())
            {
                board->nextScene();
                QTimer::singleShot(300, this, [this]() {
                    if (mContinuous)
                    {
                        say(pageText(false));
                    }
                });
            }
            else
            {
                mContinuous = false;
            }
        }

        mStopping = false;
    }

    emit readingChanged(state == QTextToSpeech::Speaking || state == QTextToSpeech::Paused);
}

QString UBReadAloud::pageText(bool selectionFirst)
{
    std::shared_ptr<UBGraphicsScene> scene = UBApplication::boardController ? UBApplication::boardController->activeScene() : nullptr;

    if (!scene)
    {
        return QString();
    }

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

    if (selectionFirst && !selected.isEmpty())
    {
        return selected.join("\n").trimmed();
    }

    // The text of the PDF page, then the texts written, from the top.
    QStringList parts;
    auto pdf = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());
    std::shared_ptr<UBDocumentProxy> document = scene->document();

    if (pdf && document)
    {
        const QString pdfFile = document->persistencePath() + "/" + UBPersistenceManager::objectDirectory + "/"
                                + pdf->fileUuid().toString() + ".pdf";
        parts << UBPdfTextSearch(pdfFile).textIn(pdf->pageNumber(), QRectF(QPointF(0, 0), pdf->pageSize()));
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

// The PDF page under the scene, as a file and a page of it, and the
// rectangle of the scene in points of that page.
struct PdfPlace
{
    QString file;
    int page{0};
    QSizeF pageSize;
    QRectF onPage;
};

static bool pdfPlace(const std::shared_ptr<UBGraphicsScene>& scene, const QRectF& sceneRect, PdfPlace& place)
{
    auto pdf = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());
    std::shared_ptr<UBDocumentProxy> document = scene->document();

    if (!pdf || !document)
    {
        return false;
    }

    const QRectF onItem = pdf->mapFromScene(sceneRect).boundingRect();
    const QRectF bounds = pdf->boundingRect();
    const QSizeF points = pdf->pageSize();

    if (bounds.isEmpty() || points.isEmpty())
    {
        return false;
    }

    const qreal sx = points.width() / bounds.width();
    const qreal sy = points.height() / bounds.height();
    place.onPage = QRectF((onItem.left() - bounds.left()) * sx, (onItem.top() - bounds.top()) * sy,
                          onItem.width() * sx, onItem.height() * sy);
    place.file = document->persistencePath() + "/" + UBPersistenceManager::objectDirectory + "/"
                 + pdf->fileUuid().toString() + ".pdf";
    place.page = pdf->pageNumber();
    place.pageSize = points;
    return true;
}

QString UBReadAloud::areaText(const QRectF& sceneRect)
{
    std::shared_ptr<UBGraphicsScene> scene = UBApplication::boardController ? UBApplication::boardController->activeScene() : nullptr;

    if (!scene || sceneRect.isEmpty())
    {
        return QString();
    }

    QStringList parts;
    PdfPlace place;

    if (pdfPlace(scene, sceneRect, place))
    {
        parts << UBPdfTextSearch(place.file).textIn(place.page, place.onPage);
    }

    for (QGraphicsItem* item : scene->items(sceneRect, Qt::IntersectsItemShape, Qt::AscendingOrder))
    {
        if (auto text = dynamic_cast<UBGraphicsTextItem*>(item))
        {
            parts << text->toPlainText();
        }
    }

    parts.removeAll(QString());
    return parts.join("\n").trimmed();
}

QString UBReadAloud::textFrom(const QPointF& scenePoint)
{
    std::shared_ptr<UBGraphicsScene> scene = UBApplication::boardController ? UBApplication::boardController->activeScene() : nullptr;

    if (!scene)
    {
        return QString();
    }

    QStringList parts;
    PdfPlace place;

    if (pdfPlace(scene, QRectF(scenePoint, QSizeF(0, 0)), place))
    {
        // The line clicked from the word clicked, then all below it.
        const UBPdfTextSearch search(place.file);
        const QPointF point = place.onPage.topLeft();
        const QList<QRectF> words = search.wordsIn(place.page, QRectF(point - QPointF(2, 2), QSizeF(4, 4)));
        const QRectF word = words.isEmpty() ? QRectF(point.x(), point.y() - 6, 1, 12) : words.first();
        const qreal width = place.pageSize.width();
        const qreal height = place.pageSize.height();

        parts << search.textIn(place.page, QRectF(word.left(), word.top(), width - word.left(), word.height()));
        parts << search.textIn(place.page, QRectF(0, word.bottom() + 1, width, height - word.bottom() - 1));
    }

    // The texts written that end below the click, from the top.
    QList<UBGraphicsTextItem*> texts;

    for (QGraphicsItem* item : scene->items(Qt::AscendingOrder))
    {
        auto text = dynamic_cast<UBGraphicsTextItem*>(item);

        if (text && text->sceneBoundingRect().bottom() > scenePoint.y())
        {
            texts << text;
        }
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
