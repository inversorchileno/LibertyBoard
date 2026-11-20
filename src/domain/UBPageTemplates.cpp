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


#include "UBPageTemplates.h"

#include <QBuffer>
#include <QHash>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QSvgGenerator>
#include <QtMath>

#include <cmath>

namespace
{
    // Neutral greys, half transparent, that show on a light page as well
    // as on a dark one.
    const QColor sLineColor(120, 128, 140, 130);
    const QColor sBorderColor(120, 128, 140, 210);
    const QColor sTextColor(120, 128, 140);

    // Written in the description of the SVG, to know a template behind a page.
    const char* const sTemplateMark = "LibertyBoard page template";

    struct Words
    {
        const char* spanish;
        const char* german;
        const char* french;
        const char* chinese;
        const char* japanese;
    };

    // The words of the templates, by their English text. They are part of
    // the templates, like the texts of a printed form, so they do not go
    // through the translations of the interface.
    const QHash<QString, Words>& words()
    {
        static const QHash<QString, Words> table = {
            { "To-do list", { "Lista de tareas", "Aufgabenliste", "Liste de tâches", "待办清单", "やることリスト" } },
            { "Bullet list", { "Lista con viñetas", "Aufzählung", "Liste à puces", "项目列表", "箇条書きリスト" } },
            { "Daily planner", { "Planificador diario", "Tagesplaner", "Agenda du jour", "每日计划", "デイリープランナー" } },
            { "Weekly calendar", { "Calendario semanal", "Wochenkalender", "Calendrier hebdomadaire", "周历", "週間カレンダー" } },
            { "Weekly calendar with hours", { "Calendario semanal con horas", "Wochenkalender mit Stunden",
                                              "Semaine avec horaires", "带时间的周历", "時間付き週間カレンダー" } },
            { "Monthly calendar", { "Calendario mensual", "Monatskalender", "Calendrier mensuel", "月历", "月間カレンダー" } },
            { "Year planner", { "Planificador anual", "Jahresplaner", "Planning annuel", "年度计划", "年間プランナー" } },
            { "Shopping list", { "Lista de compras", "Einkaufsliste", "Liste de courses", "购物清单", "買い物リスト" } },
            { "Habit tracker", { "Seguimiento de hábitos", "Gewohnheiten", "Suivi des habitudes", "习惯追踪", "習慣トラッカー" } },
            { "Cornell notes", { "Notas Cornell", "Cornell-Notizen", "Notes Cornell", "康奈尔笔记", "コーネル式ノート" } },
            { "Meeting notes", { "Hoja de reunión", "Besprechungsnotizen", "Compte rendu de réunion", "会议记录", "議事録" } },
            { "Dot grid", { "Hoja de puntos", "Punktraster", "Papier à points", "点阵纸", "ドット方眼" } },
            { "Handwriting", { "Pauta de caligrafía", "Schreiblineatur", "Lignes d'écriture", "书写练习纸", "書き方練習" } },
            { "Graph paper", { "Papel milimetrado", "Millimeterpapier", "Papier millimétré", "坐标纸", "方眼紙" } },
            { "Music staff", { "Pentagrama", "Notenpapier", "Portées", "五线谱", "五線譜" } },
            { "Lesson plan", { "Plan de clase", "Unterrichtsplan", "Plan de cours", "教案", "授業計画" } },
            { "Attendance", { "Lista de asistencia", "Anwesenheitsliste", "Liste de présence", "考勤表", "出席簿" } },
            { "Gradebook", { "Planilla de notas", "Notenliste", "Relevé de notes", "成绩表", "成績表" } },
            { "Two columns", { "Tabla de 2 columnas", "Zwei Spalten", "Deux colonnes", "两栏表", "2列の表" } },
            { "Mind map", { "Mapa mental", "Mindmap", "Carte mentale", "思维导图", "マインドマップ" } },
            { "Storyboard", { "Guion gráfico", "Storyboard", "Scénarimage", "故事板", "絵コンテ" } },
            { "Priorities", { "Prioridades", "Prioritäten", "Priorités", "优先事项", "優先事項" } },
            { "Notes", { "Notas", "Notizen", "Notes", "备注", "メモ" } },
            { "Product", { "Producto", "Artikel", "Article", "商品", "品名" } },
            { "Quantity", { "Cantidad", "Menge", "Quantité", "数量", "数量" } },
            { "Habit", { "Hábito", "Gewohnheit", "Habitude", "习惯", "習慣" } },
            { "Topic", { "Tema", "Thema", "Sujet", "主题", "テーマ" } },
            { "Date", { "Fecha", "Datum", "Date", "日期", "日付" } },
            { "Key ideas", { "Ideas clave", "Kernideen", "Idées clés", "要点", "キーワード" } },
            { "Summary", { "Resumen", "Zusammenfassung", "Résumé", "总结", "まとめ" } },
            { "Time", { "Hora", "Uhrzeit", "Heure", "时间", "時間" } },
            { "Attendees", { "Asistentes", "Teilnehmer", "Participants", "参会人员", "出席者" } },
            { "Topics", { "Temas", "Themen", "Points abordés", "议题", "議題" } },
            { "Agreements", { "Acuerdos", "Beschlüsse", "Décisions", "决议", "決定事項" } },
            { "Tasks", { "Tareas", "Aufgaben", "Tâches", "任务", "タスク" } },
            { "Subject", { "Asignatura", "Fach", "Matière", "科目", "教科" } },
            { "Class", { "Curso", "Klasse", "Classe", "班级", "クラス" } },
            { "Objective", { "Objetivo", "Lernziel", "Objectif", "教学目标", "目標" } },
            { "Start", { "Inicio", "Einstieg", "Introduction", "导入", "導入" } },
            { "Development", { "Desarrollo", "Erarbeitung", "Développement", "展开", "展開" } },
            { "Closing", { "Cierre", "Abschluss", "Conclusion", "总结", "まとめ" } },
            { "Materials", { "Materiales", "Material", "Matériel", "教学材料", "教材" } },
            { "Name", { "Nombre", "Name", "Nom", "姓名", "氏名" } },
            { "Average", { "Promedio", "Durchschnitt", "Moyenne", "平均", "平均" } },
            { "Pros", { "A favor", "Pro", "Pour", "优点", "賛成" } },
            { "Cons", { "En contra", "Contra", "Contre", "缺点", "反対" } },
            { "Scene", { "Escena", "Szene", "Scène", "场景", "シーン" } },
        };

        return table;
    }

