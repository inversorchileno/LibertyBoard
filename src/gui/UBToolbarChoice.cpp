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


#include "UBToolbarChoice.h"

#include <QAction>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidgetAction>
#include <QEvent>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>

#include "core/UBSettings.h"
#include "gui/UBActionPalette.h"
#include "gui/UBShortcutsDialog.h"

namespace
{
    struct Registered
    {
        QPointer<UBActionPalette> palette;
        QList<QAction*> choosable;
    };

    QMap<UBToolbarChoice::Palette, Registered>& palettes()
    {
        static QMap<UBToolbarChoice::Palette, Registered> registered;
        return registered;
    }

    UBSetting* hiddenSetting(UBToolbarChoice::Palette palette)
    {
        return palette == UBToolbarChoice::Palette::Board ? UBSettings::settings()->hiddenBoardTools
                                                           : UBSettings::settings()->hiddenDesktopTools;
    }

    /*
     * A line of the Customize menu: a box with the name of the tool, and its
     * icon at the right. A click anywhere on the line ticks the box; the
     * menu stays open, to tick several.
     */
    class ToolRow : public QWidget
    {
    public:
        ToolRow(QAction* tool, bool shown, QWidget* parent)
            : QWidget(parent)
        {
            setObjectName("toolRow");
            setAttribute(Qt::WA_StyledBackground);
            setAttribute(Qt::WA_Hover);
            // Dark text on the light menu, white on the line under the mouse.
            setStyleSheet("QWidget#toolRow { background: transparent; }"
                          "QWidget#toolRow:hover { background: #308cc6; }");

            mBox = new QCheckBox(tool->text(), this);
            QPalette colors = mBox->palette();
            colors.setColor(QPalette::Base, Qt::white);
            colors.setColor(QPalette::Text, QColor("#1e1e1e"));
            colors.setColor(QPalette::WindowText, QColor("#1e1e1e"));
            mBox->setPalette(colors);
            mBox->setStyleSheet("QCheckBox { color: #1e1e1e; }");
            mBox->setChecked(shown);

            auto icon = new QLabel(this);
            icon->setPixmap(tool->icon().pixmap(QSize(22, 22)));

            auto layout = new QHBoxLayout(this);
            layout->setContentsMargins(8, 3, 10, 3);
            layout->setSpacing(12);
            layout->addWidget(mBox);
            layout->addStretch(1);
            layout->addWidget(icon);
        }

        QCheckBox* box() const
        {
            return mBox;
        }

    protected:
        void enterEvent(QEnterEvent*) override
        {
            mBox->setStyleSheet("QCheckBox { color: #ffffff; }");
        }

        void leaveEvent(QEvent*) override
        {
            mBox->setStyleSheet("QCheckBox { color: #1e1e1e; }");
        }

        void mouseReleaseEvent(QMouseEvent* event) override
        {
            // On the name or the icon, as on the box.
            if (event->button() == Qt::LeftButton)
            {
                mBox->toggle();
            }
        }

    private:
        QCheckBox* mBox;
    };

    // A click on a box ticks it without closing the menu, to tick several.
    class KeepMenuOpen : public QObject
    {
    public:
        using QObject::QObject;

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override
        {
            auto menu = qobject_cast<QMenu*>(watched);

            if (menu && event->type() == QEvent::MouseButtonRelease)
            {
                QAction* action = menu->actionAt(static_cast<QMouseEvent*>(event)->position().toPoint());

                if (action && action->isCheckable() && action->isEnabled())
                {
                    action->trigger();
                    return true;
                }
            }

            return QObject::eventFilter(watched, event);
        }
    };
}

void UBToolbarChoice::registerPalette(Palette palette, UBActionPalette* actionPalette, const QList<QAction*>& choosable)
{
    palettes()[palette] = Registered{ actionPalette, choosable };
    apply(palette);

    // Their shortcuts work also on the window of the palette, the desktop.
    actionPalette->window()->addActions(choosable);
    UBShortcutsDialog::adopt(choosable);
}

QList<QAction*> UBToolbarChoice::choosableActions(Palette palette)
{
    return palettes().value(palette).choosable;
}

bool UBToolbarChoice::isShown(Palette palette, QAction* action)
{
    return !hiddenSetting(palette)->get().toStringList().contains(action->objectName());
}

void UBToolbarChoice::setShown(Palette palette, QAction* action, bool shown)
{
    QStringList hidden = hiddenSetting(palette)->get().toStringList();
    hidden.removeAll(action->objectName());

    if (!shown)
    {
        hidden << action->objectName();
    }

    hiddenSetting(palette)->set(hidden);
    apply(palette);
}

void UBToolbarChoice::apply(Palette palette)
{
    const Registered registered = palettes().value(palette);

    if (!registered.palette)
    {
        return;
    }

    QList<QAction*> hidden;

    for (QAction* action : registered.choosable)
    {
        if (!isShown(palette, action))
        {
            hidden << action;
        }
    }

    registered.palette->setHiddenActions(hidden);
}

void UBToolbarChoice::fillToolsMenu(QMenu* menu)
{
    // The tools of the board, then those only on the desktop: one entry
    // each, whatever the palettes show.
    menu->clear();
    QList<QAction*> listed;

    for (Palette palette : { Palette::Board, Palette::Desktop })
    {
        for (QAction* action : palettes().value(palette).choosable)
        {
            if (!listed.contains(action) && action->actionGroup())
            {
                menu->addAction(action);
                listed << action;
            }
        }
    }
}

void UBToolbarChoice::fillCustomizeMenu(QMenu* menu)
{
    menu->clear();

    const QList<QPair<Palette, QString>> sections = {
        { Palette::Desktop, tr("Desktop") },
        { Palette::Board, tr("Board / PDF") },
    };

    for (const auto& section : sections)
    {
        const Palette palette = section.first;
        QMenu* sub = menu->addMenu(section.second);
        sub->installEventFilter(new KeepMenuOpen(sub));

        for (QAction* tool : palettes().value(palette).choosable)
        {
            // The box at the left shows whether the button is shown, its
            // icon at the right which button it is.
            auto row = new ToolRow(tool, isShown(palette, tool), sub);
            QObject::connect(row->box(), &QCheckBox::toggled, sub, [palette, tool](bool shown) { setShown(palette, tool, shown); });

            auto item = new QWidgetAction(sub);
            item->setDefaultWidget(row);
            sub->addAction(item);
        }
    }
}
