/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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




#include "UBDesktopPalette.h"

#include <QtGui>


#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "gui/UBMainWindow.h"

#include "core/memcheck.h"

UBDesktopPalette::UBDesktopPalette(QWidget *parent, UBRightPalette* _rightPalette)
    : UBActionPalette(Qt::TopLeftCorner, parent)
    , mShowHideAction(NULL)
    , mDisplaySelectAction(NULL)
    , rightPalette(_rightPalette)
{
    QList<QAction*> actions;

    mActionUniboard = new QAction(QIcon(":/images/toolbar/board.png"), tr("Show LibertyBoard"), this);
    connect(mActionUniboard, SIGNAL(triggered()), this, SIGNAL(uniboardClick()));
    actions << mActionUniboard;


    actions << UBApplication::mainWindow->actionPen;
    actions << UBApplication::mainWindow->actionEraser;
    actions << UBApplication::mainWindow->actionMarker;
    actions << UBApplication::mainWindow->actionPointer;
    actions << UBApplication::mainWindow->actionSelector;
    actions << UBApplication::mainWindow->actionArrowShape;
    actions << UBApplication::mainWindow->actionRectangle;
    actions << UBApplication::mainWindow->actionEllipse;
    actions << UBApplication::mainWindow->actionUnderline;
    actions << UBApplication::mainWindow->actionText;

    // The screen captures stay available through their shortcuts and the
    // controller, but they are not shown on the palette.
    mActionCustomSelect = new QAction(QIcon(":/images/toolbar/captureArea.png"), tr("Capture Part of the Screen"), this);
    connect(mActionCustomSelect, SIGNAL(triggered()), this, SIGNAL(customClick()));

    mDisplaySelectAction = new QAction(QIcon(":/images/toolbar/captureScreen.png"), tr("Capture the Screen"), this);
    connect(mDisplaySelectAction, SIGNAL(triggered()), this, SIGNAL(screenClick()));

    QIcon showHideIcon;
    showHideIcon.addPixmap(QPixmap(":/images/toolbar/eyeOpened.png"), QIcon::Normal , QIcon::On);
    showHideIcon.addPixmap(QPixmap(":/images/toolbar/eyeClosed.png"), QIcon::Normal , QIcon::Off);
    mShowHideAction = new QAction(showHideIcon, "", this);
    mShowHideAction->setCheckable(true);

    connect(mShowHideAction, SIGNAL(triggered(bool)), this, SLOT(showHideClick(bool)));
    actions << mShowHideAction;

    setActions(actions);
    setButtonIconSize(QSize(42, 42));

    adjustSizeAndPosition();

    //  This palette can be minimized
    QIcon maximizeIcon;
    maximizeIcon.addPixmap(QPixmap(":/images/toolbar/stylusTab.png"), QIcon::Normal, QIcon::On);
    mMaximizeAction = new QAction(maximizeIcon, tr("Show the stylus palette"), this);
    connect(mMaximizeAction, SIGNAL(triggered()), this, SLOT(maximizeMe()));
    connect(this, SIGNAL(maximizeStart()), this, SLOT(maximizeMe()));
    connect(this, SIGNAL(minimizeStart(eMinimizedLocation)), this, SLOT(minimizeMe(eMinimizedLocation)));
    setMinimizePermission(true);

    connect(rightPalette, SIGNAL(resized()), this, SLOT(parentResized()));
}


UBDesktopPalette::~UBDesktopPalette()
{

}


void UBDesktopPalette::adjustPosition()
{
    QPoint pos = this->pos();
    if(this->pos().y() < 30){
        pos.setY(30);
        moveInsideParent(pos);
    }
}

void UBDesktopPalette::disappearForCapture()
{
    setWindowOpacity(0.0);
    qApp->processEvents();
}


void UBDesktopPalette::appear()
{
    setWindowOpacity(1.0);
}


void UBDesktopPalette::showHideClick(bool checked)
{
    UBApplication::applicationController->mirroringEnabled(checked);
}


void UBDesktopPalette::updateShowHideState(bool pShowEnabled)
{
    mShowHideAction->setChecked(pShowEnabled);

    if (mShowHideAction->isChecked())
        mShowHideAction->setToolTip(tr("Show Board on Secondary Screen"));
    else
        mShowHideAction->setToolTip(tr("Show Desktop on Secondary Screen"));

    if (pShowEnabled)
        raise();
}


void UBDesktopPalette::setShowHideButtonVisible(bool visible)
{
    mShowHideAction->setVisible(visible);
}


void UBDesktopPalette::setDisplaySelectButtonVisible(bool visible)
{
    mDisplaySelectAction->setVisible(visible);
}

//  Called when the palette is near the border and must be minimized
void UBDesktopPalette::minimizeMe(eMinimizedLocation location)
{
    Q_UNUSED(location);
    QList<QAction*> actions;
    clearLayout();

    actions << mMaximizeAction;
    setActions(actions);

    adjustSizeAndPosition();

#ifdef UB_REQUIRES_MASK_UPDATE
        emit refreshMask();
#endif
}