    // Each word starts with a capital, like a title.
    QString capitalized(const QString& text)
    {
        return text.isEmpty() ? text : text.left(1).toUpper() + text.mid(1);
    }
}

QList<UBPageTemplates::Template> UBPageTemplates::all()
{
    return { Template::TodoList, Template::BulletList, Template::DailyPlanner, Template::WeeklyCalendar,
             Template::WeeklyHours, Template::MonthlyCalendar, Template::YearPlanner, Template::ShoppingList,
             Template::HabitTracker, Template::CornellNotes, Template::Meeting, Template::DotGrid,
             Template::Handwriting, Template::GraphPaper, Template::MusicStaff, Template::LessonPlan,
             Template::Attendance, Template::Gradebook, Template::TwoColumns, Template::MindMap,
             Template::Storyboard };
}

UBPageTemplates::UBPageTemplates(const QString& language)
{
    const QString code = language.left(2).toLower();
    const QStringList known = { "es", "de", "fr", "zh", "ja" };
    mLanguage = known.contains(code) ? code : QStringLiteral("en");
}

void UBPageTemplates::setWeekStartsOnSunday(bool sunday)
{
    mFirstDay = sunday ? Qt::Sunday : Qt::Monday;
}

QString UBPageTemplates::word(const char* english) const
{
    const auto found = words().constFind(QString::fromUtf8(english));

    if (mLanguage == "en" || found == words().constEnd())
    {
        return QString::fromUtf8(english);
    }

    const Words& translated = found.value();
    const char* text = mLanguage == "es"   ? translated.spanish
                       : mLanguage == "de" ? translated.german
                       : mLanguage == "fr" ? translated.french
                       : mLanguage == "zh" ? translated.chinese
                                           : translated.japanese;
    return QString::fromUtf8(text);
}

QString UBPageTemplates::name(Template pageTemplate) const
{
    switch (pageTemplate)
    {
    case Template::TodoList: return word("To-do list");
    case Template::BulletList: return word("Bullet list");
    case Template::DailyPlanner: return word("Daily planner");
    case Template::WeeklyCalendar: return word("Weekly calendar");
    case Template::WeeklyHours: return word("Weekly calendar with hours");
    case Template::MonthlyCalendar: return word("Monthly calendar");
    case Template::YearPlanner: return word("Year planner");
    case Template::ShoppingList: return word("Shopping list");
    case Template::HabitTracker: return word("Habit tracker");
    case Template::CornellNotes: return word("Cornell notes");
    case Template::Meeting: return word("Meeting notes");
    case Template::DotGrid: return word("Dot grid");
    case Template::Handwriting: return word("Handwriting");
    case Template::GraphPaper: return word("Graph paper");
    case Template::MusicStaff: return word("Music staff");
    case Template::LessonPlan: return word("Lesson plan");
    case Template::Attendance: return word("Attendance");
    case Template::Gradebook: return word("Gradebook");
    case Template::TwoColumns: return word("Two columns");
    case Template::MindMap: return word("Mind map");
    case Template::Storyboard: return word("Storyboard");
    }

    return QString();
}

bool UBPageTemplates::isDated(Template pageTemplate)
{
    return pageTemplate == Template::DailyPlanner || pageTemplate == Template::WeeklyCalendar
           || pageTemplate == Template::WeeklyHours || pageTemplate == Template::MonthlyCalendar
           || pageTemplate == Template::YearPlanner || pageTemplate == Template::HabitTracker;
}

