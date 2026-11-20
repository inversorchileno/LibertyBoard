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

#include <QColor>
#include <QIcon>

/**
 * @brief The icons of the arrow, rectangle and ellipse tools, drawn in the
 * colors the next shape will have: its border, and its filling when it is
 * filled.
 */
namespace UBShapeToolIcon
{
    /**
     * @param tool UBStylusTool::Arrow, Rectangle (the shapes tool) or Ellipse. An invalid fill color means not filled.
     * @param kind for the shapes tool, the UBShapeKind chosen.
     */
    QIcon create(int tool, const QColor& stroke, const QColor& fill, int kind = 0);
}
