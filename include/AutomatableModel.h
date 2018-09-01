/*
 * AutomatableModel.h - declaration of class AutomatableModel
 *
 * Copyright (c) 2007-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef AUTOMATABLE_MODEL_H
#define AUTOMATABLE_MODEL_H

#include "JournallingObject.h"
#include "Model.h"
#include "MidiTime.h"
#include "ValueBuffer.h"
#include "MemoryManager.h"

#include <QMap>
#include <QMutex>

// simple way to map a property of a view to a model
#define mapPropertyFromModelPtr(type,getfunc,setfunc,modelname)	\
		public:													\
			type getfunc() const								\
			{													\
				return (type) modelname->value();				\
			}													\
		public slots:											\
			void setfunc( const type val )						\
			{													\
				modelname->setValue( val );						\
			}

#define mapPropertyFromModel(type,getfunc,setfunc,modelname)	\
		public:													\
			type getfunc() const								\
			{													\
				return (type) modelname.value();				\
			}													\
		public slots:											\
			void setfunc( const type val )						\
			{													\
				modelname.setValue( val );				\
			}



class ControllerConnection;

class EXPORT AutomatableModel : public Model, public JournallingObject
{
	Q_OBJECT
	MM_OPERATORS
public:
	enum ScaleType
	{
		Linear,
		Logarithmic,
		Decibel
	};

	virtual ~AutomatableModel();


	bool isAutomated() const;

	inline bool isControlled() const
	{
		return m_controllerConnection!=NULL;
	}

	bool isAutomatedOrControlled() const
	{
		return isAutomated() || isControlled();
	}

	ControllerConnection* controllerConnection() const
	{
		return m_controllerConnection;
	}


	void setControllerConnection( ControllerConnection* c );


	template<class T>
	static T castValue( const float v )
	{
		return (T)( v );
	}

	template<bool>
	static bool castValue( const float v )
	{
		return ( qRound( v ) != 0 );
	}


	template<class T>
        inline T value( int frameOffset = 0, bool recording = false ) const
	{
                /*
                if( unlikely( m_controllerConnection != NULL || hasLinkedModels() ) )
		{
			return castValue<T>( controllerValue( frameOffset, recording ) );
		}
                */

		return castValue<T>( m_value );
	}

	//float controllerValue( int frameOffset, bool recording ) const;

	//! @brief Function that returns sample-exact data as a ValueBuffer
	//! @return pointer to model's valueBuffer when s.ex.data exists, NULL otherwise
	ValueBuffer* valueBuffer();

	template<class T>
	T initValue() const
	{
		return castValue<T>( m_initValue );
	}

	bool isAtInitValue() const
	{
		return m_value == m_initValue;
	}

	template<class T>
	T minValue() const
	{
		return castValue<T>( m_minValue );
	}

	template<class T>
	T maxValue() const
	{
		return castValue<T>( m_maxValue );
	}

	template<class T>
	T step() const
	{
		return castValue<T>( m_step );
	}

	//! @brief Returns value scaled with the scale type and min/max values of this model
	float scaledValue( float value ) const;
	//! @brief Returns value applied with the inverse of this model's scale type
	float inverseScaledValue( float value ) const;

	//! @brief Convert value from min-max to 0-1
	float normalizedValue( float value ) const;
	//! @brief Convert value from 0-1 to min-max
	float inverseNormalizedValue( float value ) const;

	void setInitValue( const float value );

	void decrValue( int steps = 1)
	{
		setValue( m_value - steps * m_step );
	}

	void incrValue( int steps = 1)
	{
		setValue( m_value + steps * m_step );
	}

	float range() const
	{
		return m_range;
	}

	virtual void setRange( const float min, const float max, const float step = 1.f );

	virtual void setStep( const float step );

	void setScaleType( ScaleType sc ) {
		m_scaleType = sc;
	}
	void setScaleLogarithmic( bool setToTrue = true )
	{
		setScaleType( setToTrue ? Logarithmic : Linear );
	}
	bool isScaleLogarithmic() const
	{
		return m_scaleType == Logarithmic;
	}

	float centerValue() const
	{
		return m_centerValue;
	}

	void setCenterValue( const float centerVal )
	{
		m_centerValue = centerVal;
	}

	static void linkModels( AutomatableModel* m1, AutomatableModel* m2 );
	static void unlinkModels( AutomatableModel* m1, AutomatableModel* m2 );

	void unlinkAllModels();

	/**
	 * @brief Saves settings (value, automation links and controller connections) of AutomatableModel into
	 *  specified DOM element using <name> as attribute/node name
	 * @param doc TODO
	 * @param element Where this option shall be saved.
	 *  Depending on the model, this can be done in an attribute or in a subnode.
	 * @param name Name to store this model as.
	 */
	virtual void saveSettings( QDomDocument& doc, QDomElement& element, const QString& name );

	/*! \brief Loads settings (value, automation links and controller connections) of AutomatableModel from
				specified DOM element using <name> as attribute/node name */
	virtual void loadSettings( const QDomElement& element,
                                   const QString& name,
                                   const bool required = true);

	virtual QString nodeName() const
	{
		return "automatablemodel";
	}

	virtual QString displayValue( const float val ) const = 0;
	virtual inline QString displayValue() const final { return displayValue(m_value); }

	bool hasLinkedModels() const
	{
		return !m_linkedModels.empty();
	}

	// a way to track changed values in the model and avoid using signals/slots.
        // useful for speed-critical code.
	// note that this method should only be called once per period since it resets the
        // state of the variable - so if your model has to be accessed by more than one
        // object, then this function shouldn't be used.
	bool isValueChanged()
	{
		if( m_valueChanged || valueBuffer() )
		{
			m_valueChanged = false;
			return true;
		}
		return false;
	}

	float globalAutomationValueAt( const MidiTime& time );

	bool hasStrictStepSize() const
	{
		return m_hasStrictStepSize;
	}

	void setStrictStepSize( const bool b )
	{
		m_hasStrictStepSize = b;
	}

	static void incrementPeriodCounter()
	{
		++s_periodCounter;
	}

	static void resetPeriodCounter()
	{
		s_periodCounter = 0;
	}

        //tmp
	QVector<AutomatableModel *> m_linkedModels;


