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

#include "UBToolOptionsWidgets.h"

#include <cmath>

#include <QButtonGroup>
#include <QColorDialog>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QSlider>
#include <QToolButton>
#include <QtMath>

#include "core/UB.h"

static constexpr int sSwatchSize = 24;
static constexpr int sSliderSteps = 1000;

QString toolOptionsStyleSheet()
{
    return QStringLiteral(
        "UBToolOptionsPanel { background: #20242b; }"
        "QLabel { color: #e8e8e8; background: transparent; }"
        "QLabel#title { font-weight: bold; font-size: 15px; }"
        "QLabel#section { color: #9aa3b2; }"
        "QToolButton { background: #2c313a; color: #e8e8e8; border: 1px solid #3a404b; border-radius: 6px; padding: 4px; }"
        "QToolButton:hover { border-color: #6b7383; }"
        "QToolButton:checked { background: #3d4452; border: 2px solid #f0f0f0; }"
        "QToolButton#swatch { border-radius: 4px; padding: 0px; }"
        "QToolButton#swatch:checked { border: 2px solid #f0f0f0; }"
        "QComboBox { background: #111317; color: #f0f0f0; border: 1px solid #3a404b; border-radius: 4px; padding: 2px 6px; min-width: 64px; }"
        "QComboBox QAbstractItemView { background: #111317; color: #f0f0f0; selection-background-color: #3d4452; }"
        "QSlider::groove:horizontal { height: 4px; background: #4a5160; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: #f0f0f0; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #ffffff; width: 16px; height: 16px; margin: -6px 0; border-radius: 8px; }"
        "QCheckBox { color: #e8e8e8; }"
        "QFontComboBox { min-width: 140px; }");
}

static QString swatchStyle(const QColor& color)
{
    if (!color.isValid())
    {
        // "None" swatch: a white square crossed by a red line.
        return QStringLiteral("QToolButton#swatch { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, "
                              "stop:0 #f0f0f0, stop:0.46 #f0f0f0, stop:0.5 #e02020, stop:0.54 #f0f0f0, stop:1 #f0f0f0); }");
    }

    return QStringLiteral("QToolButton#swatch { background: %1; }").arg(color.name());
}

UBColorSwatches::UBColorSwatches(const QList<QColor>& colors, bool offerNone, QWidget* parent)
    : QWidget(parent)
    , mColors(colors)
{
    if (offerNone)
    {
        mColors.prepend(QColor());
    }

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    for (const QColor& color : std::as_const(mColors))
    {
        auto swatch = new QToolButton(this);
        swatch->setObjectName("swatch");
        swatch->setCheckable(true);
        swatch->setFixedSize(sSwatchSize, sSwatchSize);
        swatch->setStyleSheet(swatchStyle(color));
        swatch->setToolTip(color.isValid() ? color.name() : tr("No color"));
        layout->addWidget(swatch);
        mSwatches << swatch;

        connect(swatch, &QToolButton::clicked, this, [this, color]() {
            selectSwatch(color);
            emit colorChosen(color);
        });
    }

    mOtherColorButton = new QToolButton(this);
    mOtherColorButton->setText(QStringLiteral("…"));
    mOtherColorButton->setToolTip(tr("Other color"));
    mOtherColorButton->setFixedSize(sSwatchSize + 6, sSwatchSize);
    layout->addWidget(mOtherColorButton);
    layout->addStretch();

    connect(mOtherColorButton, &QToolButton::clicked, this, &UBColorSwatches::chooseOtherColor);
}

void UBColorSwatches::setCurrentColor(const QColor& color)
{
    selectSwatch(color);
}

QColor UBColorSwatches::currentColor() const
{
    return mCurrentColor;
}

void UBColorSwatches::selectSwatch(const QColor& color)
{
    mCurrentColor = color;

    QColor opaque = color;

    if (opaque.isValid())
    {
        opaque.setAlpha(255);
    }

    for (int i = 0; i < mSwatches.size(); ++i)
    {
        const QColor& swatchColor = mColors.at(i);
        const bool same = (!swatchColor.isValid() && !opaque.isValid())
                || (swatchColor.isValid() && opaque.isValid() && swatchColor.rgb() == opaque.rgb());
        mSwatches.at(i)->setChecked(same);
    }
}

void UBColorSwatches::chooseOtherColor()
{
    const QColor initial = mCurrentColor.isValid() ? mCurrentColor : QColor(Qt::black);

    // A popup keeps the mouse for itself: it must be closed first, or the
    // dialog cannot be clicked.
    QWidget* topLevel = window();

    if (topLevel && (topLevel->windowFlags() & Qt::Popup) == Qt::Popup)
    {
        topLevel->hide();
    }

    // Kept above the desktop annotation layer, which stays on top of all windows.
    QColorDialog dialog(initial);
    dialog.setWindowTitle(tr("Choose a color"));
    dialog.setWindowFlag(Qt::WindowStaysOnTopHint);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const QColor color = dialog.selectedColor();

    if (color.isValid())
    {
        selectSwatch(color);
        emit colorChosen(color);
    }
}

