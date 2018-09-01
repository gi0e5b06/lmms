/*
 * AutomationPattern.h - declaration of class AutomationPattern, which contains
 *                       all information about an automation pattern
 *
 * Copyright (c) 2018      gi0e5b06 (on github.com)
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2006-2008 Javier Serrano Polo <jasp00/at/users.sourceforge.net>
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

#ifndef AUTOMATION_PATTERN_H
#define AUTOMATION_PATTERN_H

#include <QMap>
#include <QPointer>

#include "Track.h"


class AutomationTrack;
class MidiTime;



class EXPORT AutomationPattern : public TrackContentObject
{
	Q_OBJECT
public:
	enum ProgressionTypes
	{
		DiscreteProgression,
		LinearProgression,
		CubicHermiteProgression,
		ParabolicProgression
	} ;

	typedef QMap<int, float> timeMap;
	typedef QVector<QPointer<AutomatableModel> > objectVector;

	AutomationPattern( AutomationTrack * _auto_track );
	AutomationPattern( const AutomationPattern & _pat_to_copy );
	virtual ~AutomationPattern();

	bool addObject( AutomatableModel * _obj, bool _search_dup = true );

	const AutomatableModel * firstObject() const;
	const objectVector& objects() const;

	MidiTime putValue( const MidiTime & time,
				const float value,
				const bool quantPos = true,
				const bool ignoreSurroundingPoints = true );

	void removeValue( const MidiTime & time );

	// progression-type stuff
	inline ProgressionTypes progressionType() const
	{
		return m_progressionType;
	}
	void setProgressionType( ProgressionTypes _progressionType );

	inline float tension() const
	{
		return m_tension;
	}
	void setTension( const float _tension );

	inline int waveBank() const
	{
		return m_waveBank;
	}
	void setWaveBank( const int _waveBank );

	inline int waveIndex() const
	{
		return m_waveIndex;
	}
	void setWaveIndex( const int _waveIndex );

	inline float waveRatio() const
	{
		return m_waveRatio;
	}
	void setWaveRatio( const float _waveRatio );

	inline float waveSkew() const
	{
		return m_waveSkew;
	}
	void setWaveSkew( const float _waveSkew );

	inline float waveAmplitude() const
	{
		return m_waveAmplitude;
	}
	void setWaveAmplitude( const float _waveAmplitude );

	inline float waveRepeat() const
	{
		return m_waveRepeat;
	}
	void setWaveRepeat( const float _waveRepeat );

	MidiTime timeMapLength() const;

	//virtual MidiTime beatLength() const;
	//virtual void changeLength( const MidiTime & _length );
	virtual void updateLength();
        //virtual void updateBBTrack();

	MidiTime setDragValue( const MidiTime & time,
				const float value,
				const bool quantPos = true,
				const bool controlKey = false );

	void applyDragValue();


	bool isDragging() const
	{
		return m_dragging;
	}

	inline const timeMap & getTimeMap() const
	{
		return m_timeMap;
	}

	inline timeMap & getTimeMap()
	{
		return m_timeMap;
	}

	inline const timeMap & getTangents() const
	{
		return m_tangents;
	}

	inline timeMap & getTangents()
	{
		return m_tangents;
	}

	inline float getMin() const
	{
		return firstObject()->minValue<float>();
	}

	inline float getMax() const
	{
		return firstObject()->maxValue<float>();
	}

	inline bool hasAutomation() const
	{
		return m_timeMap.isEmpty() == false;
	}

	float valueAt( const MidiTime & _time ) const;
	float *valuesAfter( const MidiTime & _time ) const;

	const QString name() const;

	// settings-management
	virtual void saveSettings( QDomDocument & _doc, QDomElement & _parent );
	virtual void loadSettings( const QDomElement & _this );

	static const QString classNodeName() { return "automationpattern"; }
	QString nodeName() const { return classNodeName(); }

	virtual TrackContentObjectView * createView( TrackView * _tv );


	static bool isAutomated( const AutomatableModel * _m );
	static QVector<AutomationPattern *> patternsForModel( const AutomatableModel * _m );
	static AutomationPattern * globalAutomationPattern( AutomatableModel * _m );
	static void resolveAllIDs();

	bool isRecording() const { return m_isRecording; }
	void setRecording( const bool b );

	static int quantization() { return s_quantization; }
	static void setQuantization(int q) { s_quantization = q; }

public slots:
	virtual void clear();

	void objectDestroyed( jo_id_t );
	void flipY( int min, int max );
	void flipY();
	void flipX( int length = -1 );

	void onRecordValue(MidiTime time, float value);

 signals:
	void recordValue(MidiTime time, float value);

private:
	void cleanObjects();
	void generateTangents();
	void generateTangents( timeMap::const_iterator it, int numToGenerate );
	float valueAt( timeMap::const_iterator v, int offset,
                       bool xruns=false ) const;

	AutomationTrack * m_autoTrack;
	QVector<jo_id_t> m_idsToResolve;
	objectVector m_objects;
	timeMap m_timeMap;	// actual values
	timeMap m_oldTimeMap;	// old values for storing the values before setDragValue() is called.
	timeMap m_tangents;	// slope at each point for calculating spline

	float m_tension;
        int   m_waveBank;
        int   m_waveIndex;
	float m_waveRatio;
	float m_waveSkew;
	float m_waveAmplitude;
	float m_waveRepeat;

	bool m_hasAutomation;
	ProgressionTypes m_progressionType;

	bool m_dragging;

	bool m_isRecording;
	float m_lastRecordedValue;

	static int s_quantization;

	static const float DEFAULT_MIN_VALUE;
	static const float DEFAULT_MAX_VALUE;

	friend class AutomationPatternView;

} ;


#endif