signals:
	void initValueChanged( float val );
	void destroyed( jo_id_t id );
        //void controllerValueChanged();


public slots:
	void setValue( const float value );
	void setAutomatedValue( const float value );
	void setControlledValue( const float value );
        //void setBuffer(const ValueBuffer* _vb);
        void setAutomatedBuffer(const ValueBuffer* _vb);
        void setControlledBuffer(const ValueBuffer* _vb);
	virtual void reset();
	virtual void unlinkControllerConnection();
        //virtual void onControllerValueChanged();


 protected:
	AutomatableModel(const float val = 0,
                         const float min = 0,
                         const float max = 0,
                         const float step = 0,
                         Model* parent = NULL,
                         const QString& displayName = QString(),
                         bool defaultConstructed = false );

        //! returns a value which is in range between min() and
	//! max() and aligned according to the step size (step size 0.05 -> value
	//! 0.12345 becomes 0.10 etc.). You should always call it at the end after
	//! doing your own calculations.
	float fittedValue( float value ) const;

        void propagateValue();
        void propagateAutomatedValue();


private:
	virtual void saveSettings( QDomDocument& doc, QDomElement& element )
	{
		saveSettings( doc, element, "value" );
	}

	virtual void loadSettings( const QDomElement& element )
	{
		loadSettings( element, "value" );
	}

	void linkModel( AutomatableModel* model, bool propagate );
	void unlinkModel( AutomatableModel* model );

	//! @brief Scales @value from linear to logarithmic.
	//! Value should be within [0,1]
	template<class T> T logToLinearScale( T value ) const;

	//! rounds @a value to @a where if it is close to it
	//! @param value will be modified to rounded value
	template<class T> void roundAt( T &value, const T &where ) const;


	ScaleType m_scaleType; //! scale type, linear by default
	float m_value;
	float m_initValue;
	float m_minValue;
	float m_maxValue;
	float m_step;
	float m_range;
	float m_centerValue;

	bool m_valueChanged;

	// currently unused?
	float m_oldValue;
	int m_setValueDepth;

	// used to determine if step size should be applied strictly (ie. always)
	// or only when value set from gui (default)
	bool m_hasStrictStepSize;


	//! NULL if not appended to controller, otherwise connection info
	ControllerConnection* m_controllerConnection;


	ValueBuffer m_valueBuffer;
	long m_lastUpdatedPeriod;
	static long s_periodCounter;

	bool m_hasSampleExactData;

	// prevent several threads from attempting to write the same vb at
        // the same time. Still useful with signals?
	QMutex m_valueBufferMutex;
};


template <typename T> class EXPORT TypedAutomatableModel :
public AutomatableModel
{
 public:
	using AutomatableModel::AutomatableModel;
	T value( int frameOffset = 0 ) const
	{
		return AutomatableModel::value<T>( frameOffset );
	}

	T initValue() const
	{
		return AutomatableModel::initValue<T>();
	}

	T minValue() const
	{
		return AutomatableModel::minValue<T>();
	}

	T maxValue() const
	{
		return AutomatableModel::maxValue<T>();
	}
};


// some typed AutomatableModel-definitions

class FloatModel : public TypedAutomatableModel<float>
{
public:
	FloatModel( float val = 0.f, float min = 0.f, float max = 0.f,
                    float step = 0.f,
                    Model * parent = NULL,
                    const QString& displayName = QString(),
                    bool defaultConstructed = false );
	float getRoundedValue() const;
	int getDigitCount() const;

	virtual void setRange( const float min, const float max,
                               const float step = 1.f ) override;
	virtual void setStep( const float step ) override;
	virtual QString displayValue( const float val ) const override;

 protected:
        void setDigitCount();

        int m_digitCount;
};


class IntModel : public TypedAutomatableModel<int>
{
public:
	IntModel( int val = 0, int min = 0, int max = 0,
                  Model* parent = NULL,
                  const QString& displayName = QString(),
                  bool defaultConstructed = false ) :
        TypedAutomatableModel( val, min, max, 1, parent, displayName, defaultConstructed )
	{
                setStrictStepSize( true );
	}
	virtual QString displayValue( const float val ) const override;
};


class BoolModel : public TypedAutomatableModel<bool>
{
public:
	BoolModel( const bool val = false,
                   Model* parent = NULL,
                   const QString& displayName = QString(),
                   bool defaultConstructed = false ) :
        TypedAutomatableModel( val, false, true, 1, parent, displayName, defaultConstructed )
        {
                setStrictStepSize( true );
	}
	virtual QString displayValue( const float val ) const override;
};

typedef QMap<AutomatableModel*, float> AutomatedValueMap;

#endif