static QLocale localeOf(const QString& language)
{
    if (language == "es") return QLocale(QLocale::Spanish, QLocale::Spain);
    if (language == "de") return QLocale(QLocale::German, QLocale::Germany);
    if (language == "fr") return QLocale(QLocale::French, QLocale::France);
    if (language == "zh") return QLocale(QLocale::Chinese, QLocale::China);
    if (language == "ja") return QLocale(QLocale::Japanese, QLocale::Japan);
    return QLocale(QLocale::English, QLocale::UnitedStates);
}

QString UBPageTemplates::monthTitle(const QDate& date) const
{
    if (mLanguage == "zh" || mLanguage == "ja")
    {
        return QStringLiteral("%1年%2月").arg(date.year()).arg(date.month());
    }

    return capitalized(localeOf(mLanguage).standaloneMonthName(date.month())) + " " + QString::number(date.year());
}

QString UBPageTemplates::dayName(int dayOfWeek, bool shortName) const
{
    const QLocale locale = localeOf(mLanguage);
    QString name = locale.standaloneDayName(dayOfWeek, shortName ? QLocale::ShortFormat : QLocale::LongFormat);
    name.remove(QChar('.'));
    return capitalized(name);
}

QList<QDate> UBPageTemplates::weekOf(const QDate& date) const
{
    const int firstDay = mFirstDay;
    const QDate start = date.addDays(-((date.dayOfWeek() - firstDay + 7) % 7));
    QList<QDate> days;

    for (int i = 0; i < 7; ++i)
    {
        days << start.addDays(i);
    }

    return days;
}

// ---------------------------------------------------------------------------
// Common parts

static qreal unitOf(const QRectF& rect)
{
    return qMin(rect.width(), rect.height()) / 100.0;
}

void UBPageTemplates::drawLabel(QPainter* painter, const QRectF& area, const QString& text, int flags, qreal size,
                                bool bold) const
{
    QFont font(QStringLiteral("Arial"));
    font.setPixelSize(qMax(1, qRound(size)));
    font.setBold(bold);
    painter->setFont(font);
    painter->setPen(sTextColor);
    painter->drawText(area, flags, text);
}

void UBPageTemplates::drawTitle(QPainter* painter, const QRectF& rect, const QString& title, const QString& right) const
{
    const qreal u = unitOf(rect);
    const QRectF titleArea(rect.left() + 5 * u, rect.top() + 3 * u, rect.width() - 10 * u, 7 * u);

    drawLabel(painter, titleArea, title, Qt::AlignLeft | Qt::AlignVCenter, 4.2 * u, true);

    if (!right.isEmpty())
    {
        drawLabel(painter, titleArea, right, Qt::AlignRight | Qt::AlignVCenter, 3 * u);
    }

    painter->setPen(QPen(sBorderColor, 0.3 * u));
    painter->drawLine(titleArea.bottomLeft(), titleArea.bottomRight());
}

QRectF UBPageTemplates::body(const QRectF& rect) const
{
    const qreal u = unitOf(rect);
    return QRectF(rect.left() + 5 * u, rect.top() + 12 * u, rect.width() - 10 * u, rect.height() - 16 * u);
}

void UBPageTemplates::drawLines(QPainter* painter, const QRectF& area, qreal spacing, bool withBoxes, bool withBullets) const
{
    painter->setPen(QPen(sLineColor, spacing / 25));

    for (qreal y = area.top() + spacing; y <= area.bottom() + 0.5; y += spacing)
    {
        qreal left = area.left();

        if (withBoxes)
        {
            const qreal box = spacing * 0.45;
            painter->setPen(QPen(sBorderColor, spacing / 25));
            painter->drawRoundedRect(QRectF(left, y - spacing * 0.2 - box, box, box), box * 0.15, box * 0.15);
            painter->setPen(QPen(sLineColor, spacing / 25));
            left += box * 1.8;
        }
        else if (withBullets)
        {
            const qreal dot = spacing * 0.14;
            painter->setBrush(sBorderColor);
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QPointF(left + dot, y - spacing * 0.35), dot, dot);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(sLineColor, spacing / 25));
            left += dot * 5;
        }

        painter->drawLine(QPointF(left, y), QPointF(area.right(), y));
    }
}

void UBPageTemplates::drawGrid(QPainter* painter, const QRectF& area, int columns, int rows) const
{
    const qreal width = qMin(area.width(), area.height()) / 400;
    painter->setPen(QPen(sLineColor, width));

    for (int i = 1; i < columns; ++i)
    {
        const qreal x = area.left() + area.width() * i / columns;
        painter->drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
    }

    for (int i = 1; i < rows; ++i)
    {
        const qreal y = area.top() + area.height() * i / rows;
        painter->drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
    }

    painter->setPen(QPen(sBorderColor, width * 1.5));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(area);
}

