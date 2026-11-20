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


#include "UBTextTables.h"

#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextTable>

namespace
{
    const QColor sHeaderColor(225, 229, 235);
    const qreal sPadding = 4;
    const qreal sGrip = 4;              // how near a line the pointer grabs it
    const qreal sMinimumColumn = 3;     // percent of the width of the table

    // The cells of a table, row by row.
    template <typename Function>
    void forEachCell(QTextTable* table, Function function)
    {
        for (int row = 0; row < table->rows(); ++row)
        {
            for (int column = 0; column < table->columns(); ++column)
            {
                function(table->cellAt(row, column), row, column);
            }
        }
    }

    void setCellHighlighted(QTextTableCell cell, bool highlighted)
    {
        QTextTableCellFormat format = cell.format().toTableCellFormat();

        if (highlighted)
        {
            format.setBackground(sHeaderColor);
            format.setFontWeight(QFont::Bold);
        }
        else
        {
            format.clearBackground();
            format.setFontWeight(QFont::Normal);
        }

        cell.setFormat(format);

        // The text already written in the cell too.
        QTextCursor cursor = cell.firstCursorPosition();
        cursor.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
        QTextCharFormat weight;
        weight.setFontWeight(highlighted ? QFont::Bold : QFont::Normal);
        cursor.mergeCharFormat(weight);
    }

    // The rectangle of a cell, from its text, in the coordinates of the document.
    QRectF cellRect(QTextDocument* document, const QTextTableCell& cell)
    {
        QAbstractTextDocumentLayout* layout = document->documentLayout();
        QRectF rect;

        for (QTextFrame::iterator it = cell.begin(); !it.atEnd(); ++it)
        {
            if (it.currentBlock().isValid())
            {
                rect = rect.united(layout->blockBoundingRect(it.currentBlock()));
            }
        }

        const QTextTableCellFormat format = cell.format().toTableCellFormat();
        return rect.adjusted(-format.leftPadding(), -format.topPadding(), format.rightPadding(), format.bottomPadding());
    }

    // The left of each column and the right of the last, from the first row.
    QList<qreal> columnLines(QTextDocument* document, QTextTable* table)
    {
        QList<qreal> lines;

        for (int column = 0; column < table->columns(); ++column)
        {
            lines << cellRect(document, table->cellAt(0, column)).left();
        }

        lines << document->documentLayout()->frameBoundingRect(table).right();
        return lines;
    }

    // The bottom of each row.
    QList<qreal> rowLines(QTextDocument* document, QTextTable* table)
    {
        QList<qreal> lines;

        for (int row = 0; row < table->rows(); ++row)
        {
            qreal bottom = 0;

            for (int column = 0; column < table->columns(); ++column)
            {
                bottom = qMax(bottom, cellRect(document, table->cellAt(row, column)).bottom());
            }

            lines << bottom;
        }

        return lines;
    }

    QList<QTextTable*> tablesOf(QTextFrame* frame)
    {
        QList<QTextTable*> tables;

        for (QTextFrame* child : frame->childFrames())
        {
            if (auto table = qobject_cast<QTextTable*>(child))
            {
                tables << table;
            }

            tables << tablesOf(child);
        }

        return tables;
    }
}

QTextTable* UBTextTables::insert(QTextCursor& cursor, int rows, int columns)
{
    QTextTableFormat format;
    format.setCellPadding(sPadding);
    format.setCellSpacing(0);
    format.setBorder(1);
    format.setBorderCollapse(true);
    format.setBorderBrush(QColor(120, 128, 140));
    format.setWidth(QTextLength(QTextLength::PercentageLength, 100));

    QList<QTextLength> widths;

    for (int i = 0; i < columns; ++i)
    {
        widths << QTextLength(QTextLength::PercentageLength, 100.0 / columns);
    }

    format.setColumnWidthConstraints(widths);
    return cursor.insertTable(rows, columns, format);
}

