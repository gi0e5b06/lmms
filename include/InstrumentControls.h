/*
 * InstrumentControls.h - model for instrument-controls
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

#ifndef INSTRUMENT_CONTROLS_H
#define INSTRUMENT_CONTROLS_H

#include "Model.h"
#include "JournallingObject.h"
#include "Instrument.h"

class InstrumentControlDialog;


class InstrumentControls : public JournallingObject, public Model
{
 public:
        InstrumentControls( Instrument* _instrument ) :
	        JournallingObject(),
		Model( _instrument ),
		m_instrument( _instrument )
		{
		}

	virtual ~InstrumentControls()
	{
	}

	virtual int controlCount() = 0;
	virtual InstrumentControlDialog * createView() = 0;

	Instrument* instrument()
	{
		return m_instrument;
	}


private:
	Instrument* m_instrument;

} ;

#endif
