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

#include "core/UB.h"
#include "domain/UBGraphicsPageMarkItem.h"

/**
 * @brief A sound on a page, recorded with the microphone or taken from an
 * audio file, shown as a loudspeaker. A click plays it, another click stops
 * it. The sound file is kept in the document.
 */
class UBGraphicsSoundItem : public UBGraphicsPageMarkItem
{
    Q_OBJECT

public:
    enum { Type = UBGraphicsItemType::SoundItemType };

    /** @param file The path of the sound, relative to the folder of the document. */
    explicit UBGraphicsSoundItem(const QString& file, QGraphicsItem* parent = nullptr);
    ~UBGraphicsSoundItem() override;

    int type() const override { return Type; }

    QString file() const;

    /** @brief Plays the sound, or stops it when it plays. */
    void open() override;

private:
    QString mFile;
};