void UBPageTemplates::drawMonth(QPainter* painter, const QRectF& area, const QDate& month, bool compact) const
{
    const QDate first(month.year(), month.month(), 1);
    const int firstDay = mFirstDay;
    const int offset = (first.dayOfWeek() - firstDay + 7) % 7;
    const int weeks = (offset + first.daysInMonth() + 6) / 7;

    QRectF grid = area;

    if (compact)
    {
        const qreal titleHeight = area.height() * 0.16;
        drawLabel(painter, QRectF(area.left(), area.top(), area.width(), titleHeight),
                  mLanguage == "zh" || mLanguage == "ja" ? QStringLiteral("%1月").arg(month.month())
                                                         : capitalized(localeOf(mLanguage).standaloneMonthName(month.month())),
                  Qt::AlignLeft | Qt::AlignVCenter, titleHeight * 0.7, true);
        grid.setTop(area.top() + titleHeight);
    }

    const qreal headerHeight = compact ? grid.height() / 7 : grid.height() * 0.07;
    const qreal cellWidth = grid.width() / 7;
    const qreal cellHeight = (grid.height() - headerHeight) / (compact ? 6 : weeks);

    for (int i = 0; i < 7; ++i)
    {
        const int dayOfWeek = (firstDay - 1 + i) % 7 + 1;
        const QString name = compact ? dayName(dayOfWeek, true).left(mLanguage == "en" || mLanguage == "de" ? 2 : 1)
                                   : dayName(dayOfWeek, false);
        drawLabel(painter, QRectF(grid.left() + i * cellWidth, grid.top(), cellWidth, headerHeight), name,
                  Qt::AlignCenter, headerHeight * (compact ? 0.6 : 0.45), !compact);
    }

    const QRectF days(grid.left(), grid.top() + headerHeight, grid.width(), cellHeight * (compact ? 6 : weeks));

    if (!compact)
    {
        drawGrid(painter, days, 7, weeks);
    }

    for (int day = 1; day <= first.daysInMonth(); ++day)
    {
        const int index = offset + day - 1;
        const QRectF cell(days.left() + (index % 7) * cellWidth, days.top() + (index / 7) * cellHeight, cellWidth, cellHeight);

        if (compact)
        {
            drawLabel(painter, cell, QString::number(day), Qt::AlignCenter, cellHeight * 0.55);
        }
        else
        {
            const qreal size = qMin(cellHeight * 0.2, cellWidth * 0.16);
            drawLabel(painter, cell.adjusted(size * 0.4, size * 0.2, 0, 0), QString::number(day),
                      Qt::AlignLeft | Qt::AlignTop, size, true);
        }
    }
}

// ---------------------------------------------------------------------------
// The templates

void UBPageTemplates::drawDailyPlanner(QPainter* painter, const QRectF& rect, const QDate& date) const
{
    drawTitle(painter, rect, name(Template::DailyPlanner),
              capitalized(localeOf(mLanguage).toString(date, QLocale::LongFormat)));

    const QRectF area = body(rect);
    const qreal u = unitOf(rect);
    const QRectF hours(area.left(), area.top(), area.width() * 0.58, area.height());
    const QRectF side(hours.right() + 4 * u, area.top(), area.right() - hours.right() - 4 * u, area.height());
    const int count = 16; // from 7:00 to 22:00
    const qreal rowHeight = hours.height() / count;

    for (int i = 0; i < count; ++i)
    {
        const QRectF row(hours.left(), hours.top() + i * rowHeight, hours.width(), rowHeight);
        drawLabel(painter, QRectF(row.left(), row.top(), 7 * u, row.height()),
                  QStringLiteral("%1:00").arg(7 + i), Qt::AlignLeft | Qt::AlignVCenter, qMin(rowHeight * 0.4, 2.4 * u));
        painter->setPen(QPen(sLineColor, 0.15 * u));
        painter->drawLine(row.bottomLeft(), row.bottomRight());
    }

    const qreal labelHeight = 4 * u;
    const QRectF priorities(side.left(), side.top(), side.width(), side.height() * 0.42);
    drawLabel(painter, QRectF(priorities.topLeft(), QSizeF(priorities.width(), labelHeight)), word("Priorities"),
              Qt::AlignLeft | Qt::AlignVCenter, 2.8 * u, true);
    drawLines(painter, priorities.adjusted(0, labelHeight, 0, 0), (priorities.height() - labelHeight) / 5, true);

    const QRectF notes(side.left(), priorities.bottom() + 3 * u, side.width(), side.bottom() - priorities.bottom() - 3 * u);
    drawLabel(painter, QRectF(notes.topLeft(), QSizeF(notes.width(), labelHeight)), word("Notes"),
              Qt::AlignLeft | Qt::AlignVCenter, 2.8 * u, true);
    drawLines(painter, notes.adjusted(0, labelHeight, 0, 0), 5 * u);
}

