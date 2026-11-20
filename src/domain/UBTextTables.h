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

#include <QColor>
#include <QPointF>

class QTextCursor;
class QTextDocument;
class QTextTable;

/**
 * @brief The tables of the text boxes: making them, adding and removing
 * rows and columns, their style, and the resizing of their rows and
 * columns by dragging their lines.
 *
 * A table is a table of the text of the box, saved with its HTML, so that
 * every version of OpenBoard shows it.
 */
class UBTextTables
{
public:
    /** @brief Which lines of the table are drawn. */
    enum class Borders
    {
        All,
        Horizontal,
        Outer,
        None
    };

    /** @brief A line between two rows or two columns, that can be dragged. */
    struct Edge
    {
        QTextTable* table{nullptr};
        bool column{false};   // a line between columns, else under a row
        int index{0};         // the column after the line, or the row above it
    };

    /** @brief Inserts a table at the cursor, its columns sharing the width. */
    static QTextTable* insert(QTextCursor& cursor, int rows, int columns);

    /** @brief The table the cursor is in, or none. */
    static QTextTable* tableAt(const QTextCursor& cursor);

    static void addRow(const QTextCursor& cursor);
    static void addColumn(const QTextCursor& cursor);
    static void removeRow(const QTextCursor& cursor);
    static void removeColumn(const QTextCursor& cursor);
    static void removeTable(const QTextCursor& cursor);

    /** @brief The first row, or the first column, on a grey background in bold. */
    static void setHeaderRowHighlighted(QTextTable* table, bool highlighted);
    static bool isHeaderRowHighlighted(QTextTable* table);
    static void setFirstColumnHighlighted(QTextTable* table, bool highlighted);
    static bool isFirstColumnHighlighted(QTextTable* table);

    static void setBorders(QTextTable* table, Borders borders);

    /** @brief The background of the cells of the cursor or of its selection; an invalid color clears it. */
    static void setCellBackground(const QTextCursor& cursor, const QColor& color);

    /** @brief All the columns as wide. */
    static void equalizeColumns(QTextTable* table);

    /** @brief The line of a table near this point of the document, if any. */
    static bool edgeAt(QTextDocument* document, const QPointF& point, Edge& edge);

    /** @brief Moves the line to this point of the document, resizing the row or the columns beside it. */
    static void moveEdge(QTextDocument* document, const Edge& edge, const QPointF& point);
};
