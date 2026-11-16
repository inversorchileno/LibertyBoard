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

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;

/**
 * @brief Preferences page where the user chooses how the laser pointer
 * looks and how long its trail stays visible.
 */
class UBLaserPreferencesPage : public QWidget
{
    Q_OBJECT

public:
    explicit UBLaserPreferencesPage(QWidget* parent = nullptr);

    void loadSettings();
    void resetToDefaults();

private:
    void onColorChosen(int index);

    QComboBox* mColorBox{nullptr};
    QSpinBox* mWidthBox{nullptr};
    QDoubleSpinBox* mDelayBox{nullptr};
    QDoubleSpinBox* mDurationBox{nullptr};
};
