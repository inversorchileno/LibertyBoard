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

#include "UBToolOptionsPanel.h"

#include <QFontComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBSetting.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsTextItem.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolOptionsWidgets.h"

// Width of the eraser on screen for each point of its size
static constexpr qreal sEraserPixelsPerPoint = 3.0;

static QList<QColor> penColors()
{
    return { QColor("#ff3b30"), QColor("#2f54ff"), QColor("#000000"), QColor("#14b85a"), QColor("#9a46f0"),
             QColor("#ff9a1f"), QColor("#12b5e8"), QColor("#ff1fd2"), QColor("#ffee1f") };
}

static QList<QColor> markerColors()
{
    return { QColor("#ffff00"), QColor("#ff00ff"), QColor("#00ffff"), QColor("#92d050"), QColor("#ff7c80"),
             QColor("#ffc000"), QColor("#9a46f0"), QColor("#ffa07a"), QColor("#ff4f6d") };
}

static QList<QColor> laserColors()
{
    return { QColor("#ff2828"), QColor("#ff9a1f"), QColor("#ff1fd2"), QColor("#2bff2b"), QColor("#2f54ff"),
             QColor("#9a46f0"), QColor("#000000"), QColor("#14b85a"), QColor("#ffee1f") };
}

static QColor settingColor(UBSetting* setting)
{
    return QColor(setting->get().toString());
}

static QString colorName(const QColor& color)
{
    return color.isValid() ? color.name() : QString();
}

static QList<qreal> opacityMarks()
{
    return { 0, 25, 50, 75, 100 };
}

UBToolOptionsPanel* UBToolOptionsPanel::create(int tool, QWidget* parent)
{
    switch (tool)
    {
        case UBStylusTool::Pen:
        case UBStylusTool::Line:
            return new UBPenOptionsPanel(parent);
        case UBStylusTool::Eraser:
            return new UBEraserOptionsPanel(parent);
        case UBStylusTool::Marker:
            return new UBMarkerOptionsPanel(parent);
        case UBStylusTool::Pointer:
            return new UBLaserOptionsPanel(parent);
        case UBStylusTool::Rectangle:
        case UBStylusTool::Ellipse:
        case UBStylusTool::Arrow:
        case UBStylusTool::Underline:
        case UBStylusTool::Highlight:
            return new UBShapeOptionsPanel(tool, parent);
        case UBStylusTool::Text:
            return new UBTextOptionsPanel(parent);
        default:
            return nullptr;
    }
}

UBToolOptionsPanel::UBToolOptionsPanel(const QString& title, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet(toolOptionsStyleSheet());
    setMinimumWidth(320);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(14, 12, 14, 14);
    mLayout->setSpacing(8);

    mTitle = new QLabel(title, this);
    mTitle->setObjectName("title");
    mLayout->addWidget(mTitle);
}

void UBToolOptionsPanel::setHorizontal()
{
    mLayout->setDirection(QBoxLayout::LeftToRight);
    mLayout->setContentsMargins(8, 2, 8, 2);
    mLayout->setSpacing(10);
    setMinimumWidth(0);
    setStyleSheet(toolOptionsStyleSheet() + "UBToolOptionsPanel { background: transparent; }");

    // The toolbar shows which tool is selected: no title and no preview.
    mTitle->hide();

    for (UBStrokePreview* preview : findChildren<UBStrokePreview*>())
    {
        preview->hide();
    }

    for (UBSegmentedChoice* choice : findChildren<UBSegmentedChoice*>())
    {
        choice->setCompact();
    }

    for (UBValueSlider* slider : findChildren<UBValueSlider*>())
    {
        slider->setCompact();
    }

    if (stacksSlidersInToolbar())
    {
        // A section title just before a slider becomes its caption.
        QLabel* section = nullptr;

        for (int i = 0; i < mLayout->count(); ++i)
        {
            QWidget* widget = mLayout->itemAt(i)->widget();

            if (auto slider = qobject_cast<UBValueSlider*>(widget))
            {
                slider->setStacked(section ? section->text() : QString());

                if (section)
                {
                    section->hide();
                }
            }

            auto label = qobject_cast<QLabel*>(widget);
            section = (label && label->objectName() == "section") ? label : nullptr;
        }
    }

    mLayout->addStretch();
}

void UBToolOptionsPanel::addRow(QWidget* widget)
{
    mLayout->addWidget(widget);
}

