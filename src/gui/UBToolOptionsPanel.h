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

#include "core/UBSettings.h"

class QCheckBox;
class QFontComboBox;
class QToolButton;
class QVBoxLayout;
class UBColorSwatches;
class UBPictureChoice;
class UBSegmentedChoice;
class UBStrokePreview;
class UBValueSlider;

/**
 * @brief Base of the panels where the user sets the options of one tool,
 * laid out like the tool panels of Drawboard PDF.
 *
 * A panel reads its values from the settings when it is shown and writes
 * every change back immediately, so that the next stroke uses it.
 */
class UBToolOptionsPanel : public QWidget
{
    Q_OBJECT

public:
    /** @brief Returns a new panel for the tool, or nullptr when the tool has no options. */
    static UBToolOptionsPanel* create(int tool, QWidget* parent = nullptr);

    /** @brief Shows the current values of the settings. */
    virtual void refresh() = 0;

protected:
    UBToolOptionsPanel(const QString& title, QWidget* parent);

    void addRow(QWidget* widget);
    void addSection(const QString& title);

    QVBoxLayout* mLayout{nullptr};
};

class UBPenOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBPenOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    void updatePreview();

    UBStrokePreview* mPreview{nullptr};
    UBSegmentedChoice* mStrokeType{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBColorSwatches* mColors{nullptr};
    UBValueSlider* mOpacity{nullptr};
};

class UBEraserOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBEraserOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    UBSegmentedChoice* mMode{nullptr};
    UBValueSlider* mSize{nullptr};
    QToolButton* mEraseAll{nullptr};
};

class UBMarkerOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBMarkerOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    void updatePreview();

    UBStrokePreview* mPreview{nullptr};
    UBColorSwatches* mColors{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBValueSlider* mOpacity{nullptr};
};

class UBLaserOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBLaserOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    void updatePreview();

    UBStrokePreview* mPreview{nullptr};
    UBSegmentedChoice* mMode{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBColorSwatches* mColors{nullptr};
    UBSegmentedChoice* mEffect{nullptr};
    UBValueSlider* mOpacity{nullptr};
    UBValueSlider* mDelay{nullptr};
    UBValueSlider* mDuration{nullptr};
};

/** @brief Options of the rectangle, ellipse, arrow and underline tools. */
class UBShapeOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    UBShapeOptionsPanel(int tool, QWidget* parent = nullptr);
    void refresh() override;

private:
    void showColorTarget(int target);

    UBSettings::ShapeSettings* mSettings{nullptr};
    int mTool{0};
    UBSegmentedChoice* mColorTarget{nullptr};
    UBColorSwatches* mStrokeColors{nullptr};
    UBColorSwatches* mFillColors{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBPictureChoice* mDash{nullptr};
    UBPictureChoice* mBorderStyle{nullptr};
    UBPictureChoice* mStartEnding{nullptr};
    UBPictureChoice* mEndEnding{nullptr};
    UBValueSlider* mOpacity{nullptr};
};

class UBTextOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBTextOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    QFontComboBox* mFont{nullptr};
    UBValueSlider* mSize{nullptr};
    UBColorSwatches* mColors{nullptr};
    QToolButton* mBold{nullptr};
    QToolButton* mItalic{nullptr};
};