QTextTable* UBTextTables::tableAt(const QTextCursor& cursor)
{
    return cursor.currentTable();
}

void UBTextTables::addRow(const QTextCursor& cursor)
{
    if (QTextTable* table = tableAt(cursor))
    {
        table->insertRows(table->cellAt(cursor).row() + 1, 1);
    }
}

void UBTextTables::addColumn(const QTextCursor& cursor)
{
    QTextTable* table = tableAt(cursor);

    if (!table)
    {
        return;
    }

    table->insertColumns(table->cellAt(cursor).column() + 1, 1);
    equalizeColumns(table);
}

void UBTextTables::removeRow(const QTextCursor& cursor)
{
    QTextTable* table = tableAt(cursor);

    if (table && table->rows() > 1)
    {
        table->removeRows(table->cellAt(cursor).row(), 1);
    }
}

void UBTextTables::removeColumn(const QTextCursor& cursor)
{
    QTextTable* table = tableAt(cursor);

    if (table && table->columns() > 1)
    {
        table->removeColumns(table->cellAt(cursor).column(), 1);
        equalizeColumns(table);
    }
}

void UBTextTables::removeTable(const QTextCursor& cursor)
{
    QTextTable* table = tableAt(cursor);

    if (!table)
    {
        return;
    }

    table->removeRows(0, table->rows());
}

void UBTextTables::setHeaderRowHighlighted(QTextTable* table, bool highlighted)
{
    for (int column = 0; column < table->columns(); ++column)
    {
        setCellHighlighted(table->cellAt(0, column), highlighted);
    }
}

bool UBTextTables::isHeaderRowHighlighted(QTextTable* table)
{
    return table->columns() > 1 && table->cellAt(0, 1).format().background().color() == sHeaderColor;
}

void UBTextTables::setFirstColumnHighlighted(QTextTable* table, bool highlighted)
{
    for (int row = 0; row < table->rows(); ++row)
    {
        // The corner stays as the header row says.
        if (row == 0 && isHeaderRowHighlighted(table))
        {
            continue;
        }

        setCellHighlighted(table->cellAt(row, 0), highlighted);
    }
}

bool UBTextTables::isFirstColumnHighlighted(QTextTable* table)
{
    return table->rows() > 1 && table->cellAt(1, 0).format().background().color() == sHeaderColor;
}

void UBTextTables::setBorders(QTextTable* table, Borders borders)
{
    QTextTableFormat format = table->format();
    format.setBorder(borders == Borders::None ? 0 : 1);
    table->setFormat(format);

    const QTextFrameFormat::BorderStyle none = QTextFrameFormat::BorderStyle_None;
    const QTextFrameFormat::BorderStyle solid = QTextFrameFormat::BorderStyle_Solid;
    const int lastRow = table->rows() - 1;
    const int lastColumn = table->columns() - 1;

    forEachCell(table, [=](QTextTableCell cell, int row, int column) {
        QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
        QTextFrameFormat::BorderStyle top = solid;
        QTextFrameFormat::BorderStyle bottom = solid;
        QTextFrameFormat::BorderStyle left = solid;
        QTextFrameFormat::BorderStyle right = solid;

        if (borders == Borders::Horizontal)
        {
            left = none;
            right = none;
        }
        else if (borders == Borders::Outer)
        {
            top = row == 0 ? solid : none;
            bottom = row == lastRow ? solid : none;
            left = column == 0 ? solid : none;
            right = column == lastColumn ? solid : none;
        }
        else if (borders == Borders::None)
        {
            top = bottom = left = right = none;
        }

        cellFormat.setTopBorderStyle(top);
        cellFormat.setBottomBorderStyle(bottom);
        cellFormat.setLeftBorderStyle(left);
        cellFormat.setRightBorderStyle(right);
        cell.setFormat(cellFormat);
    });
}