void UBToolOptionsPanel::addSection(const QString& title)
{
    auto label = new QLabel(title, this);
    label->setObjectName("section");
    mLayout->addSpacing(4);
    mLayout->addWidget(label);
}

// ---------------------------------------------------------------------------
// Pen

UBPenOptionsPanel::UBPenOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Pen"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mPreview = new UBStrokePreview(this);
    addRow(mPreview);

    mStrokeType = new UBSegmentedChoice({ tr("Basic\nFixed width"), tr("Pro\nVariable width"), tr("Calligraphy\nDynamic width") }, this);
    mStrokeType->setIcons({ optionIcon(UBOptionIcon::BasicStroke), optionIcon(UBOptionIcon::ProStroke),
                            optionIcon(UBOptionIcon::CalligraphyStroke) });
    addRow(mStrokeType);

    mWidth = new UBValueSlider({ 1, 2, 3, 6, 10, 50 }, tr(" pt"), 1, this);
    addRow(mWidth);

    addSection(tr("Appearance"));
    mColors = new UBColorSwatches(penColors(), false, this);
    addRow(mColors);

    mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
    addRow(mOpacity);

    connect(mStrokeType, &UBSegmentedChoice::choiceChanged, this, [this, settings](int type) {
        settings->penStrokeType->set(type);
        updatePreview();
    });
    connect(mWidth, &UBValueSlider::valueChanged, this, [this, settings](qreal width) {
        settings->setCurrentPenWidth(width);
        updatePreview();
    });
    connect(mColors, &UBColorSwatches::colorChosen, this, [this, settings](const QColor& color) {
        const int index = settings->penColorIndex();
        UBDrawingController::drawingController()->setPenColor(false, color, index);
        UBDrawingController::drawingController()->setPenColor(true, color, index);
        updatePreview();
    });
    connect(mOpacity, &UBValueSlider::valueChanged, this, [this, settings](qreal opacity) {
        settings->penOpacity->set(opacity / 100);
        updatePreview();
    });

    refresh();
}

void UBPenOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mStrokeType->setChoice(settings->penStrokeType->get().toInt());
    mWidth->setValue(settings->currentPenWidth());
    mColors->setCurrentColor(settings->currentPenColor());
    mOpacity->setValue(settings->penOpacity->get().toDouble() * 100);
    updatePreview();
}

void UBPenOptionsPanel::updatePreview()
{
    mPreview->setStrokeType(mStrokeType->choice());
    mPreview->setStroke(mColors->currentColor(), mWidth->value(), mOpacity->value() / 100, false);
}

// ---------------------------------------------------------------------------
// Eraser

UBEraserOptionsPanel::UBEraserOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Eraser"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mMode = new UBSegmentedChoice({ tr("Whole"), tr("Partial") }, this);
    mMode->setToolTip(tr("Whole: erases every stroke the eraser touches. Partial: erases only where the eraser passes."));
    addRow(mMode);

    mSize = new UBValueSlider({ 1, 2, 3, 6, 10, 50 }, tr(" pt"), 0, this);
    addRow(mSize);

    mEraseAll = new QToolButton(this);
    mEraseAll->setText(tr("Erase all annotations"));
    mEraseAll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addRow(mEraseAll);

    // Whole is shown first, like in Drawboard PDF, but is stored as its own value.
    connect(mMode, &UBSegmentedChoice::choiceChanged, this, [settings](int choice) {
        settings->eraserMode->set(choice == 0 ? int(UBEraserMode::Whole) : int(UBEraserMode::Partial));
    });
    connect(mSize, &UBValueSlider::valueChanged, this, [settings](qreal size) {
        settings->setCurrentEraserWidth(size * sEraserPixelsPerPoint);
    });
    connect(mEraseAll, &QToolButton::clicked, this, []() {
        if (UBDrawingController::drawingController()->isInDesktopMode())
        {
            UBApplication::mainWindow->actionEraseDesktopAnnotations->trigger();
        }
        else
        {
            UBApplication::mainWindow->actionEraseAnnotations->trigger();
        }
    });

    refresh();
}

void UBEraserOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mMode->setChoice(settings->eraserMode->get().toInt() == UBEraserMode::Whole ? 0 : 1);
    mSize->setValue(settings->currentEraserWidth() / sEraserPixelsPerPoint);
}

// ---------------------------------------------------------------------------
// Marker