UBValueSlider::UBValueSlider(const QList<qreal>& marks, const QString& unit, int decimals, QWidget* parent)
    : QWidget(parent)
    , mMarks(marks)
    , mUnit(unit)
    , mDecimals(decimals)
{
    mSlider = new QSlider(Qt::Horizontal, this);
    mSlider->setRange(0, sSliderSteps);
    mSlider->setMinimumWidth(170);

    mValueBox = new QComboBox(this);
    mValueBox->setEditable(true);
    mValueBox->setInsertPolicy(QComboBox::NoInsert);

    for (qreal mark : std::as_const(mMarks))
    {
        mValueBox->addItem(QString::number(mark, 'f', mDecimals) + mUnit, mark);
    }

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mSlider, 1);
    layout->addWidget(mValueBox);

    connect(mSlider, &QSlider::valueChanged, this, &UBValueSlider::onSliderMoved);
    connect(mValueBox, qOverload<int>(&QComboBox::activated), this, [this](int index) {
        setValue(mValueBox->itemData(index).toDouble());
        emit valueChanged(mValue);
    });
    connect(mValueBox->lineEdit(), &QLineEdit::editingFinished, this, &UBValueSlider::onTextEdited);
}

void UBValueSlider::setValue(qreal value)
{
    mValue = qBound(mMarks.first(), value, mMarks.last());

    const QSignalBlocker blocker(mSlider);
    mSlider->setValue(positionOf(mValue));
    showValue(mValue);
}

qreal UBValueSlider::value() const
{
    return mValue;
}

void UBValueSlider::setCompact()
{
    mSlider->setMinimumWidth(90);
    mSlider->setMaximumWidth(140);
}

int UBValueSlider::positionOf(qreal value) const
{
    const int intervals = mMarks.size() - 1;

    for (int i = 0; i < intervals; ++i)
    {
        if (value <= mMarks.at(i + 1))
        {
            const qreal ratio = (value - mMarks.at(i)) / (mMarks.at(i + 1) - mMarks.at(i));
            return qRound((i + ratio) * sSliderSteps / intervals);
        }
    }

    return sSliderSteps;
}

qreal UBValueSlider::valueAt(int position) const
{
    const int intervals = mMarks.size() - 1;
    const qreal scaled = qreal(position) * intervals / sSliderSteps;
    const int i = qMin(int(scaled), intervals - 1);
    const qreal ratio = scaled - i;

    return mMarks.at(i) + ratio * (mMarks.at(i + 1) - mMarks.at(i));
}

void UBValueSlider::onSliderMoved(int position)
{
    const qreal factor = std::pow(10.0, mDecimals);
    mValue = std::round(valueAt(position) * factor) / factor;
    showValue(mValue);
    emit valueChanged(mValue);
}

void UBValueSlider::onTextEdited()
{
    QString text = mValueBox->currentText();
    text.remove(mUnit.trimmed());
    text.replace(',', '.');

    bool ok = false;
    const qreal value = text.trimmed().toDouble(&ok);

    if (ok)
    {
        setValue(value);
        emit valueChanged(mValue);
    }
    else
    {
        showValue(mValue);
    }
}

void UBValueSlider::showValue(qreal value)
{
    const QSignalBlocker blocker(mValueBox);
    mValueBox->setCurrentIndex(-1);
    mValueBox->setEditText(QString::number(value, 'f', mDecimals) + mUnit);
}

UBSegmentedChoice::UBSegmentedChoice(const QStringList& labels, QWidget* parent)
    : QWidget(parent)
{
    mGroup = new QButtonGroup(this);
    mGroup->setExclusive(true);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    for (int i = 0; i < labels.size(); ++i)
    {
        auto button = new QToolButton(this);
        button->setText(labels.at(i));
        button->setCheckable(true);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mGroup->addButton(button, i);
        layout->addWidget(button);
    }

    connect(mGroup, &QButtonGroup::idClicked, this, &UBSegmentedChoice::choiceChanged);
}

void UBSegmentedChoice::setChoice(int index)
{
    if (auto button = mGroup->button(index))
    {
        button->setChecked(true);
    }
}

int UBSegmentedChoice::choice() const
{
    return mGroup->checkedId();
}

