/*
 * AutomatableModel.h - declaration of class AutomatableModel
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
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
#include "MemoryManager.h"
#include "MidiTime.h"
#include "Model.h"
#include "ValueBuffer.h"
#include "lmms_math.h" // REQUIRED

#include <QMap>
#include <QMutex>
#include <QString>

// simple way to map a property of a view to a model
#define mapPropertyFromModelPtr(type, getfunc, setfunc, modelname) \
  public:                                                          \
    type getfunc() const                                           \
    {                                                              \
        return (type)modelname->value();                           \
    }                                                              \
  public slots:                                                    \
    void setfunc(const type val)                                   \
    {                                                              \
        modelname->setValue(val);                                  \
    }

#define mapPropertyFromModel(type, getfunc, setfunc, modelname) \
  public:                                                       \
    type getfunc() const                                        \
    {                                                           \
        return (type)modelname.value();                         \
    }                                                           \
  public slots:                                                 \
    void setfunc(const type val)                                \
    {                                                           \
        modelname.setValue(val);                                \
    }

class AutomatableModel;
class ControllerConnection;
class WaveFormStandard;

typedef QVector<AutomatableModel*>      AutomatableModels;
typedef QMap<AutomatableModel*, real_t> AutomatedValueMap;

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

    INLINE bool isControlled() const
    {
        return m_controllerConnection != nullptr;
    }

    bool isAutomatedOrControlled() const
    {
        return isAutomated() || isControlled();
    }

    ControllerConnection* controllerConnection() const
    {
        return m_controllerConnection;
    }

    void setControllerConnection(ControllerConnection* c);

    template <typename T>
    static T castValue(const real_t v)
    {
        return static_cast<T>(v);
    }

    template <bool>
    static bool castValue(const real_t v)
    {
        return abs(round(v)) >= 0.5;
    }

    template <int>
    static int castValue(const real_t v)
    {
        return int(floor(v));
    }

    // template <real_t>
    static real_t castValue(const real_t v)
    {
        return v;
    }

    template <typename T>
    INLINE T value(f_cnt_t frameOffset = 0, bool recording = false) const
    {
        /*
        if( unlikely( m_controllerConnection != nullptr || hasLinkedModels() )
        )
        {
                return castValue<T>( controllerValue( frameOffset, recording )
        );
        }
        */

        return castValue<T>(recording || m_randomRatio == 0.
                                    ? m_value
                                    : fittedValue(randomizedValue(m_value)));
    }

    // real_t controllerValue( int frameOffset, bool recording ) const;

    //! @brief Function that returns sample-exact data as a ValueBuffer
    //! @return pointer to model's valueBuffer when s.ex.data exists, nullptr
    //! otherwise
    ValueBuffer* valueBuffer();

    // Initial/Reset value

    template <typename T>
    T initValue() const
    {
        return castValue<T>(m_initValue);
    }

    bool isAtInitValue() const
    {
        return m_value == m_initValue;
    }

    void setInitValue(const real_t value);

    // Center

    real_t centerValue() const
    {
        return m_centerValue;
    }

    void setCenterValue(const real_t _value)
    {
        m_centerValue = _value;
    }

    // Range values (min,max,step)

    template <typename T>
    T minValue() const
    {
        return castValue<T>(m_minValue);
    }

    template <typename T>
    T maxValue() const
    {
        return castValue<T>(m_maxValue);
    }

    template <typename T>
    T step() const
    {
        return castValue<T>(m_step);
    }

    virtual void setRange(const real_t min,
                          const real_t max,
                          const real_t step = 1.);

    virtual void setStep(const real_t step);

    real_t range() const
    {
        return m_maxValue - m_minValue;
    }

    bool hasStrictStepSize() const
    {
        return m_hasStrictStepSize;
    }

    void setStrictStepSize(const bool b)
    {
        m_hasStrictStepSize = b;
    }

    // Scale

    void setScaleType(ScaleType sc)
    {
        m_scaleType = sc;
    }
    void setScaleLogarithmic(bool setToTrue = true)
    {
        setScaleType(setToTrue ? Logarithmic : Linear);
    }
    bool isScaleLogarithmic() const
    {
        return m_scaleType == Logarithmic;
    }

    // Randomization

    INLINE real_t randomRatio() const
    {
        return m_randomRatio;
    }

    //! @brief Amount of randomization, between 0 and 1. Default to 0.
    void setRandomRatio(const real_t _ratio);

    //! @brief Distribution of randomization, Default to gaussf().
    void setRandomDistribution(const WaveFormStandard* _wf);

    template <typename T>
    INLINE T rawValue() const
    {
        return castValue<T>(m_value);
    }

    // Various functions

    //! @brief Returns value scaled with the scale type and min/max values of
    //! this model
    real_t scaledValue(real_t value) const;
    //! @brief Returns value applied with the inverse of this model's scale
    //! type
    real_t inverseScaledValue(real_t value) const;

    //! @brief Convert value from min-max to 0-1
    real_t normalizedValue(real_t value) const;
    //! @brief Convert value from 0-1 to min-max
    real_t inverseNormalizedValue(real_t value) const;

    void decrValue(int steps = 1)
    {
        setValue(m_value - steps * m_step);
    }

    void incrValue(int steps = 1)
    {
        setValue(m_value + steps * m_step);
    }

    // Links

    static void linkModels(AutomatableModel* m1, AutomatableModel* m2);
    static void unlinkModels(AutomatableModel* m1, AutomatableModel* m2);

    void unlinkAllModels();

    bool hasLinkedModels() const
    {
        return !m_linkedModels.empty();
    }

    virtual bool hasCableFrom(Model* _m) const;

    /**
     * @brief Saves settings (value, automation links and controller
     * connections) of AutomatableModel into specified DOM element using
     * <name> as attribute/node name
     * @param doc TODO
     * @param element Where this option shall be saved.
     *  Depending on the model, this can be done in an attribute or in a
     * subnode.
     * @param name Name to store this model as.
     */
    virtual void saveSettings(QDomDocument&  doc,
                              QDomElement&   element,
                              const QString& name,
                              const bool     unique = true);

    /*! \brief Loads settings (value, automation links and controller
       connections) of AutomatableModel from specified DOM element using
       <name> as attribute/node name */
    virtual void loadSettings(const QDomElement& element,
                              const QString&     name,
                              const bool         required = true);

    /*! \brief return true if settings are stored under this name */
    virtual bool hasSettings(const QDomElement& element, const QString& name);

    /*! \brief convenient for compatibility, newest name first */
    virtual bool hasSettings(const QDomElement& element,
                             const QString&     name,
                             const QString&     oldName);
    virtual void loadSettings(const QDomElement& element,
                              const QString&     name,
                              const QString&     oldName,
                              const bool         required = true);

    virtual QString nodeName() const
    {
        return "automatablemodel";
    }

    virtual QString        displayValue(const real_t val) const = 0;
    virtual INLINE QString displayValue() const final
    {
        return displayValue(m_value);
    }

    // a way to track changed values in the model and avoid using
    // signals/slots. useful for speed-critical code. note that this method
    // should only be called once per period since it resets the state of the
    // variable - so if your model has to be accessed by more than one object,
    // then this function shouldn't be used.
    bool isValueChanged()
    {
        if(m_valueChanged || valueBuffer())
        {
            m_valueChanged = false;
            return true;
        }
        return false;
    }

    real_t globalAutomationValueAt(const MidiTime& time);

    static long periodCounter();
    static void incrementPeriodCounter();
    static void resetPeriodCounter();
    static void postponeUpdate(AutomatableModel* _m, const ValueBuffer* _vb);

    // tmp
    AutomatableModels m_linkedModels;

  signals:
    void initValueChanged(real_t val);
    void destroyed(jo_id_t id);
    // void controllerValueChanged();

  public slots:
    void setValue(const real_t value);
    void setAutomatedValue(const real_t value);
    void setControlledValue(const real_t value);
    // void setBuffer(const ValueBuffer* _vb);
    void setAutomatedBuffer(const ValueBuffer* _vb);
    void setControlledBuffer(const ValueBuffer* _vb);

    virtual void reset();
    virtual void unlinkControllerConnection();
    // virtual void onControllerValueChanged();

  protected:
    AutomatableModel(const real_t   val                = 0,
                     const real_t   min                = 0,
                     const real_t   max                = 0,
                     const real_t   step               = 0,
                     Model*         parent             = nullptr,
                     const QString& displayName        = QString::null,
                     const QString& objectName         = QString::null,
                     bool           defaultConstructed = false);

    //! returns a value which is in range between min() and
    //! max() and aligned according to the step size (step size 0.05 -> value
    //! 0.12345 becomes 0.10 etc.). You should always call it at the end after
    //! doing your own calculations.
    virtual real_t fittedValue(real_t value) const;
    virtual void   fitValues(ValueBuffer* _vb) const final;

    virtual real_t randomizedValue(real_t value) const;
    virtual void   randomizeValues(ValueBuffer* _vb) const final;

    void propagateValue();
    void propagateAutomatedValue();
    void propagateAutomatedBuffer();

  private:
    // QString formatNumber(real_t v);

    virtual void saveSettings(QDomDocument& doc, QDomElement& element)
    {
        saveSettings(doc, element, "value");
    }

    virtual void loadSettings(const QDomElement& element)
    {
        loadSettings(element, "value");
    }

    void linkModel(AutomatableModel* model, bool propagate);
    void unlinkModel(AutomatableModel* model);

    //! @brief Scales @value from linear to logarithmic.
    //! Value should be within [0,1]
    // template <typename T>
    // T logToLinearScale(T value) const;

    //! rounds @a value to @a where if it is close to it
    //! @param value will be modified to rounded value
    // template <typename T>
    // void roundAt(T& value, const T& where) const;

    ScaleType m_scaleType;  //! scale type, linear by default
    real_t    m_value;
    real_t    m_initValue;
    real_t    m_minValue;
    real_t    m_maxValue;
    real_t    m_step;
    // real_t    m_range;
    real_t m_centerValue;
    real_t m_randomRatio;

    const WaveFormStandard* m_randomDistribution;

    bool m_valueChanged;

    // currently unused?
    real_t m_oldValue;
    int    m_setValueDepth;

    // used to determine if step size should be applied strictly (ie. always)
    // or only when value set from gui (default)
    bool m_hasStrictStepSize;

    //! nullptr if not appended to controller, otherwise connection info
    ControllerConnection* m_controllerConnection;

    ValueBuffer m_valueBuffer;
    long        m_lastUpdatedPeriod;
    static long s_periodCounter;

    bool   m_hasSampleExactData;
    QMutex m_valueBufferMutex;

    static QHash<AutomatableModel*, const ValueBuffer*> s_postponedUpdates;
};

