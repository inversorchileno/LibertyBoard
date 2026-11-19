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

#include <QCheckBox>
#include <QFileDialog>
#include <QFontComboBox>
#include <QGridLayout>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBSetting.h"
#include "core/UBSettings.h"
#include "board/UBComments.h"
#include "domain/UBGraphicsCommentItem.h"
#include "domain/UBGraphicsNoteItem.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBStamps.h"
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

static QIcon rainbowIcon()
{
    QPixmap pixmap(72, 28);
    pixmap.fill(Qt::transparent);

    QLinearGradient gradient(0, 0, pixmap.width(), 0);

    for (int i = 0; i <= 6; ++i)
    {
        gradient.setColorAt(i / 6.0, QColor::fromHslF(i / 7.0, 0.95, 0.6));
    }

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QBrush(gradient), 8, Qt::SolidLine, Qt::RoundCap));
    QPainterPath wave;
    wave.moveTo(6, 18);
    wave.cubicTo(24, 2, 44, 28, 66, 10);
    painter.drawPath(wave);

    return QIcon(pixmap);
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
        case UBStylusTool::Note:
            return new UBNoteOptionsPanel(parent);
        case UBStylusTool::Stamp:
            return new UBStampOptionsPanel(parent);
        case UBStylusTool::Comment:
            return new UBCommentOptionsPanel(parent);
        case UBStylusTool::Selector:
            return new UBSelectorOptionsPanel(parent);
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

    for (UBColorSwatches* swatches : findChildren<UBColorSwatches*>())
    {
        swatches->setTwoRows();
    }

    // The swatches say what they are for: no "Color" title before them.
    for (QLabel* label : findChildren<QLabel*>())
    {
        if (label->objectName() == "section" && label->text() == tr("Color"))
        {
            label->hide();
        }
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

    layOutForToolbar();
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

// Read by the scene when a stroke of the pen ends.
static const QString sRecognizeShapesKey = QStringLiteral("Board/LibertyBoardRecognizeShapes");

UBPenOptionsPanel::UBPenOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Pen"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mPreview = new UBStrokePreview(this);
    addRow(mPreview);

    // The pens of Samsung Notes, in the order of UBPenStrokeType.
    mStrokeType = new UBSegmentedChoice({ tr("Ballpoint pen"), tr("Fountain pen"), tr("Calligraphy pen"),
                                          tr("Pencil"), tr("Marker") }, this);
    mStrokeType->setToolTip(tr("Ballpoint pen: even width. Fountain pen: width follows the pressure. "
                               "Calligraphy pen: width follows the direction. Pencil: thin and light, like graphite. "
                               "Marker: wide and even."));
    addRow(mStrokeType);

    mWidth = new UBValueSlider({ 1, 2, 3, 6, 10, 50 }, tr(" pt"), 1, this);
    addRow(mWidth);

    addSection(tr("Color"));
    mColors = new UBColorSwatches(penColors(), false, this);
    addRow(mColors);

    mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
    addRow(mOpacity);

    mRecognizeShapes = new QCheckBox(tr("Recognize shapes"), this);
    mRecognizeShapes->setToolTip(tr("A line, arrow, circle, oval, triangle, square or rectangle drawn by hand becomes a clean one."));
    addRow(mRecognizeShapes);

    connect(mRecognizeShapes, &QCheckBox::toggled, this, [settings](bool on) {
        settings->setValue(sRecognizeShapesKey, on);
    });

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
    mRecognizeShapes->setChecked(settings->value(sRecognizeShapesKey, false).toBool());
    updatePreview();
}

void UBPenOptionsPanel::layOutForToolbar()
{
    // Five pens: smaller tiles, so that they leave room in the toolbar.
    mStrokeType->setSmall();
}

void UBPenOptionsPanel::updatePreview()
{
    // The pens take the color they draw with.
    QList<QIcon> icons;

    for (int type = UBPenStrokeType::Basic; type <= UBPenStrokeType::Marker; ++type)
    {
        icons << penStyleIcon(type, mColors->currentColor());
    }

    mStrokeType->setIcons(icons);

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
    mMode->setIcons({ optionIcon(UBOptionIcon::EraserWhole), optionIcon(UBOptionIcon::EraserPartial) });
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

    mTip = new UBSegmentedChoice({ tr("Round tip"), tr("Straight tip") }, this);
    mTip->setIcons({ optionIcon(UBOptionIcon::MarkerRoundTip), optionIcon(UBOptionIcon::MarkerChiselTip) });
    mTip->setToolTip(tr("Round: the stroke has round ends. Straight: like a highlighter, wide across the lines of text, with straight ends."));
    addRow(mTip);

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
    connect(mTip, &UBSegmentedChoice::choiceChanged, this, [settings](int tip) {
        settings->markerChiselTip->set(tip == 1);
    });

    refresh();
}

void UBMarkerOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mColors->setCurrentColor(settings->currentMarkerColor());
    mWidth->setValue(settings->currentMarkerWidth());
    mOpacity->setValue(settings->boardMarkerAlpha->get().toDouble() * 100);
    mTip->setChoice(settings->markerChiselTip->get().toBool() ? 1 : 0);
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

    addSection(tr("Color"));
    mColors = new UBColorSwatches(laserColors(), false, this);
    addRow(mColors);

    // Apart from the colors: the trail takes all the colors of the rainbow.
    mRainbow = new QCheckBox(tr("Rainbow"), this);
    mRainbow->setIcon(rainbowIcon());
    mRainbow->setIconSize(QSize(36, 14));
    mRainbow->setToolTip(tr("The trail takes the colors of the rainbow, one after the other."));
    addRow(mRainbow);

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
        mRainbow->setChecked(false);
        updatePreview();
    });
    connect(mRainbow, &QCheckBox::toggled, this, [settings](bool on) {
        settings->laserRainbow->set(on);
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
    mRainbow->setChecked(settings->laserRainbow->get().toBool());
    mOpacity->setValue(settings->laserOpacity->get().toDouble() * 100);
    mDelay->setValue(settings->laserFadeDelay->get().toDouble());
    mDuration->setValue(settings->laserFadeDuration->get().toDouble());
    updatePreview();
}

void UBLaserOptionsPanel::layOutForToolbar()
{
    // The two times take half the width they would take, with short
    // captions; the whole caption is the tooltip.
    const QList<QPair<UBValueSlider*, QString>> times = { { mDelay, tr("Wait") }, { mDuration, tr("Fading") } };

    for (const auto& time : times)
    {
        if (auto caption = time.first->findChild<QLabel*>("section"))
        {
            time.first->setToolTip(caption->text());
            caption->setText(time.second);
        }

        time.first->setMaximumWidth(time.first->sizeHint().width() / 2 + 40);
    }
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

        mTapered = new QCheckBox(tr("Tapered"), this);
        mTapered->setToolTip(tr("The line grows from its start to the tip of the arrow."));
        addRow(mTapered);
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

    if (mTapered)
    {
        connect(mTapered, &QCheckBox::toggled, this, [settings](bool on) {
            settings->tapered->set(on);
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

    if (mTapered)
    {
        mTapered->setChecked(mSettings->tapered->get().toBool());
    }

    if (mOpacity)
    {
        mOpacity->setValue(mSettings->opacity->get().toDouble() * 100);
    }
}

void UBShapeOptionsPanel::layOutForToolbar()
{
    if (mTool != UBStylusTool::Arrow)
    {
        return;
    }

    // In the toolbar each choice of the line shows only the chosen picture
    // and drops the others down, with a short caption.
    for (UBPictureChoice* choice : { mDash, mStartEnding, mEndEnding })
    {
        choice->setDropDown();
    }

    const QMap<QString, QString> captions = { { tr("Line style"), tr("Line") }, { tr("Line start"), tr("Start") },
                                              { tr("Line end"), tr("End") } };

    for (QLabel* label : findChildren<QLabel*>("section"))
    {
        if (captions.contains(label->text()))
        {
            label->setToolTip(label->text());
            label->setText(captions.value(label->text()));
        }
        else if (label->text() == tr("Opacity"))
        {
            label->hide();
        }
    }

    // Tapered and the opacity at the left, before the colors.
    mLayout->removeWidget(mTapered);
    mLayout->removeWidget(mOpacity);
    mLayout->insertWidget(0, mTapered);
    mLayout->insertWidget(1, mOpacity);
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

// ---------------------------------------------------------------------------
// Notes

UBStampOptionsPanel::UBStampOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Stamp"), parent)
{
    // Remove, beside the title: a click on a stamp then takes it away.
    mRemove = new QToolButton(this);
    mRemove->setText(tr("Remove"));
    mRemove->setToolTip(tr("Click a stamp to take it away"));
    mRemove->setCheckable(true);
    mRemove->setStyleSheet("QToolButton { color: #f0f0f0; background: #3a404b; border: 1px solid #555c68; border-radius: 4px; padding: 3px 10px; }"
                           "QToolButton:checked { background: #c0392b; border-color: #e5533d; }");

    auto header = new QWidget(this);
    auto headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->removeWidget(mTitle);
    headerLayout->addWidget(mTitle);
    headerLayout->addStretch(1);
    headerLayout->addWidget(mRemove);
    mLayout->insertWidget(0, header);

    connect(mRemove, &QToolButton::toggled, this, [this](bool removing) {
        UBStamps::setRemoving(removing);

        if (removing)
        {
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Stamp);
        }

        refresh();
    });

    auto gallery = new QWidget(this);
    mGallery = new QGridLayout(gallery);
    mGallery->setContentsMargins(0, 0, 0, 0);
    mGallery->setSpacing(6);

    for (int i = 0; i < UBStamps::count(); ++i)
    {
        auto button = new QToolButton(gallery);
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIconSize(QSize(150, 50));
        button->setText(UBStamps::name(i));
        button->setToolTip(UBStamps::name(i));
        button->setIcon(iconFor(i));
        button->setStyleSheet("QToolButton { background: #ffffff; color: #222222; border: 1px solid #c9ced6; border-radius: 4px; padding: 4px; }"
                              "QToolButton:hover { border-color: #f6c343; }"
                              "QToolButton:checked { background: #fff4cf; border: 2px solid #f6c343; }");
        mGallery->addWidget(button, i / 3, i % 3);
        mStamps << button;

        connect(button, &QToolButton::clicked, this, [this, i]() {
            if (UBStamps::isCustom(i))
            {
                chooseCustomPicture();
            }
            else
            {
                choose(i);
            }
        });
    }

    addRow(gallery);
    refresh();
}

void UBStampOptionsPanel::refresh()
{
    const int current = UBStamps::current();
    const bool removing = UBStamps::isRemoving();

    {
        const QSignalBlocker blocker(mRemove);
        mRemove->setChecked(removing);
    }

    for (int i = 0; i < mStamps.size(); ++i)
    {
        // While removing, no stamp is the one to put.
        mStamps.at(i)->setAutoExclusive(!removing);
        mStamps.at(i)->setChecked(!removing && i == current);
        mStamps.at(i)->setAutoExclusive(true);
    }

    mStamps.last()->setIcon(iconFor(mStamps.size() - 1));
}

void UBStampOptionsPanel::layOutForToolbar()
{
    // One row of small pictures, their names as tooltips.
    for (int i = 0; i < mStamps.size(); ++i)
    {
        QToolButton* button = mStamps.at(i);
        mGallery->removeWidget(button);
        mGallery->addWidget(button, 0, i);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setIconSize(QSize(72, 24));
    }

    mGallery->setSpacing(3);
}

void UBStampOptionsPanel::choose(int index)
{
    UBStamps::setCurrent(index);
    UBStamps::setRemoving(false);

    // The stamp is put with the stamp tool, which the choice selects.
    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Stamp);
    refresh();
}

void UBStampOptionsPanel::chooseCustomPicture()
{
    const QString current = UBStamps::customFile();
    const QString file = QFileDialog::getOpenFileName(window(), tr("Picture of the custom stamp"),
        current.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) : current,
        tr("PNG pictures (*.png)"));

    if (!file.isEmpty())
    {
        UBStamps::setCustomFile(file);
    }

    if (UBStamps::customFile().isEmpty())
    {
        // Nothing to stamp with: the previous choice stays.
        refresh();
        return;
    }

    choose(UBStamps::count() - 1);
}

QIcon UBStampOptionsPanel::iconFor(int index) const
{
    QImage picture = UBStamps::image(index);

    if (picture.isNull())
    {
        // The custom stamp before a picture is chosen: a frame with a plus.
        picture = QImage(UBStamps::pageSize() * 2, QImage::Format_ARGB32_Premultiplied);
        picture.fill(Qt::transparent);
        QPainter painter(&picture);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#8a9099"), 6, Qt::DashLine));
        painter.drawRoundedRect(QRectF(picture.rect()).adjusted(6, 6, -6, -6), 20, 20);
        QFont font = painter.font();
        font.setPixelSize(picture.height() / 2);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(picture.rect(), Qt::AlignCenter, "+");
    }

    return QIcon(QPixmap::fromImage(picture));
}

UBSelectorOptionsPanel::UBSelectorOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Select"), parent)
{
    mSwitch = new QToolButton(this);
    mSwitch->setIconSize(QSize(26, 26));
    mSwitch->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mSwitch->setStyleSheet("QToolButton { color: #e8e8e8; background: #3a404b; border: 1px solid #4a5260; border-radius: 4px; padding: 3px 10px; }"
                           "QToolButton:hover { background: #4a5260; }");
    addRow(mSwitch);

    mHint = new QLabel(this);
    mHint->setStyleSheet("QLabel { color: #9aa3b2; }");
    addRow(mHint);

    connect(mSwitch, &QToolButton::clicked, this, []() {
        UBDrawingController* drawing = UBDrawingController::drawingController();
        drawing->setLassoSelection(!drawing->isLassoSelection());
    });
    connect(UBDrawingController::drawingController(), &UBDrawingController::lassoSelectionChanged,
            this, &UBSelectorOptionsPanel::refresh);

    refresh();
}

void UBSelectorOptionsPanel::refresh()
{
    // The button offers the other way of selecting.
    if (UBDrawingController::drawingController()->isLassoSelection())
    {
        mSwitch->setIcon(QIcon(":/images/stylusPalette/arrow.png"));
        mSwitch->setText(tr("Arrow"));
        mSwitch->setToolTip(tr("Select with a rectangle again"));
        mHint->setText(tr("Draw a line around what to select."));
    }
    else
    {
        mSwitch->setIcon(QIcon(":/images/toolbar/lasso.svg"));
        mSwitch->setText(tr("Lasso"));
        mSwitch->setToolTip(tr("Select by drawing a free line around the objects"));
        mHint->setText(tr("Drag a rectangle, or over a PDF to copy its text."));
    }
}

UBCommentOptionsPanel::UBCommentOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Quick note"), parent)
{
    auto buttons = new QWidget(this);
    auto row = new QHBoxLayout(buttons);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(4);

    auto addButton = [this, buttons, row](const QString& text, const QString& badge) {
        // The note icon with a small sign, like the comment buttons of PDF readers.
        QPixmap picture(QSize(64, 64));
        picture.fill(Qt::transparent);
        {
            QPainter painter(&picture);
            painter.setRenderHint(QPainter::Antialiasing);
            QIcon(":/images/marks/comment.svg").paint(&painter, QRect(4, 4, 48, 48));

            if (!badge.isEmpty())
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor("#3fa34d"));
                painter.drawEllipse(QRectF(32, 32, 30, 30));
                painter.setPen(Qt::white);
                QFont font = painter.font();
                font.setPixelSize(24);
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(QRectF(32, 30, 30, 30), Qt::AlignCenter, badge);
            }
        }

        auto button = new QToolButton(buttons);
        button->setIcon(QIcon(picture));
        button->setIconSize(QSize(30, 30));
        button->setText(text);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setStyleSheet("QToolButton { color: #e8e8e8; background: transparent; border: none; padding: 2px 6px; }"
                              "QToolButton:hover { background: #3a404b; border-radius: 4px; }"
                              "QToolButton:checked { background: #4a5260; border-radius: 4px; }");
        row->addWidget(button);
        Q_UNUSED(this)
        return button;
    };

    auto previous = addButton(tr("Previous"), QStringLiteral("\u2039"));
    auto next = addButton(tr("Next"), QStringLiteral("\u203A"));
    auto list = addButton(tr("List of notes"), QString());
    mShow = addButton(tr("Show"), QStringLiteral("\u25C9"));
    mShow->setCheckable(true);
    mShow->setToolTip(tr("Show or hide the quick notes of the pages"));
    auto merge = addButton(tr("Merge notes"), QStringLiteral("\u2193"));
    merge->setToolTip(tr("Put the quick notes into the pages, as pictures"));

    addRow(buttons);

    connect(previous, &QToolButton::clicked, this, [this]() { UBComments::step(-1, window()); });
    connect(next, &QToolButton::clicked, this, [this]() { UBComments::step(+1, window()); });
    connect(list, &QToolButton::clicked, this, [this]() { UBComments::showList(window()); });
    connect(mShow, &QToolButton::toggled, this, [](bool shown) { UBComments::setShown(shown); });
    connect(merge, &QToolButton::clicked, this, [this]() { UBComments::merge(window()); });

    refresh();
}

void UBCommentOptionsPanel::refresh()
{
    const QSignalBlocker blocker(mShow);
    mShow->setChecked(UBGraphicsCommentItem::areShown());
}

UBNoteOptionsPanel::UBNoteOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Note"), parent)
{
    addSection(tr("Marker color"));
    mColors = new UBColorSwatches(UBGraphicsNoteItem::colors(), false, this);
    mColors->hideOtherColorButton();
    addRow(mColors);

    connect(mColors, &UBColorSwatches::colorChosen, this, [](const QColor& color) {
        UBSettings::settings()->noteColor->set(color.name());
    });

    refresh();
}

void UBNoteOptionsPanel::refresh()
{
    mColors->setCurrentColor(QColor(UBSettings::settings()->noteColor->get().toString()));
}