void UBPageTemplates::drawWeek(QPainter* painter, const QRectF& rect, const QDate& date, bool withHours) const
{
    const QList<QDate> days = weekOf(date);
    const QLocale locale = localeOf(mLanguage);
    const QString range = locale.toString(days.first(), mLanguage == "zh" || mLanguage == "ja" ? "M月d日" : "d MMM")
                          + QStringLiteral(" – ")
                          + locale.toString(days.last(), mLanguage == "zh" || mLanguage == "ja" ? "M月d日" : "d MMM yyyy");
    drawTitle(painter, rect, monthTitle(days.first()), range);

    QRectF area = body(rect);
    const qreal u = unitOf(rect);
    const bool wide = rect.width() >= rect.height();

    if (withHours || wide)
    {
        // Seven columns side by side, with the hours on the left.
        const qreal hoursWidth = withHours ? 6 * u : 0;
        const QRectF columns(area.left() + hoursWidth, area.top(), area.width() - hoursWidth, area.height());
        const qreal headerHeight = 7 * u;
        const qreal columnWidth = columns.width() / 7;

        for (int i = 0; i < 7; ++i)
        {
            const QRectF header(columns.left() + i * columnWidth, columns.top(), columnWidth, headerHeight);
            drawLabel(painter, header.adjusted(0, 0, 0, -headerHeight / 2), dayName(days.at(i).dayOfWeek(), !wide),
                      Qt::AlignCenter, 2.4 * u, true);
            drawLabel(painter, header.adjusted(0, headerHeight / 2, 0, 0), QString::number(days.at(i).day()),
                      Qt::AlignCenter, 2.6 * u);
        }

        const QRectF grid(columns.left(), columns.top() + headerHeight, columns.width(), columns.height() - headerHeight);

        if (withHours)
        {
            const int count = 14; // from 7:00 to 20:00
            drawGrid(painter, grid, 7, count);

            for (int i = 0; i < count; ++i)
            {
                drawLabel(painter, QRectF(area.left(), grid.top() + i * grid.height() / count, hoursWidth - u, grid.height() / count),
                          QStringLiteral("%1:00").arg(7 + i), Qt::AlignRight | Qt::AlignTop, 2 * u);
            }
        }
        else
        {
            drawGrid(painter, grid, 7, 1);
        }

        return;
    }

    // On a page standing up: two columns of four boxes, the last for notes.
    const qreal boxWidth = area.width() / 2;
    const qreal boxHeight = area.height() / 4;

    for (int i = 0; i < 8; ++i)
    {
        const QRectF box(area.left() + (i / 4) * boxWidth, area.top() + (i % 4) * boxHeight, boxWidth, boxHeight);
        const QString label = i < 7 ? dayName(days.at(i).dayOfWeek(), false) + " " + QString::number(days.at(i).day())
                                    : word("Notes");
        drawGrid(painter, box, 1, 1);
        drawLabel(painter, box.adjusted(1.5 * u, u, 0, 0), label, Qt::AlignLeft | Qt::AlignTop, 2.6 * u, true);
    }
}

void UBPageTemplates::drawYear(QPainter* painter, const QRectF& rect, const QDate& date) const
{
    drawTitle(painter, rect, QString::number(date.year()) + (mLanguage == "zh" || mLanguage == "ja" ? "年" : ""));

    const QRectF area = body(rect);
    const qreal u = unitOf(rect);
    const bool wide = rect.width() >= rect.height();
    const int columns = wide ? 4 : 3;
    const int rows = 12 / columns;
    const qreal cellWidth = area.width() / columns;
    const qreal cellHeight = area.height() / rows;

    for (int i = 0; i < 12; ++i)
    {
        const QRectF cell(area.left() + (i % columns) * cellWidth, area.top() + (i / columns) * cellHeight, cellWidth, cellHeight);
        drawMonth(painter, cell.adjusted(1.5 * u, u, -1.5 * u, -u), QDate(date.year(), i + 1, 1), true);
    }
}

void UBPageTemplates::drawHabits(QPainter* painter, const QRectF& rect, const QDate& date) const
{
    drawTitle(painter, rect, name(Template::HabitTracker), monthTitle(date));

    const QRectF area = body(rect);
    const qreal u = unitOf(rect);
    const int days = date.daysInMonth();
    const int rows = 12;
    const qreal nameWidth = area.width() * 0.22;
    const qreal headerHeight = 6 * u;
    const QRectF grid(area.left() + nameWidth, area.top() + headerHeight, area.width() - nameWidth, area.height() - headerHeight);
    const qreal dayWidth = grid.width() / days;

    drawLabel(painter, QRectF(area.left(), area.top(), nameWidth, headerHeight), word("Habit"),
              Qt::AlignLeft | Qt::AlignVCenter, 2.6 * u, true);

    for (int day = 1; day <= days; ++day)
    {
        const QDate current(date.year(), date.month(), day);
        const QRectF header(grid.left() + (day - 1) * dayWidth, area.top(), dayWidth, headerHeight);
        drawLabel(painter, header.adjusted(0, 0, 0, -headerHeight / 2), dayName(current.dayOfWeek(), true).left(1),
                  Qt::AlignCenter, qMin(1.8 * u, dayWidth * 0.6));
        drawLabel(painter, header.adjusted(0, headerHeight / 2, 0, 0), QString::number(day),
                  Qt::AlignCenter, qMin(1.8 * u, dayWidth * 0.5), true);
    }

    drawGrid(painter, QRectF(area.left(), grid.top(), area.width(), grid.height()), 1, rows);
    drawGrid(painter, grid, days, rows);
}