void UBSegmentedChoice::setCompact()
{
    for (QAbstractButton* button : mGroup->buttons())
    {
        const QString label = button->text();
        button->setToolTip(QString(label).replace(QChar('\n'), QStringLiteral(" - ")));
        button->setText(label.section('\n', 0, 0));
        button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
}

static void drawEnding(QPainter& painter, int ending, const QPointF& tip, qreal direction)
{
    // direction: angle in radians the line comes from, the ending points away from it
    const qreal size = 6;
    const QPointF along(std::cos(direction), std::sin(direction));
    const QPointF across(-along.y(), along.x());

    switch (ending)
    {
        case UBLineEnding::OpenArrow:
        {
            QPainterPath path;
            path.moveTo(tip - along * size + across * size * 0.7);
            path.lineTo(tip);
            path.lineTo(tip - along * size - across * size * 0.7);
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
            break;
        }
        case UBLineEnding::ClosedArrow:
        {
            QPolygonF triangle;
            triangle << tip << tip - along * size + across * size * 0.7 << tip - along * size - across * size * 0.7;
            painter.setBrush(painter.pen().color());
            painter.drawPolygon(triangle);
            break;
        }
        case UBLineEnding::Circle:
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(tip - along * size * 0.5, size * 0.5, size * 0.5);
            break;
        case UBLineEnding::Square:
        {
            const QPointF center = tip - along * size * 0.5;
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRectF(center.x() - size * 0.45, center.y() - size * 0.45, size * 0.9, size * 0.9));
            break;
        }
        case UBLineEnding::Diamond:
        {
            const QPointF center = tip - along * size * 0.5;
            QPolygonF diamond;
            diamond << center + along * size * 0.6 << center + across * size * 0.6
                    << center - along * size * 0.6 << center - across * size * 0.6;
            painter.setBrush(Qt::NoBrush);
            painter.drawPolygon(diamond);
            break;
        }
        default:
            break;
    }
}

static QIcon pictureIcon(UBPictureChoice::Kind kind, int index)
{
    const QSize size(40, 24);
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(QColor("#e8e8e8"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    switch (kind)
    {
        case UBPictureChoice::Kind::Dash:
        {
            const QList<Qt::PenStyle> styles = { Qt::SolidLine, Qt::DotLine, Qt::DashLine, Qt::DashLine, Qt::DashDotLine };
            pen.setStyle(styles.value(index, Qt::SolidLine));
            pen.setCapStyle(Qt::FlatCap);

            if (index == UBLineDash::LongDash)
            {
                pen.setDashPattern({ 5, 2 });
            }

            painter.setPen(pen);
            painter.drawLine(QPointF(4, 12), QPointF(36, 12));
            break;
        }
        case UBPictureChoice::Kind::BorderStyle:
        {
            painter.setPen(pen);

            if (index == UBBorderStyle::Cloud)
            {
                QPainterPath path;
                path.moveTo(4, 14);

                for (int x = 4; x < 36; x += 8)
                {
                    path.quadTo(x + 4, 4, x + 8, 14);
                }

                painter.drawPath(path);
            }
            else
            {
                painter.drawLine(QPointF(4, 12), QPointF(36, 12));
            }
            break;
        }
        case UBPictureChoice::Kind::StartEnding:
        case UBPictureChoice::Kind::EndEnding:
        {
            painter.setPen(pen);
            const bool atStart = (kind == UBPictureChoice::Kind::StartEnding);
            const QPointF from = atStart ? QPointF(36, 12) : QPointF(4, 12);
            const QPointF tip = atStart ? QPointF(5, 12) : QPointF(35, 12);
            painter.drawLine(from, tip);
            drawEnding(painter, index, tip, atStart ? qDegreesToRadians(180.0) : 0.0);
            break;
        }
    }

    return QIcon(pixmap);
}

UBPictureChoice::UBPictureChoice(Kind kind, int count, QWidget* parent)
    : QWidget(parent)
{
    mGroup = new QButtonGroup(this);
    mGroup->setExclusive(true);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    for (int i = 0; i < count; ++i)
    {
        auto button = new QToolButton(this);
        button->setIcon(pictureIcon(kind, i));
        button->setIconSize(QSize(40, 24));
        button->setCheckable(true);
        mGroup->addButton(button, i);
        layout->addWidget(button);
    }

    layout->addStretch();

    connect(mGroup, &QButtonGroup::idClicked, this, &UBPictureChoice::choiceChanged);
}

void UBPictureChoice::setChoice(int index)
{
    if (auto button = mGroup->button(index))
    {
        button->setChecked(true);
    }
}

int UBPictureChoice::choice() const
{
    return mGroup->checkedId();
}

UBStrokePreview::UBStrokePreview(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(56);
}

void UBStrokePreview::setStroke(const QColor& color, qreal width, qreal opacity, bool glow)
{
    mColor = color;
    mWidth = width;
    mOpacity = opacity;
    mGlow = glow;
    update();
}

void UBStrokePreview::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // A light card, so that dark colors show on the dark panel.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#e9ebef"));
    painter.drawRoundedRect(QRectF(rect()).adjusted(width() * 0.2, 2, -width() * 0.2, -2), 8, 8);

    const qreal left = width() * 0.3;
    const qreal right = width() * 0.7;
    const qreal middle = height() / 2.0;
    const qreal amplitude = height() * 0.22;

    QPainterPath wave;
    wave.moveTo(left, middle);
    wave.cubicTo(left + (right - left) * 0.3, middle - 2 * amplitude,
                 left + (right - left) * 0.7, middle + 2 * amplitude,
                 right, middle);

    const qreal penWidth = qBound(1.0, mWidth, 20.0);

    QColor color = mColor.isValid() ? mColor : QColor(Qt::white);

    if (mGlow)
    {
        QColor glow = color;
        glow.setAlphaF(0.25 * mOpacity);
        painter.setPen(QPen(glow, penWidth * 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(wave);
    }

    color.setAlphaF(mOpacity);
    painter.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawPath(wave);
}
