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


#include "UBGraphicsSoundItem.h"

#include <QAudioOutput>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QPointer>
#include <QUrl>

#include "core/UBApplication.h"

namespace
{
    /** One sound plays at a time, from one player. */
    QMediaPlayer* player()
    {
        static QPointer<QMediaPlayer> player;

        if (!player)
        {
            player = new QMediaPlayer(qApp);
            player->setAudioOutput(new QAudioOutput(player));
        }

        return player;
    }

    QPointer<UBGraphicsSoundItem> sPlaying;
}

UBGraphicsSoundItem::UBGraphicsSoundItem(const QString& file, QGraphicsItem* parent)
    : UBGraphicsPageMarkItem(":/images/marks/sound.svg", parent)
    , mFile(file)
{
    setToolTip(tr("Sound: click to play or stop"));
}

UBGraphicsSoundItem::~UBGraphicsSoundItem()
{
    if (sPlaying == this)
    {
        player()->stop();
    }
}

QString UBGraphicsSoundItem::file() const
{
    return mFile;
}

void UBGraphicsSoundItem::open()
{
    QMediaPlayer* sound = player();
    const bool wasPlaying = (sPlaying == this) && sound->playbackState() == QMediaPlayer::PlayingState;

    sound->stop();
    sound->disconnect(this);

    if (sPlaying)
    {
        sPlaying->setHighlighted(false);
    }

    sPlaying = nullptr;

    if (wasPlaying)
    {
        return;
    }

    sound->setSource(QUrl::fromLocalFile(documentFolder() + "/" + mFile));
    sPlaying = this;
    setHighlighted(true);

    connect(sound, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::StoppedState && sPlaying == this)
        {
            setHighlighted(false);
            sPlaying = nullptr;
        }
    });

    sound->play();
}
