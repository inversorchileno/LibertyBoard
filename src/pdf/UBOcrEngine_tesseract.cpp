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


#include "UBOcrEngine.h"

#include <QMap>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>

namespace
{
    QString program()
    {
        return QStandardPaths::findExecutable(QStringLiteral("tesseract"));
    }

    // Spanish and English when installed, else what there is.
    QString languages()
    {
        QProcess process;
        process.start(program(), { QStringLiteral("--list-langs") });
        process.waitForFinished(10000);
        const QStringList installed = QString::fromUtf8(process.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
        QStringList chosen;

        for (const QString& language : { QStringLiteral("spa"), QStringLiteral("eng") })
        {
            if (installed.contains(language))
            {
                chosen << language;
            }
        }

        return chosen.join('+');
    }
}

bool UBOcrEngine::isAvailable()
{
    return !program().isEmpty();
}

QList<UBOcrEngine::Word> UBOcrEngine::recognize(const QImage& image, QString* error)
{
    QList<Word> words;
    QTemporaryDir folder;

    if (!isAvailable() || !folder.isValid())
    {
        if (error)
        {
            *error = QStringLiteral("tesseract is not installed");
        }

        return words;
    }

    const QString picture = folder.filePath(QStringLiteral("page.png"));
    image.save(picture);

    QStringList arguments{ picture, QStringLiteral("stdout") };
    const QString chosen = languages();

    if (!chosen.isEmpty())
    {
        arguments << QStringLiteral("-l") << chosen;
    }

    arguments << QStringLiteral("tsv");

    QProcess process;
    process.start(program(), arguments);

    if (!process.waitForFinished(300000) || process.exitCode() != 0)
    {
        if (error)
        {
            *error = QString::fromUtf8(process.readAllStandardError()).trimmed();
        }

        return words;
    }

    // level, page, block, paragraph, line, word, left, top, width, height, confidence, text
    QMap<QString, int> lines;

    for (const QString& row : QString::fromUtf8(process.readAllStandardOutput()).split('\n'))
    {
        const QStringList cells = row.split('\t');

        if (cells.size() < 12 || cells.at(0) != QLatin1String("5") || cells.at(11).trimmed().isEmpty())
        {
            continue;
        }

        const QString lineKey = QString("%1.%2.%3").arg(cells.at(2).toInt(), 5, 10, QChar('0'))
                                                   .arg(cells.at(3).toInt(), 5, 10, QChar('0'))
                                                   .arg(cells.at(4).toInt(), 5, 10, QChar('0'));

        if (!lines.contains(lineKey))
        {
            lines.insert(lineKey, lines.size());
        }

        Word word;
        word.text = cells.at(11).trimmed();
        word.rect = QRectF(cells.at(6).toDouble(), cells.at(7).toDouble(), cells.at(8).toDouble(), cells.at(9).toDouble());
        word.line = lines.value(lineKey);
        words << word;
    }

    return words;
}
