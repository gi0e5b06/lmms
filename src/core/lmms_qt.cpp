/*
 * lmms_qt.cpp - generic Qt-related stuff
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include "lmms_qt.h"

#include "lmms_basics.h" // REQUIRED

QString TR_lmms_fix(QString _s)
{
        // doc: t.replace(QRegExp("<i>([^<]*)</i>"), "\\emph{\\1}");
        // ‎Typing "«" and "»" on ...
        _s.replace("<Alt>","«" UI_ALT_KEY "»");
        _s.replace("<Ctrl>","«" UI_CTRL_KEY "»");
        _s.replace("<Shift>","«" UI_SHIFT_KEY "»");
        _s.replace("<" UI_CTRL_KEY ">","«" UI_CTRL_KEY "»");
        _s.replace("[(]Alt (.)[)]","«" UI_ALT_KEY " \\1»");
        _s.replace("[(]Ctrl (.)[)]","«" UI_CTRL_KEY " \\1»");
        _s.replace("[(]Shift (.)[)]","«" UI_SHIFT_KEY " \\1»");
        _s.replace("'Alt (.)'","«" UI_ALT_KEY " \\1»");
        _s.replace("'Ctrl (.)'","«" UI_CTRL_KEY " \\1»");
        _s.replace("'Shift (.)'","«" UI_SHIFT_KEY " \\1»");
        return _s;
}