void UBPageTemplates::drawCornell(QPainter* painter, const QRectF& rect) const
{
    drawTitle(painter, rect, word("Topic") + ":", word("Date") + ":                    ");

    const QRectF area = body(rect);
    const qreal u = unitOf(rect);
    const QRectF summary(area.left(), area.bottom() - area.height() * 0.22, area.width(), area.height() * 0.22);
    const QRectF cues(area.left(), area.top(), area.width() * 0.3, summary.top() - area.top());
    const QRectF notes(cues.right(), area.top(), area.right() - cues.right(), cues.height());

    painter->setPen(QPen(sBorderColor, 0.3 * u));
    painter->drawLine(QPointF(cues.right(), cues.top()), QPointF(cues.right(), cues.bottom()));
    painter->drawLine(summary.topLeft(), summary.topRight());

    drawLabel(painter, cues.adjusted(u, u, 0, 0), word("Key ideas"), Qt::AlignLeft | Qt::AlignTop, 2.6 * u, true);
    drawLabel(painter, notes.adjusted(2 * u, u, 0, 0), word("Notes"), Qt::AlignLeft | Qt::AlignTop, 2.6 * u, true);
    drawLines(painter, notes.adjusted(2 * u, 5 * u, 0, 0), 5 * u);
    drawLabel(painter, summary.adjusted(u, u, 0, 0), word("Summary"), Qt::AlignLeft | Qt::AlignTop, 2.6 * u, true);
    drawLines(painter, summary.adjusted(u, 5 * u, 0, 0), 5 * u);
}

void UBPageTemplates::drawSections(QPainter* painter, const QRectF& rect, const QString& title, const QStringList& fields,
                                   const QStringList& sections) const
{
    drawTitle(painter, rect, title);

    QRectF area = body(rect);
    const qreal u = unitOf(rect);
    const qreal fieldHeight = 6 * u;
    const qreal fieldWidth = area.width() / fields.size();

    for (int i = 0; i < fields.size(); ++i)
    {
        const QRectF field(area.left() + i * fieldWidth, area.top(), fieldWidth - 2 * u, fieldHeight);
        drawLabel(painter, field, fields.at(i) + ":", Qt::AlignLeft | Qt::AlignVCenter, 2.6 * u, true);
        painter->setPen(QPen(sLineColor, 0.15 * u));
        painter->drawLine(field.bottomLeft(), field.bottomRight());
    }

    area.setTop(area.top() + fieldHeight + 2 * u);

    // In two columns on a wide page, one under the other on a page standing up.
    const int columns = rect.width() >= rect.height() * 1.2 ? 2 : 1;
    const int rows = (sections.size() + columns - 1) / columns;
    const qreal width = area.width() / columns;
    const qreal height = area.height() / rows;

    for (int i = 0; i < sections.size(); ++i)
    {
        const int column = i / rows;
        const int row = i % rows;
        const QRectF section(area.left() + column * width + (column > 0 ? 2 * u : 0), area.top() + row * height,
                             width - 2 * u, height - 2 * u);
        drawLabel(painter, QRectF(section.topLeft(), QSizeF(section.width(), 4 * u)), sections.at(i),
                  Qt::AlignLeft | Qt::AlignVCenter, 2.8 * u, true);
        drawLines(painter, section.adjusted(0, 4 * u, 0, 0), 5 * u);
    }
}

void UBPageTemplates::drawTable(QPainter* painter, const QRectF& rect, const QString& title, const QStringList& headers,
                                const QList<qreal>& widths, int rows) const
{
    drawTitle(painter, rect, title);

    const QRectF area = body(rect);
    const qreal u = unitOf(rect);
    const qreal rowHeight = area.height() / (rows + 1);
    qreal x = area.left();

    for (int i = 0; i < widths.size(); ++i)
    {
        const qreal width = area.width() * widths.at(i);

        if (i < headers.size())
        {
            drawLabel(painter, QRectF(x + u, area.top(), width - 2 * u, rowHeight), headers.at(i),
                      Qt::AlignLeft | Qt::AlignVCenter, qMin(2.6 * u, rowHeight * 0.5), true);
        }

        drawGrid(painter, QRectF(x, area.top(), width, area.height()), 1, rows + 1);
        x += width;
    }
}

void UBPageTemplates::drawDots(QPainter* painter, const QRectF& rect) const
{
    const qreal u = unitOf(rect);
    const qreal spacing = 5 * u;
    painter->setPen(Qt::NoPen);
    painter->setBrush(sBorderColor);

    for (qreal y = rect.top() + spacing; y < rect.bottom(); y += spacing)
    {
        for (qreal x = rect.left() + spacing; x < rect.right(); x += spacing)
        {
            painter->drawEllipse(QPointF(x, y), 0.25 * u, 0.25 * u);
        }
    }
}