template <typename T>
class EXPORT TypedAutomatableModel : public AutomatableModel
{
  public:
    using AutomatableModel::AutomatableModel;

    INLINE T value(f_cnt_t frameOffset = 0, bool recording = false) const
    {
        return AutomatableModel::value<T>(frameOffset, recording);
    }

    INLINE T initValue() const
    {
        return AutomatableModel::initValue<T>();
    }

    INLINE T rawValue() const
    {
        return AutomatableModel::rawValue<T>();
    }

    INLINE T minValue() const
    {
        return AutomatableModel::minValue<T>();
    }

    INLINE T maxValue() const
    {
        return AutomatableModel::maxValue<T>();
    }

    INLINE T step() const
    {
        return AutomatableModel::step<T>();
    }
};

// some typed AutomatableModel-definitions

class EXPORT RealModel : public TypedAutomatableModel<real_t>
{
  public:
    RealModel(real_t         val                = 0.,
              real_t         min                = 0.,
              real_t         max                = 0.,
              real_t         step               = 0.,
              Model*         parent             = nullptr,
              const QString& displayName        = "[real model]",
              const QString& objectName         = QString::null,
              bool           defaultConstructed = false);
    real_t getRoundedValue() const;
    int    getDigitCount() const;

    void    setRange(const real_t min,
                     const real_t max,
                     const real_t step = 1.) override;
    void    setStep(const real_t step) override;
    QString displayValue(const real_t val) const override;

