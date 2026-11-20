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
#include <QEvent>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>

#include "core/UBSettings.h"
#include "gui/UBActionPalette.h"

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
            // No icon: the box shows whether the button is shown.
            QAction* box = sub->addAction(tool->text());
            box->setCheckable(true);
            box->setChecked(isShown(palette, tool));
            QObject::connect(box, &QAction::toggled, sub, [palette, tool](bool shown) { setShown(palette, tool, shown); });
        }
    }
}
