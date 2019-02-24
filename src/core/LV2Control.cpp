/*
 * LV2Control.cpp - model for controlling a LV2 port
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

#include <cstdio>

#include "LV2Control.h"
#include "LV2Base.h"


LV2Control::LV2Control( Model * _parent, lv2_port_desc_t * _port, bool _link ) :
	Model( _parent,QString("LV2 Control %1").arg(_port->name) ),
	m_link( _link ),
	m_port( _port ),
	m_linkEnabledModel( _link, this, tr( "Link channels" ) ),
	m_toggledModel( false, this, m_port->name ),
	m_knobModel( 0, 0, 0, 1, this, m_port->name ),
	m_tempoSyncKnobModel( 0, 0, 0, 1, m_port->max.f, this, m_port->name )
{
	if( m_link )
	{
		connect( &m_linkEnabledModel, SIGNAL( dataChanged() ),
                         this, SLOT( linkStateChanged() ) );
	}

	switch( m_port->data_type )
	{
		case TOGGLED:
			connect( &m_toggledModel, SIGNAL( dataChanged() ),
                                 this, SLOT( ledChanged() ) );
			if( m_port->def.f == 1.0f )
			{
				m_toggledModel.setValue( true );
			}
			// TODO: careful: we must prevent saved scales
			m_toggledModel.setScaleLogarithmic( m_port->suggests_logscale );
			break;

		case INTEGER:
			m_knobModel.setRange( static_cast<int>( m_port->max.f ),
					  static_cast<int>( m_port->min.f ),
					  1 + static_cast<int>( m_port->max.f -
							  m_port->min.f ) / 400 );
			m_knobModel.setInitValue(
					static_cast<int>( m_port->def.f ) );
			connect( &m_knobModel, SIGNAL( dataChanged() ),
						 this, SLOT( knobChanged() ) );
			// TODO: careful: we must prevent saved scales
			m_knobModel.setScaleLogarithmic( m_port->suggests_logscale );
			break;

		case FLOATING:
			m_knobModel.setRange( m_port->min.f, m_port->max.f,
				( m_port->max.f - m_port->min.f )
				/ ( m_port->name.toUpper() == "GAIN"
					&& m_port->max.f == 10.0f ? 4000.0f :
								( m_port->suggests_logscale ? 8000.0f : 800.0f ) ) );
			m_knobModel.setInitValue( m_port->def.f );
			connect( &m_knobModel, SIGNAL( dataChanged() ),
						 this, SLOT( knobChanged() ) );
			// TODO: careful: we must prevent saved scales
			m_knobModel.setScaleLogarithmic( m_port->suggests_logscale );
			break;

		case TIME:
			m_tempoSyncKnobModel.setRange( m_port->min.f, m_port->max.f,
					  ( m_port->max.f -
						m_port->min.f ) / 800.0f );
			m_tempoSyncKnobModel.setInitValue( m_port->def.f );
			connect( &m_tempoSyncKnobModel, SIGNAL( dataChanged() ),
					 this, SLOT( tempoKnobChanged() ) );
			// TODO: careful: we must prevent saved scales
			m_tempoSyncKnobModel.setScaleLogarithmic( m_port->suggests_logscale );
			break;

		default:
			break;
	}
}




LV2Control::~LV2Control()
{
}




LV2_Data LV2Control::value()
{
        LV2_Data r;
        r.f=0.f;

	switch( m_port->data_type )
	{
		case TOGGLED:
			r.f=m_toggledModel.value();
                        break;
		case INTEGER:
		case FLOATING:
			r.f=m_knobModel.value();
                        break;
		case TIME:
			r.f=m_tempoSyncKnobModel.value();
                        break;
		default:
			qWarning( "LV2Control::value(): BAD BAD BAD\n" );
			break;
	}

	return r;
}


ValueBuffer * LV2Control::valueBuffer()
{
	switch( m_port->data_type )
	{
		case TOGGLED:
		case INTEGER:
			return NULL;
		case FLOATING:
			return m_knobModel.valueBuffer();
		case TIME:
			return m_tempoSyncKnobModel.valueBuffer();
		default:
			qWarning( "LV2Control::valueBuffer(): BAD BAD BAD\n" );
			break;
	}

	return NULL;
}



void LV2Control::setValue( LV2_Data _value )
{
	switch( m_port->data_type )
	{
		case TOGGLED:
			m_toggledModel.setValue( static_cast<bool>( _value.f ) );
			break;
		case INTEGER:
			m_knobModel.setValue( static_cast<int>( _value.f ) );
			break;
		case FLOATING:
			m_knobModel.setValue( static_cast<float>( _value.f ) );
			break;
		case TIME:
			m_tempoSyncKnobModel.setValue( static_cast<float>(
								_value.f ) );
			break;
		default:
			printf("LV2Control::setValue BAD BAD BAD\n");
			break;
	}
}




void LV2Control::saveSettings( QDomDocument& doc,
					   QDomElement& parent,
					   const QString& name )
{
	QDomElement e = doc.createElement( name );

	if( m_link )
	{
		m_linkEnabledModel.saveSettings( doc, e, "link" );
	}
	switch( m_port->data_type )
	{
		case TOGGLED:
			m_toggledModel.saveSettings( doc, e, "data" );
			break;
		case INTEGER:
		case FLOATING:
			m_knobModel.saveSettings( doc, e, "data" );
			break;
		case TIME:
			m_tempoSyncKnobModel.saveSettings( doc, e, "data" );
			break;
		default:
			printf("LV2Control::saveSettings BAD BAD BAD\n");
			break;
	}

	parent.appendChild( e );
}




void LV2Control::loadSettings( const QDomElement& parent, const QString& name )
{
	QString dataModelName = "data";
	QString linkModelName = "link";
	QDomElement e = parent.namedItem( name ).toElement();

	// COMPAT < 1.0.0: detect old data format where there's either no dedicated sub
	// element or there's a direct sub element with automation link information
	if( e.isNull() || e.hasAttribute( "id" ) )
	{
		dataModelName = name;
		linkModelName = name + "link";
		e = parent;
	}

	if( m_link )
	{
		m_linkEnabledModel.loadSettings( e, linkModelName );
	}

	switch( m_port->data_type )
	{
		case TOGGLED:
			m_toggledModel.loadSettings( e, dataModelName );
			break;
		case INTEGER:
		case FLOATING:
			m_knobModel.loadSettings( e, dataModelName );
			break;
		case TIME:
			m_tempoSyncKnobModel.loadSettings( e, dataModelName );
			break;
		default:
			printf("LV2Control::loadSettings BAD BAD BAD\n");
			break;
	}
}




void LV2Control::linkControls( LV2Control * _control )
{
	switch( m_port->data_type )
	{
		case TOGGLED:
			BoolModel::linkModels( &m_toggledModel, _control->toggledModel() );
			break;
		case INTEGER:
		case FLOATING:
			FloatModel::linkModels( &m_knobModel, _control->knobModel() );
			break;
		case TIME:
			TempoSyncKnobModel::linkModels( &m_tempoSyncKnobModel,
					_control->tempoSyncKnobModel() );
			break;
		default:
			break;
	}
}




void LV2Control::ledChanged()
{
        LV2_Data r;
        r.f=m_toggledModel.value();
	emit changed( m_port->port_id, r );
}




void LV2Control::knobChanged()
{
        LV2_Data r;
        r.f=m_knobModel.value();
	emit changed( m_port->port_id, r );
}




void LV2Control::tempoKnobChanged()
{
        LV2_Data r;
        r.f=m_tempoSyncKnobModel.value();
	emit changed( m_port->port_id, r );
}




void LV2Control::unlinkControls( LV2Control * _control )
{
	switch( m_port->data_type )
	{
		case TOGGLED:
			BoolModel::unlinkModels( &m_toggledModel, _control->toggledModel() );
			break;
		case INTEGER:
		case FLOATING:
			FloatModel::unlinkModels( &m_knobModel, _control->knobModel() );
			break;
		case TIME:
			TempoSyncKnobModel::unlinkModels( &m_tempoSyncKnobModel,
					_control->tempoSyncKnobModel() );
			break;
		default:
			break;
	}
}




void LV2Control::linkStateChanged()
{
	emit linkChanged( m_port->control_id, m_linkEnabledModel.value() );
}




void LV2Control::setLink( bool _state )
{
	m_linkEnabledModel.setValue( _state );
}