    static RealModel* createDefaultConstructed(const QString& _displayName
                                               = QString::null)
    {
        const QString& s = _displayName.isEmpty()
                                   ? "[default real model]"
                                   : _displayName + " [default]";
        return new RealModel(0., 0., 1., 0.01, nullptr, s, QString::null,
                             true);
    }

  protected:
    void setDigitCount();

    int m_digitCount;
};

#define FloatModel RealModel

class EXPORT IntModel : public TypedAutomatableModel<int>
{
  public:
    IntModel(int            val                = 0,
             int            min                = 0,
             int            max                = 0,
             Model*         parent             = nullptr,
             const QString& displayName        = "[int model]",
             const QString& objectName         = QString::null,
             bool           defaultConstructed = false) :
          TypedAutomatableModel(val,
                                min,
                                max,
                                1,
                                parent,
                                displayName,
                                objectName,
                                defaultConstructed)
    {
        setStrictStepSize(true);
    }

    QString displayValue(const real_t val) const override;

    static IntModel* createDefaultConstructed(const QString& _displayName
                                              = QString::null)
    {
        const QString& s = _displayName.isEmpty()
                                   ? "[default int model]"
                                   : _displayName + " [default]";
        return new IntModel(0, 0, 10, nullptr, s, QString::null, true);
    }
};

