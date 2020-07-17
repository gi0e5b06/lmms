/*
 * MidiMapper.h -
 *
 * Copyright (c) 2020      gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef MIDI_MAPPER_H
#define MIDI_MAPPER_H

#include "Model.h"
#include "ModelView.h"

#include <QHash>
#include <QWidget>

class MidiMapper
{
  public:
    static int list(QWidget* _w);
    static int map(QWidget* _w,const QString& _p);
    static int unmap(QWidget* _w);
    static void collect(QWidget* _w, QHash<ModelView*, Model*>& _table);
};

#endif
