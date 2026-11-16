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

#include "UBLaserPreferencesPage.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPixmap>
#include <QSpinBox>
#include <QVBoxLayout>

#include "core/UBSetting.h"
#include "core/UBSettings.h"

static QIcon colorIcon(const QColor& color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    return QIcon(pixmap);
}

UBLaserPreferencesPage::UBLaserPreferencesPage(QWidget* parent)
    : QWidget(parent)
{
    const QList<QPair<QString, QColor>> colors = {
        { tr("Red"), QColor("#ff2828") },
        { tr("Orange"), QColor("#ff9a1f") },
        { tr("Magenta"), QColor("#ff1fd2") },
        { tr("Green"), QColor("#2bff2b") },
        { tr("Blue"), QColor("#2f54ff") },
        { tr("Purple"), QColor("#9a46f0") },
        { tr("Black"), QColor("#000000") },
        { tr("Dark green"), QColor("#14b85a") },
        { tr("Yellow"), QColor("#ffee1f") },
    };

    mColorBox = new QComboBox(this);

    for (const auto& color : colors)
    {
        mColorBox->addItem(colorIcon(color.second), color.first, color.second.name());
    }

    mWidthBox = new QSpinBox(this);
    mWidthBox->setRange(1, 40);
    mWidthBox->setSuffix(tr(" px"));

    mDelayBox = new QDoubleSpinBox(this);
    mDelayBox->setRange(0.0, 5.0);
    mDelayBox->setSingleStep(0.1);
    mDelayBox->setDecimals(1);
    mDelayBox->setSuffix(tr(" s"));

    mDurationBox = new QDoubleSpinBox(this);
    mDurationBox->setRange(0.1, 10.0);
    mDurationBox->setSingleStep(0.1);
    mDurationBox->setDecimals(1);
    mDurationBox->setSuffix(tr(" s"));

    auto form = new QFormLayout();
    form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    form->addRow(tr("Color"), mColorBox);
    form->addRow(tr("Width"), mWidthBox);
    form->addRow(tr("Wait before fading"), mDelayBox);
    form->addRow(tr("Fading duration"), mDurationBox);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addStretch();

    loadSettings();

    UBSettings* settings = UBSettings::settings();

    connect(mColorBox, qOverload<int>(&QComboBox::activated), this, &UBLaserPreferencesPage::onColorChosen);
    connect(mWidthBox, qOverload<int>(&QSpinBox::valueChanged), settings->laserWidth, [settings](int value) {
        settings->laserWidth->set(double(value));
    });
    connect(mDelayBox, qOverload<double>(&QDoubleSpinBox::valueChanged), settings->laserFadeDelay, [settings](double value) {
        settings->laserFadeDelay->set(value);
    });
    connect(mDurationBox, qOverload<double>(&QDoubleSpinBox::valueChanged), settings->laserFadeDuration, [settings](double value) {
        settings->laserFadeDuration->set(value);
    });
}

void UBLaserPreferencesPage::loadSettings()
{
    UBSettings* settings = UBSettings::settings();

    const QString colorName = QColor(settings->laserColor->get().toString()).name();
    int colorIndex = mColorBox->findData(colorName);

    if (colorIndex < 0)
    {
        // A color chosen elsewhere is kept and shown as its own entry.
        mColorBox->addItem(colorIcon(QColor(colorName)), colorName, colorName);
        colorIndex = mColorBox->count() - 1;
    }

    mColorBox->setCurrentIndex(colorIndex);
    mWidthBox->setValue(qRound(settings->laserWidth->get().toDouble()));
    mDelayBox->setValue(settings->laserFadeDelay->get().toDouble());
    mDurationBox->setValue(settings->laserFadeDuration->get().toDouble());
}

void UBLaserPreferencesPage::resetToDefaults()
{
    UBSettings* settings = UBSettings::settings();

    settings->laserColor->reset();
    settings->laserWidth->reset();
    settings->laserFadeDelay->reset();
    settings->laserFadeDuration->reset();

    loadSettings();
}

void UBLaserPreferencesPage::onColorChosen(int index)
{
    UBSettings::settings()->laserColor->set(mColorBox->itemData(index).toString());
}