void UBTextTables::setCellBackground(const QTextCursor& cursor, const QColor& color)
{
    QTextTable* table = tableAt(cursor);

    if (!table)
    {
        return;
    }

    int firstRow = table->cellAt(cursor).row();
    int rows = 1;
    int firstColumn = table->cellAt(cursor).column();
    int columns = 1;

    if (cursor.hasComplexSelection())
    {
        cursor.selectedTableCells(&firstRow, &rows, &firstColumn, &columns);
    }

    for (int row = firstRow; row < firstRow + rows; ++row)
    {
        for (int column = firstColumn; column < firstColumn + columns; ++column)
        {
            QTextTableCell cell = table->cellAt(row, column);
            QTextTableCellFormat format = cell.format().toTableCellFormat();

            if (color.isValid())
            {
                format.setBackground(color);
            }
            else
            {
                format.clearBackground();
            }

            cell.setFormat(format);
        }
    }
}

void UBTextTables::equalizeColumns(QTextTable* table)
{
    QTextTableFormat format = table->format();
    QList<QTextLength> widths;

    for (int i = 0; i < table->columns(); ++i)
    {
        widths << QTextLength(QTextLength::PercentageLength, 100.0 / table->columns());
    }

    format.setColumnWidthConstraints(widths);
    table->setFormat(format);
}

bool UBTextTables::edgeAt(QTextDocument* document, const QPointF& point, Edge& edge)
{
    for (QTextTable* table : tablesOf(document->rootFrame()))
    {
        const QRectF frame = document->documentLayout()->frameBoundingRect(table);

        if (!frame.adjusted(-sGrip, -sGrip, sGrip, sGrip).contains(point))
        {
            continue;
        }

        // The lines between columns, not the outer ones.
        const QList<qreal> columns = columnLines(document, table);

        for (int i = 1; i < columns.size() - 1; ++i)
        {
            if (std::abs(point.x() - columns.at(i)) <= sGrip)
            {
                edge = Edge{ table, true, i };
                return true;
            }
        }

        // The line under each row.
        const QList<qreal> rows = rowLines(document, table);

        for (int i = 0; i < rows.size(); ++i)
        {
            if (std::abs(point.y() - rows.at(i)) <= sGrip)
            {
                edge = Edge{ table, false, i };
                return true;
            }
        }
    }

    return false;
}

void UBTextTables::moveEdge(QTextDocument* document, const Edge& edge, const QPointF& point)
{
    QTextTable* table = edge.table;

    if (!table)
    {
        return;
    }

    if (edge.column)
    {
        // The two columns beside the line share their width differently.
        const QList<qreal> lines = columnLines(document, table);
        const qreal tableWidth = lines.last() - lines.first();

        if (tableWidth <= 0)
        {
            return;
        }

        QList<qreal> percents;

        for (int i = 0; i + 1 < lines.size(); ++i)
        {
            percents << (lines.at(i + 1) - lines.at(i)) * 100 / tableWidth;
        }

        const int left = edge.index - 1;
        const int right = edge.index;
        const qreal pair = percents.at(left) + percents.at(right);
        const qreal wanted = (point.x() - lines.at(left)) * 100 / tableWidth;
        percents[left] = qBound(sMinimumColumn, wanted, pair - sMinimumColumn);
        percents[right] = pair - percents.at(left);

        QTextTableFormat format = table->format();
        QList<QTextLength> widths;

        for (qreal percent : std::as_const(percents))
        {
            widths << QTextLength(QTextLength::PercentageLength, percent);
        }

        format.setColumnWidthConstraints(widths);
        table->setFormat(format);
        return;
    }

    // A row higher or lower: room under the text of its cells.
    const QList<qreal> bottoms = rowLines(document, table);
    const qreal growth = point.y() - bottoms.at(edge.index);

    for (int column = 0; column < table->columns(); ++column)
    {
        QTextTableCell cell = table->cellAt(edge.index, column);
        QTextTableCellFormat format = cell.format().toTableCellFormat();
        format.setBottomPadding(qMax(sPadding, format.bottomPadding() + growth));
        cell.setFormat(format);
    }
}