void UBPageTemplates::drawHandwriting(QPainter* painter, const QRectF& rect) const
{
    // Groups of four lines: the top of the capitals, the middle dashed,
    // the line where letters stand and the bottom of g, p and y.
    const qreal u = unitOf(rect);
    const qreal step = 3 * u;
    const qreal group = 4 * step;
    QPen solid(sLineColor, 0.15 * u);
    QPen strong(sBorderColor, 0.25 * u);
    QPen dashed(sLineColor, 0.15 * u, Qt::DashLine);

    for (qreal top = rect.top() + 5 * u; top + 3 * step < rect.bottom() - 3 * u; top += group)
    {
        const qreal y[4] = { top, top + step, top + 2 * step, top + 3 * step };
        painter->setPen(solid);
        painter->drawLine(QPointF(rect.left() + 5 * u, y[0]), QPointF(rect.right() - 5 * u, y[0]));
        painter->setPen(dashed);
        painter->drawLine(QPointF(rect.left() + 5 * u, y[1]), QPointF(rect.right() - 5 * u, y[1]));
        painter->setPen(strong);
        painter->drawLine(QPointF(rect.left() + 5 * u, y[2]), QPointF(rect.right() - 5 * u, y[2]));
        painter->setPen(solid);
        painter->drawLine(QPointF(rect.left() + 5 * u, y[3]), QPointF(rect.right() - 5 * u, y[3]));
    }
}

