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

#include <QMap>
#include <QWidget>

class QHBoxLayout;
class QLabel;
class UBToolOptionsPanel;

/**
 * @brief Shows the options of the selected tool in one row of the board
 * toolbar, so that they are always at hand without taking room on the board.
 */
class UBToolOptionsBar : public QWidget
{
    Q_OBJECT

public:
    explicit UBToolOptionsBar(QWidget* parent = nullptr);

    void showTool(int tool);

    QSize sizeHint() const override;

protected:
    void showEvent(QShowEvent* event) override;

private:
    void onStylusToolChanged(int tool);
    void watchActiveScene();
    void onTextBoxChanged();

    QWidget* mContent{nullptr};
    QHBoxLayout* mContentLayout{nullptr};
    QLabel* mNoOptions{nullptr};
    QMap<int, UBToolOptionsPanel*> mPanels;
    UBToolOptionsPanel* mCurrentPanel{nullptr};
};