class EXPORT BoolModel : public TypedAutomatableModel<bool>
{
  public:
    BoolModel(const bool     val                = false,
              Model*         parent             = nullptr,
              const QString& displayName        = "[bool model]",
              const QString& objectName         = QString::null,
              bool           defaultConstructed = false) :
          TypedAutomatableModel(val,
                                false,
                                true,
                                1,
                                parent,
                                displayName,
                                objectName,
                                defaultConstructed)
    {
        setStrictStepSize(true);
    }

    virtual QString displayValue(const real_t val) const override;

    static BoolModel* createDefaultConstructed(const QString& _displayName
                                               = QString::null)
    {
        const QString& s = _displayName.isEmpty()
                                   ? "[default bool model]"
                                   : _displayName + " [default]";
        return new BoolModel(false, nullptr, s, QString::null, true);
    }
};

class EXPORT DiscreteIntModel : public IntModel
{
  public:
    DiscreteIntModel(int            val,
                     QVector<int>&  set,
                     Model*         parent      = nullptr,
                     const QString& displayName = "[discrete int model]",
                     const QString& objectName  = QString::null,
                     bool           defaultConstructed = false) :
          IntModel(val,
                   val,
                   val,
                   parent,
                   displayName,
                   objectName,
                   defaultConstructed)
    {
        setSet(set);
        setValue(val);
    }

    void setSet(QVector<int>& _set)
    {
        m_set = _set;
        if(m_set.isEmpty())
            insertValue(initValue());
        setRange(m_set.first(), m_set.last());
    }

    void insertValue(int _value)
    {
        if(!m_set.contains(_value))
        {
            m_set.append(_value);
            qSort(m_set.begin(), m_set.end());
            setRange(m_set.first(), m_set.last());
        }
    }

    virtual real_t fittedValue(real_t value) const override
    {
        value = IntModel::fittedValue(value);

        int v = IntModel::castValue(value);
        if(m_set.contains(v))
            return v;

        if(v <= m_set.first())
            return m_set.first();
        if(v >= m_set.last())
            return m_set.last();

        int i;
        for(i = m_set.size() - 1; i >= 0; i--)
            if(v >= m_set.at(i))
                break;
        return m_set.at(i);
    }

    virtual QString displayValue(const real_t _val) const override
    {
        return IntModel::displayValue(_val);
    }

  private:
    QVector<int> m_set;
};

#endif