void UBPageTemplates::drawGraphPaper(QPainter* painter, const QRectF& rect) const
{
    const qreal u = unitOf(rect);
    const qreal minor = u;
    QColor light = sLineColor;
    light.setAlpha(60);
    int index = 0;

    for (qreal x = rect.left(); x <= rect.right(); x += minor, ++index)
    {
        painter->setPen(QPen(index % 5 == 0 ? sLineColor : light, index % 5 == 0 ? 0.12 * u : 0.06 * u));
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }

    index = 0;

    for (qreal y = rect.top(); y <= rect.bottom(); y += minor, ++index)
    {
        painter->setPen(QPen(index % 5 == 0 ? sLineColor : light, index % 5 == 0 ? 0.12 * u : 0.06 * u));
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

void UBPageTemplates::drawStaves(QPainter* painter, const QRectF& rect) const
{
    const qreal u = unitOf(rect);
    const qreal line = 1.3 * u;
    const qreal staff = 4 * line;
    const qreal gap = 5 * u;
    painter->setPen(QPen(sBorderColor, 0.15 * u));

    for (qreal top = rect.top() + 6 * u; top + staff < rect.bottom() - 4 * u; top += staff + gap)
    {
        for (int i = 0; i < 5; ++i)
        {
            painter->drawLine(QPointF(rect.left() + 5 * u, top + i * line), QPointF(rect.right() - 5 * u, top + i * line));
        }

        painter->drawLine(QPointF(rect.left() + 5 * u, top), QPointF(rect.left() + 5 * u, top + staff));
        painter->drawLine(QPointF(rect.right() - 5 * u, top), QPointF(rect.right() - 5 * u, top + staff));
    }
}

void UBPageTemplates::drawMindMap(QPainter* painter, const QRectF& rect) const
{
    const qreal u = unitOf(rect);
    const QPointF center = rect.center();
    const QSizeF middle(32 * u, 16 * u);
    const QSizeF leaf(22 * u, 10 * u);
    const qreal rx = rect.width() / 2 - leaf.width() / 2 - 5 * u;
    const qreal ry = rect.height() / 2 - leaf.height() / 2 - 5 * u;

    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < 6; ++i)
    {
        const qreal angle = M_PI / 6 + i * M_PI / 3;
        const QPointF end(center.x() + rx * std::cos(angle), center.y() + ry * std::sin(angle));
        QPainterPath branch(center);
        branch.cubicTo(QPointF((center.x() + end.x()) / 2, center.y()), QPointF((center.x() + end.x()) / 2, end.y()), end);
        painter->setPen(QPen(sLineColor, 0.4 * u));
        painter->drawPath(branch);
        painter->setPen(QPen(sBorderColor, 0.3 * u));
        painter->drawEllipse(QRectF(end - QPointF(leaf.width() / 2, leaf.height() / 2), leaf));
    }

    painter->setPen(QPen(sBorderColor, 0.5 * u));
    painter->drawEllipse(QRectF(center - QPointF(middle.width() / 2, middle.height() / 2), middle));
}

void UBPageTemplates::drawStoryboard(QPainter* painter, const QRectF& rect) const
{
    drawTitle(painter, rect, name(Template::Storyboard));

    const QRectF area = body(rect);
    const qreal u = unitOf(rect);
    const bool wide = rect.width() >= rect.height();
    const int columns = wide ? 3 : 2;
    const int rows = wide ? 2 : 3;
    const qreal cellWidth = area.width() / columns;
    const qreal cellHeight = area.height() / rows;

    for (int i = 0; i < columns * rows; ++i)
    {
        const QRectF cell(area.left() + (i % columns) * cellWidth, area.top() + (i / columns) * cellHeight,
                          cellWidth, cellHeight);
        const QRectF frame = cell.adjusted(1.5 * u, 4 * u, -1.5 * u, -cellHeight * 0.3);
        drawLabel(painter, QRectF(cell.left() + 1.5 * u, cell.top(), cell.width(), 4 * u),
                  word("Scene") + " " + QString::number(i + 1), Qt::AlignLeft | Qt::AlignVCenter, 2.4 * u, true);
        drawGrid(painter, frame, 1, 1);
        drawLines(painter, QRectF(frame.left(), frame.bottom(), frame.width(), cell.bottom() - frame.bottom() - u), 4 * u);
    }
}

void UBPageTemplates::draw(QPainter* painter, const QRectF& rect, Template pageTemplate, const QDate& date) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);

    const qreal u = unitOf(rect);

    switch (pageTemplate)
    {
    case Template::TodoList:
        drawTitle(painter, rect, name(pageTemplate));
        drawLines(painter, body(rect), 6 * u, true);
        break;
    case Template::BulletList:
        drawTitle(painter, rect, name(pageTemplate));
        drawLines(painter, body(rect), 6 * u, false, true);
        break;
    case Template::DailyPlanner:
        drawDailyPlanner(painter, rect, date);
        break;
    case Template::WeeklyCalendar:
        drawWeek(painter, rect, date, false);
        break;
    case Template::WeeklyHours:
        drawWeek(painter, rect, date, true);
        break;
    case Template::MonthlyCalendar:
        drawTitle(painter, rect, monthTitle(date));
        drawMonth(painter, body(rect), date, false);
        break;
    case Template::YearPlanner:
        drawYear(painter, rect, date);
        break;
    case Template::ShoppingList:
        drawTable(painter, rect, name(pageTemplate), { QString(), word("Product"), word("Quantity") }, { 0.07, 0.68, 0.25 }, 16);
        break;
    case Template::HabitTracker:
        drawHabits(painter, rect, date);
        break;
    case Template::CornellNotes:
        drawCornell(painter, rect);
        break;
    case Template::Meeting:
        drawSections(painter, rect, name(pageTemplate), { word("Date"), word("Time") },
                     { word("Attendees"), word("Topics"), word("Agreements"), word("Tasks") });
        break;
    case Template::DotGrid:
        drawDots(painter, rect);
        break;
    case Template::Handwriting:
        drawHandwriting(painter, rect);
        break;
    case Template::GraphPaper:
        drawGraphPaper(painter, rect);
        break;
    case Template::MusicStaff:
        drawStaves(painter, rect);
        break;
    case Template::LessonPlan:
        drawSections(painter, rect, name(pageTemplate), { word("Subject"), word("Class"), word("Date") },
                     { word("Objective"), word("Start"), word("Development"), word("Closing"), word("Materials"), word("Notes") });
        break;
    case Template::Attendance:
    {
        QList<qreal> widths = { 0.28 };
        for (int i = 0; i < 18; ++i)
        {
            widths << 0.72 / 18;
        }
        drawTable(painter, rect, name(pageTemplate), { word("Name") }, widths, 25);
        break;
    }
    case Template::Gradebook:
    {
        QList<qreal> widths = { 0.3 };
        for (int i = 0; i < 8; ++i)
        {
            widths << 0.58 / 8;
        }
        widths << 0.12;
        QStringList headers = { word("Name") };
        for (int i = 0; i < 8; ++i)
        {
            headers << QString();
        }
        headers << word("Average");
        drawTable(painter, rect, name(pageTemplate), headers, widths, 25);
        break;
    }
    case Template::TwoColumns:
    {
        drawTitle(painter, rect, name(pageTemplate));
        const QRectF area = body(rect);
        const QRectF left(area.left(), area.top(), area.width() / 2 - 2 * u, area.height());
        const QRectF right(area.center().x() + 2 * u, area.top(), area.width() / 2 - 2 * u, area.height());
        drawLabel(painter, QRectF(left.topLeft(), QSizeF(left.width(), 5 * u)), word("Pros"), Qt::AlignCenter, 3 * u, true);
        drawLabel(painter, QRectF(right.topLeft(), QSizeF(right.width(), 5 * u)), word("Cons"), Qt::AlignCenter, 3 * u, true);
        drawLines(painter, left.adjusted(0, 5 * u, 0, 0), 5 * u);
        drawLines(painter, right.adjusted(0, 5 * u, 0, 0), 5 * u);
        painter->setPen(QPen(sBorderColor, 0.3 * u));
        painter->drawLine(QPointF(area.center().x(), area.top()), QPointF(area.center().x(), area.bottom()));
        break;
    }
    case Template::MindMap:
        drawMindMap(painter, rect);
        break;
    case Template::Storyboard:
        drawStoryboard(painter, rect);
        break;
    }

    painter->restore();
}

QByteArray UBPageTemplates::svg(Template pageTemplate, const QSizeF& pageSize, const QDate& date) const
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    QSvgGenerator generator;
    generator.setOutputDevice(&buffer);
    generator.setSize(pageSize.toSize());
    generator.setViewBox(QRectF(QPointF(0, 0), pageSize));
    generator.setTitle(name(pageTemplate));
    generator.setDescription(QString::fromLatin1(sTemplateMark));

    QPainter painter(&generator);
    draw(&painter, QRectF(QPointF(0, 0), pageSize), pageTemplate, date);
    painter.end();

    return buffer.data();
}

bool UBPageTemplates::isTemplate(const QByteArray& svg)
{
    return svg.contains(sTemplateMark);
}