UBMarkerOptionsPanel::UBMarkerOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Marker"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mPreview = new UBStrokePreview(this);
    addRow(mPreview);

    mColors = new UBColorSwatches(markerColors(), false, this);
    addRow(mColors);

    mWidth = new UBValueSlider({ 20, 30, 40, 50, 80, 100 }, tr(" pt"), 0, this);
    addRow(mWidth);

    addSection(tr("Advanced"));
    mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
    addRow(mOpacity);

    connect(mColors, &UBColorSwatches::colorChosen, this, [this, settings](QColor color) {
        color.setAlphaF(settings->boardMarkerAlpha->get().toDouble());
        const int index = settings->markerColorIndex();
        UBDrawingController::drawingController()->setMarkerColor(false, color, index);
        UBDrawingController::drawingController()->setMarkerColor(true, color, index);
        updatePreview();
    });
    connect(mWidth, &UBValueSlider::valueChanged, this, [this, settings](qreal width) {
        settings->setCurrentMarkerWidth(width);
        updatePreview();
    });
    connect(mOpacity, &UBValueSlider::valueChanged, this, [this](qreal opacity) {
        UBDrawingController::drawingController()->setMarkerAlpha(opacity / 100);
        updatePreview();
    });

    refresh();
}

void UBMarkerOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mColors->setCurrentColor(settings->currentMarkerColor());
    mWidth->setValue(settings->currentMarkerWidth());
    mOpacity->setValue(settings->boardMarkerAlpha->get().toDouble() * 100);
    updatePreview();
}

void UBMarkerOptionsPanel::updatePreview()
{
    // Shown thinner than drawn so that the preview stays readable.
    mPreview->setStroke(mColors->currentColor(), mWidth->value() / 4, mOpacity->value() / 100, false);
}

// ---------------------------------------------------------------------------
// Laser pointer

UBLaserOptionsPanel::UBLaserOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Laser pointer"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mPreview = new UBStrokePreview(this);
    addRow(mPreview);

    mMode = new UBSegmentedChoice({ tr("Trail"), tr("Point") }, this);
    mMode->setIcons({ optionIcon(UBOptionIcon::LaserTrail), optionIcon(UBOptionIcon::LaserPoint) });
    addRow(mMode);

    mWidth = new UBValueSlider({ 0.5, 1, 2, 4, 8, 16, 64 }, tr(" pt"), 1, this);
    addRow(mWidth);

    addSection(tr("Appearance"));
    mColors = new UBColorSwatches(laserColors(), false, this);
    addRow(mColors);

    mEffect = new UBSegmentedChoice({ tr("Standard"), tr("White burn") }, this);
    mEffect->setIcons({ optionIcon(UBOptionIcon::LaserStandard), optionIcon(UBOptionIcon::LaserWhiteBurn) });
    addRow(mEffect);

    mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
    addRow(mOpacity);

    addSection(tr("Wait before fading"));
    mDelay = new UBValueSlider({ 0, 0.1, 0.5, 1, 2, 5 }, tr(" s"), 1, this);
    addRow(mDelay);

    addSection(tr("Fading duration"));
    mDuration = new UBValueSlider({ 0.1, 0.5, 1, 2, 3, 5, 10 }, tr(" s"), 1, this);
    addRow(mDuration);

    connect(mMode, &UBSegmentedChoice::choiceChanged, this, [settings](int mode) {
        settings->laserMode->set(mode);
    });
    connect(mWidth, &UBValueSlider::valueChanged, this, [this, settings](qreal width) {
        settings->laserWidth->set(width);
        updatePreview();
    });
    connect(mColors, &UBColorSwatches::colorChosen, this, [this, settings](const QColor& color) {
        settings->laserColor->set(color.name());
        updatePreview();
    });
    connect(mEffect, &UBSegmentedChoice::choiceChanged, this, [settings](int effect) {
        settings->laserEffect->set(effect);
    });
    connect(mOpacity, &UBValueSlider::valueChanged, this, [this, settings](qreal opacity) {
        settings->laserOpacity->set(opacity / 100);
        updatePreview();
    });
    connect(mDelay, &UBValueSlider::valueChanged, this, [settings](qreal delay) {
        settings->laserFadeDelay->set(delay);
    });
    connect(mDuration, &UBValueSlider::valueChanged, this, [settings](qreal duration) {
        settings->laserFadeDuration->set(duration);
    });

    refresh();
}

void UBLaserOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mMode->setChoice(settings->laserMode->get().toInt());
    mWidth->setValue(settings->laserWidth->get().toDouble());
    mColors->setCurrentColor(settingColor(settings->laserColor));
    mEffect->setChoice(settings->laserEffect->get().toInt());
    mOpacity->setValue(settings->laserOpacity->get().toDouble() * 100);
    mDelay->setValue(settings->laserFadeDelay->get().toDouble());
    mDuration->setValue(settings->laserFadeDuration->get().toDouble());
    updatePreview();
}

