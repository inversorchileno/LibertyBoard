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


#include "UBEmojis.h"

#include <QFile>
#include <QFont>
#include <QPainter>

#include "core/UBSettings.h"

QStringList UBEmojis::all()
{
    // Faces, hands, marks of a correction, school, hearts and celebrations.
    static const QStringList emojis = QString::fromUtf8(
        "😀 😃 😄 😁 😆 😅 😂 🙂 😉 😊 😇 🥰 😍 🤩 😎 🤓 🤔 😮 😲 😢 😭 😡 🥳 😴 "
        "👍 👎 👏 🙌 👋 ✋ ☝️ 👉 👆 💪 🤝 🙏 "
        "✅ ❌ ❓ ❗ ⭐ 🌟 💯 🏆 🥇 🎯 💡 📌 "
        "📚 📖 ✏️ 📝 📐 📏 🧮 🔬 🌍 ⏰ 📅 🎓 "
        "❤️ 💙 💚 💛 🧡 💜 🎉 🎈 🎁 🔥 ☀️ 🌈")
        .split(' ', Qt::SkipEmptyParts);
    return emojis;
}

QImage UBEmojis::image(const QString& emoji, int side)
{
    QImage picture(side, side, QImage::Format_ARGB32_Premultiplied);
    picture.fill(Qt::transparent);

    // The color emoji font of each system.
    QFont font;
    font.setFamilies({ "Segoe UI Emoji", "Apple Color Emoji", "Noto Color Emoji" });
    font.setPixelSize(qRound(side * 0.8));

    QPainter painter(&picture);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, side, side), Qt::AlignCenter, emoji);
    painter.end();

    return picture;
}

bool UBEmojis::putsGif()
{
    return UBSettings::settings()->emojiPutsGif->get().toBool() && !gifFile().isEmpty();
}

QString UBEmojis::emoji()
{
    const QString chosen = UBSettings::settings()->emojiChoice->get().toString();
    return chosen.isEmpty() ? all().first() : chosen;
}

QString UBEmojis::gifFile()
{
    const QString file = UBSettings::settings()->gifFile->get().toString();
    return QFile::exists(file) ? file : QString();
}

int UBEmojis::size()
{
    return qBound(16, UBSettings::settings()->emojiSize->get().toInt(), 600);
}