//  Called when the user wants to maximize the palette
void UBDesktopPalette::maximizeMe()
{
    QList<QAction*> actions;
    clearLayout();

    actions << mActionUniboard;
    actions << UBApplication::mainWindow->actionPen;
    actions << UBApplication::mainWindow->actionEraser;
    actions << UBApplication::mainWindow->actionMarker;
    actions << UBApplication::mainWindow->actionPointer;
    actions << UBApplication::mainWindow->actionSelector;
    actions << UBApplication::mainWindow->actionArrowShape;
    actions << UBApplication::mainWindow->actionRectangle;
    actions << UBApplication::mainWindow->actionEllipse;
    actions << UBApplication::mainWindow->actionUnderline;
    actions << UBApplication::mainWindow->actionText;
    actions << mShowHideAction;

    setActions(actions);

    adjustSizeAndPosition();

    // Notify that the maximization has been done
    emit maximized();
#ifdef UB_REQUIRES_MASK_UPDATE
        emit refreshMask();
#endif
}

/**
 * \brief Adds the little arrow that opens the options of a tool, as on the pen icon.
 */
static QIcon iconWithOptionsArrow(const QIcon& icon)
{
    QIcon result;
    const QSize size(42, 42);

    for (QIcon::State state : { QIcon::Off, QIcon::On })
    {
        QPixmap pixmap = icon.pixmap(size, QIcon::Normal, state);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor(220, 220, 220), 1));
        painter.setBrush(Qt::black);
        painter.drawPolygon(QPolygonF({ QPointF(35, 30), QPointF(41, 35), QPointF(35, 40) }));
        painter.end();

        result.addPixmap(pixmap, QIcon::Normal, state);
    }

    return result;
}

void UBDesktopPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    // Tools without an arrow icon of their own
    const QList<QAction*> toolsWithOptions = {
        UBApplication::mainWindow->actionPointer, UBApplication::mainWindow->actionArrowShape,
        UBApplication::mainWindow->actionRectangle, UBApplication::mainWindow->actionEllipse,
        UBApplication::mainWindow->actionUnderline, UBApplication::mainWindow->actionText
    };

    for (QAction* action : toolsWithOptions)
    {
        if (!mIconsWithoutArrow.contains(action))
        {
            mIconsWithoutArrow.insert(action, action->icon());
            action->setIcon(iconWithOptionsArrow(action->icon()));
        }
    }

    QIcon penIcon;
    QIcon markerIcon;
    QIcon eraserIcon;
    penIcon.addFile(":images/stylusPalette/penArrow.svg", QSize(), QIcon::Normal, QIcon::Off);
    penIcon.addFile(":images/stylusPalette/penOnArrow.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionPen->setIcon(penIcon);
    markerIcon.addFile(":images/stylusPalette/markerArrow.svg", QSize(), QIcon::Normal, QIcon::Off);
    markerIcon.addFile(":images/stylusPalette/markerOnArrow.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionMarker->setIcon(markerIcon);
    eraserIcon.addFile(":images/stylusPalette/eraserArrow.svg", QSize(), QIcon::Normal, QIcon::Off);
    eraserIcon.addFile(":images/stylusPalette/eraserOnArrow.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionEraser->setIcon(eraserIcon);

    adjustPosition();
}

void UBDesktopPalette::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);

    for (auto it = mIconsWithoutArrow.constBegin(); it != mIconsWithoutArrow.constEnd(); ++it)
    {
        it.key()->setIcon(it.value());
    }

    mIconsWithoutArrow.clear();

    QIcon penIcon;
    QIcon markerIcon;
    QIcon eraserIcon;
    penIcon.addFile(":images/stylusPalette/pen.svg", QSize(), QIcon::Normal, QIcon::Off);
    penIcon.addFile(":images/stylusPalette/penOn.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionPen->setIcon(penIcon);
    markerIcon.addFile(":images/stylusPalette/marker.svg", QSize(), QIcon::Normal, QIcon::Off);
    markerIcon.addFile(":images/stylusPalette/markerOn.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionMarker->setIcon(markerIcon);
    eraserIcon.addFile(":images/stylusPalette/eraser.svg", QSize(), QIcon::Normal, QIcon::Off);
    eraserIcon.addFile(":images/stylusPalette/eraserOn.svg", QSize(), QIcon::Normal, QIcon::On);
    UBApplication::mainWindow->actionEraser->setIcon(eraserIcon);
}

QPoint UBDesktopPalette::buttonPos(QAction *action)
{
    QPoint p;

    UBActionPaletteButton* pB = mMapActionToButton[action];
    if(NULL != pB)
    {
        p = pB->pos();
    }

    return p;
}


int UBDesktopPalette::getParentRightOffset()
{
    return rightPalette->width();
}

void UBDesktopPalette::parentResized()
{
    QPoint p = pos();
    if (minimizedLocation() == eMinimizedLocation_Right)
    {
        p.setX(parentWidget()->width() - getParentRightOffset() -width());
    }

    moveInsideParent(p);
}