void UBLaserOptionsPanel::updatePreview()
{
    mPreview->setStroke(mColors->currentColor(), mWidth->value(), mOpacity->value() / 100, true);
}

// ---------------------------------------------------------------------------
// Shapes

static QString shapeTitle(int tool)
{
    switch (tool)
    {
        case UBStylusTool::Rectangle:
            return UBShapeOptionsPanel::tr("Rectangle");
        case UBStylusTool::Ellipse:
            return UBShapeOptionsPanel::tr("Ellipse");
        case UBStylusTool::Arrow:
            return UBShapeOptionsPanel::tr("Arrow");
        case UBStylusTool::Highlight:
            return UBShapeOptionsPanel::tr("Text highlight");
        default:
            return UBShapeOptionsPanel::tr("Underline");
    }
}

UBShapeOptionsPanel::UBShapeOptionsPanel(int tool, QWidget* parent)
    : UBToolOptionsPanel(shapeTitle(tool), parent)
    , mSettings(UBSettings::settings()->shapeSettings(tool))
    , mTool(tool)
{
    const bool closedShape = (tool == UBStylusTool::Rectangle || tool == UBStylusTool::Ellipse);
    const bool isArrow = (tool == UBStylusTool::Arrow);
    const bool isUnderline = (tool == UBStylusTool::Underline);
    const bool isHighlight = (tool == UBStylusTool::Highlight);

    if (closedShape)
    {
        mColorTarget = new UBSegmentedChoice({ tr("Border"), tr("Fill") }, this);
        mColorTarget->setIcons({ optionIcon(UBOptionIcon::Border), optionIcon(UBOptionIcon::Fill) }, Qt::ToolButtonTextBesideIcon);
        addRow(mColorTarget);
    }

    mStrokeColors = new UBColorSwatches(isHighlight ? markerColors() : penColors(), false, this);
    addRow(mStrokeColors);

    if (closedShape)
    {
        mFillColors = new UBColorSwatches(penColors(), true, this);
        addRow(mFillColors);
    }

    if (isHighlight)
    {
        addSection(tr("Height"));
        mWidth = new UBValueSlider({ 8, 12, 16, 20, 30, 40, 60 }, tr(" pt"), 0, this);
    }
    else
    {
        mWidth = new UBValueSlider({ 1, 2, 3, 6, 10, 50 }, tr(" pt"), 1, this);
    }

    addRow(mWidth);

    if (!isUnderline && !isHighlight)
    {
        addSection(isArrow ? tr("Line style") : tr("Border styles"));
        mDash = new UBPictureChoice(UBPictureChoice::Kind::Dash, 5, this);
        addRow(mDash);
    }

    if (closedShape)
    {
        mBorderStyle = new UBPictureChoice(UBPictureChoice::Kind::BorderStyle, 2, this);
        addRow(mBorderStyle);
    }

    if (isArrow)
    {
        addSection(tr("Line start"));
        mStartEnding = new UBPictureChoice(UBPictureChoice::Kind::StartEnding, 6, this);
        addRow(mStartEnding);

        addSection(tr("Line end"));
        mEndEnding = new UBPictureChoice(UBPictureChoice::Kind::EndEnding, 6, this);
        addRow(mEndEnding);
    }

    if (!isUnderline)
    {
        addSection(tr("Opacity"));
        mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
        addRow(mOpacity);
    }

    UBSettings::ShapeSettings* settings = mSettings;

    if (mColorTarget)
    {
        connect(mColorTarget, &UBSegmentedChoice::choiceChanged, this, &UBShapeOptionsPanel::showColorTarget);
    }

    connect(mStrokeColors, &UBColorSwatches::colorChosen, this, [settings](const QColor& color) {
        settings->strokeColor->set(colorName(color));
    });

    if (mFillColors)
    {
        connect(mFillColors, &UBColorSwatches::colorChosen, this, [settings](const QColor& color) {
            settings->fillColor->set(colorName(color));
        });
    }

    connect(mWidth, &UBValueSlider::valueChanged, this, [settings](qreal width) {
        settings->width->set(width);
    });

    if (mDash)
    {
        connect(mDash, &UBPictureChoice::choiceChanged, this, [settings](int dash) {
            settings->dash->set(dash);
        });
    }

    if (mBorderStyle)
    {
        connect(mBorderStyle, &UBPictureChoice::choiceChanged, this, [settings](int style) {
            settings->borderStyle->set(style);
        });
    }

    if (mStartEnding)
    {
        connect(mStartEnding, &UBPictureChoice::choiceChanged, this, [settings](int ending) {
            settings->startEnding->set(ending);
        });
        connect(mEndEnding, &UBPictureChoice::choiceChanged, this, [settings](int ending) {
            settings->endEnding->set(ending);
        });
    }

    if (mOpacity)
    {
        connect(mOpacity, &UBValueSlider::valueChanged, this, [settings](qreal opacity) {
            settings->opacity->set(opacity / 100);
        });
    }

    refresh();
    showColorTarget(0);
}

void UBShapeOptionsPanel::refresh()
{
    mStrokeColors->setCurrentColor(settingColor(mSettings->strokeColor));

    if (mFillColors)
    {
        mFillColors->setCurrentColor(settingColor(mSettings->fillColor));
    }

    mWidth->setValue(mSettings->width->get().toDouble());

    if (mDash)
    {
        mDash->setChoice(mSettings->dash->get().toInt());
    }

    if (mBorderStyle)
    {
        mBorderStyle->setChoice(mSettings->borderStyle->get().toInt());
    }

    if (mStartEnding)
    {
        mStartEnding->setChoice(mSettings->startEnding->get().toInt());
        mEndEnding->setChoice(mSettings->endEnding->get().toInt());
    }

    if (mOpacity)
    {
        mOpacity->setValue(mSettings->opacity->get().toDouble() * 100);
    }
}

void UBShapeOptionsPanel::showColorTarget(int target)
{
    if (!mColorTarget)
    {
        return;
    }

    mColorTarget->setChoice(target);
    mStrokeColors->setVisible(target == 0);
    mFillColors->setVisible(target == 1);
}

// ---------------------------------------------------------------------------
// Text

UBTextOptionsPanel::UBTextOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Text"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mFont = new QFontComboBox(this);
    addRow(mFont);

    mSize = new UBValueSlider({ 8, 12, 16, 24, 36, 48, 72 }, tr(" pt"), 0, this);
    addRow(mSize);

    auto styleRow = new QWidget(this);
    auto styleLayout = new QHBoxLayout(styleRow);
    styleLayout->setContentsMargins(0, 0, 0, 0);

    mBold = new QToolButton(styleRow);
    mBold->setText(tr("B"));
    mBold->setToolTip(tr("Bold"));
    mBold->setCheckable(true);
    QFont boldFont = mBold->font();
    boldFont.setBold(true);
    mBold->setFont(boldFont);

    mItalic = new QToolButton(styleRow);
    mItalic->setText(tr("I"));
    mItalic->setToolTip(tr("Italic"));
    mItalic->setCheckable(true);
    QFont italicFont = mItalic->font();
    italicFont.setItalic(true);
    mItalic->setFont(italicFont);

    styleLayout->addWidget(mBold);
    styleLayout->addWidget(mItalic);
    styleLayout->addStretch();
    addRow(styleRow);

    addSection(tr("Color"));
    mColors = new UBColorSwatches(penColors(), false, this);
    addRow(mColors);

    connect(mFont, &QFontComboBox::currentFontChanged, this, [settings](const QFont& font) {
        settings->setFontFamily(font.family());
    });
    connect(mSize, &UBValueSlider::valueChanged, this, [settings](qreal size) {
        settings->setFontPointSize(qRound(size));
    });
    connect(mBold, &QToolButton::toggled, this, [settings](bool bold) {
        settings->setBoldFont(bold);
    });
    connect(mItalic, &QToolButton::toggled, this, [settings](bool italic) {
        settings->setItalicFont(italic);
    });
    connect(mColors, &UBColorSwatches::colorChosen, this, [](const QColor& color) {
        UBGraphicsTextItem::lastUsedTextColor = color;
    });

    refresh();
}

void UBTextOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    const QSignalBlocker fontBlocker(mFont);
    const QSignalBlocker boldBlocker(mBold);
    const QSignalBlocker italicBlocker(mItalic);

    mFont->setCurrentFont(QFont(settings->fontFamily()));
    mSize->setValue(settings->fontPointSize());
    mBold->setChecked(settings->isBoldFont());
    mItalic->setChecked(settings->isItalicFont());
    mColors->setCurrentColor(UBGraphicsTextItem::lastUsedTextColor);
}
