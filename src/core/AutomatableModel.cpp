/*
 * AutomatableModel.cpp - some implementations of AutomatableModel-class
 *
 * Copyright (c) 2017-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "AutomatableModel.h"

#include "AutomationPattern.h"
#include "Backtrace.h"
#include "ControllerConnection.h"
#include "Engine.h"
#include "Mixer.h"
#include "ProjectJournal.h"
#include "Song.h"
#include "WaveFormStandard.h"
#include "debug.h"
#include "lmms_math.h"  // REQUIRED

#include <QTimer>

long AutomatableModel::s_periodCounter = 0;

AutomatableModel::AutomatableModel(const real_t   val,
                                   const real_t   min,
                                   const real_t   max,
                                   const real_t   step,
                                   Model*         parent,
                                   const QString& displayName,
                                   const QString& objectName,
                                   bool           defaultConstructed) :
      Model(parent, displayName, objectName, defaultConstructed),
      m_scaleType(Linear), m_minValue(min), m_maxValue(max), m_step(step),
      m_centerValue(m_minValue), m_randomRatio(0.),
      m_randomDistribution(nullptr), m_valueChanged(false),
      m_setValueDepth(0), m_hasStrictStepSize(false),
      m_controllerConnection(nullptr),
      m_valueBuffer(static_cast<int>(Engine::mixer()->framesPerPeriod())),
      m_lastUpdatedPeriod(-1), m_hasSampleExactData(false)

{
    m_value = fittedValue(val);
    setInitValue(val);
}

AutomatableModel::~AutomatableModel()
{
    /*
      while(!m_linkedModels.empty())
      {
      m_linkedModels.last()->unlinkModel(this);
      m_linkedModels.erase(m_linkedModels.end() - 1);
      }
    */
    for(AutomatableModel* m: m_linkedModels)
        m->unlinkModel(this);
    m_linkedModels.clear();

    if(m_controllerConnection)
    {
        delete m_controllerConnection;
    }

    m_valueBuffer.clear();

    emit destroyed(id());
}

bool AutomatableModel::isAutomated() const
{
    return AutomationPattern::isAutomated(this);
}

/*
QString AutomatableModel::formatNumber(real_t v)
{
#ifdef REAL_IS_FLOAT
    QString r = QString::number(v, 'f', 9);
#endif
#ifdef REAL_IS_DOUBLE
    QString r = QString::number(v, 'f', 17);
#endif
    if(r.indexOf(QChar('.')) >= 0)
    {
        r.replace(QRegExp("0*$"), "");
        r.replace(QRegExp("[.]$"), "");
    }
    return r;
}
*/

void AutomatableModel::saveSettings(QDomDocument&  doc,
                                    QDomElement&   element,
                                    const QString& name,
                                    const bool     unique)
{
    QRegExp reg1("^[A-Za-z_][A-Za-z0-9._-]*$");
    QRegExp reg2("^xml", Qt::CaseInsensitive);
    bool    mustQuote = (reg1.indexIn(name) < 0) || (reg2.indexIn(name) >= 0);

    if(mustQuote || isAutomated() || m_scaleType != Linear
       || m_randomRatio != 0. || hasLinkedModels())
    {
        // TODO: check for unicity
#ifdef DEBUG_LMMS
        if(unique)
        {
            if(mustQuote)
            {
                bool warning = false;

                QDomElement e = element.firstChildElement(
                        QString("automatablemodel"));
                while(!e.isNull())
                {
                    if(e.hasAttribute("nodename")
                       && e.attribute("nodename") == name)
                    {
                        warning = true;
                        break;
                    }
                }

                if(warning)
                    qWarning(
                            "AutomatableModel: suspicious multiple element "
                            "'automatablemodel/%s'",
                            qPrintable(name));
            }
            else
            {
                if(!element.elementsByTagName(name).isEmpty())
                    qWarning(
                            "AutomatableModel: suspicious multiple element "
                            "'%s'",
                            qPrintable(name));
            }
        }
#endif

        // automation needs tuple of data (name, id, value)
        // scale type also needs an extra value
        // => it must be appended as a node
        QDomElement me = doc.createElement(
                mustQuote ? QString("automatablemodel") : name);

        element.appendChild(me);
        me.setAttribute("id", ProjectJournal::idToSave(id()));
        me.setAttribute("value", formatNumber(m_value));
        me.setAttribute("scale_type",
                        m_scaleType == Logarithmic ? "log" : "linear");

        me.setAttribute("random_ratio", m_randomRatio);
        // TODO: randomDistribution

        if(mustQuote)
            me.setAttribute("nodename", name);

        if(hasLinkedModels())
        {
            if(!hasUuid())
                qWarning(
                        "AutomatableModel::saveSettings Strange: '%s' has "
                        "linked models but no uuid.",
                        qPrintable(fullDisplayName()));

            QDomElement linksElement = doc.createElement("links");
            me.appendChild(linksElement);
            for(AutomatableModel* m: m_linkedModels)
            {
                QDomElement linkElement = doc.createElement("link");
                linksElement.appendChild(linkElement);
                linkElement.setAttribute("target", m->uuid());
                // other properties will come here
            }
        }

        if(hasUuid())
            me.setAttribute("uuid", uuid());
    }
    else
    {
#ifdef DEBUG_LMMS
        if(unique && !element.elementsByTagName(name).isEmpty())
            qWarning("AutomatableModel: suspicious multiple element '%s'",
                     qPrintable(name));
#endif

        // non automation, linear scale (default), can be saved as
        // attribute
        element.setAttribute(name, formatNumber(m_value));
    }

    if(m_controllerConnection
       && m_controllerConnection->getController()->type()
                  != Controller::DummyController)
    {
        QDomElement controllerElement;

        // get "connection" element (and create it if needed)
        QDomNode node = element.namedItem("connection");
        if(node.isElement())
        {
            controllerElement = node.toElement();
        }
        else
        {
            controllerElement = doc.createElement("connection");
            element.appendChild(controllerElement);
        }

        QDomElement connectionElement = doc.createElement(name);
        m_controllerConnection->saveSettings(doc, connectionElement);
        controllerElement.appendChild(connectionElement);
    }
}

void AutomatableModel::loadSettings(const QDomElement& element,
                                    const QString&     name,
                                    const bool         required)
{
    if(element.isNull())
    {
        BACKTRACE
        qWarning("AutomatableModel::loadSettings ERROR element is null");
    }

    if(name.isEmpty())
    {
        BACKTRACE
        qWarning("AutomatableModel::loadSettings ERROR name is empty");
    }

    // compat code
    QDomNode node = element.namedItem(AutomationPattern::classNodeName());
    if(node.isElement())
    {
        node = node.namedItem(name);
        if(node.isElement())
        {
            AutomationPattern* p
                    = AutomationPattern::globalAutomationPattern(this);
            p->loadSettings(node.toElement());
            setValue(p->valueAt(0));
            // in older projects we sometimes have odd automations
            // with just one value in - eliminate if necessary
            if(!p->hasAutomation())
            {
                delete p;
            }
            return;
        }
        // logscales were not existing at this point of time
        // so they can be ignored
    }

    QDomNode connectionNode = element.namedItem("connection");
    // reads controller connection
    if(connectionNode.isElement())
    {
        QDomNode thisConnection = connectionNode.toElement().namedItem(name);
        if(thisConnection.isElement())
        {
            setControllerConnection(
                    new ControllerConnection((Controller*)nullptr));
            m_controllerConnection->loadSettings(thisConnection.toElement());
            // m_controllerConnection->setTargetName( displayName() );
        }
    }

    node = element.namedItem(name);

    if(node.isNull())
    {
        // PR 4605
        QDomElement e
                = element.firstChildElement(QString("automatablemodel"));
        while(!e.isNull())
        {
            if(!e.hasAttribute("nodename") || e.attribute("nodename") != name)
            {
                e = e.nextSiblingElement();
                continue;
            }

            node = e;
            break;
        }
    }

    // models can be stored as elements (port00) or attributes (port10):
    // <ladspacontrols port10="4.41">
    //   <port00 value="4.41" id="4249278"/>
    // </ladspacontrols>
    // element => there is automation data, or scaletype information

    if(node.isElement())
    {
        QDomElement me = node.toElement();

        if(me.hasAttribute("uuid"))
            setUuid(me.attribute("uuid"));

        changeID(me.attribute("id").toInt());

        QString v = me.attribute("value");
        v.replace(',', '.');
#ifdef REAL_IS_FLOAT
        setValue(v.toFloat());
#endif
#ifdef REAL_IS_DOUBLE
        setValue(v.toDouble());
#endif

        if(me.hasAttribute("scale_type"))
        {
            if(me.attribute("scale_type") == "linear")
            {
                setScaleType(Linear);
            }
            else if(me.attribute("scale_type") == "log")
            {
                setScaleType(Logarithmic);
            }
        }

        QDomNode linksNode = me.namedItem("links");
        if(linksNode.isElement())
        {
            QDomElement linkElement
                    = linksNode.toElement().firstChildElement("link");
            while(!linkElement.isNull())
            {
                QString target = linkElement.attribute("target");

                AutomatableModel* m = dynamic_cast<AutomatableModel*>(
                        Model::find(target));
                if(m != nullptr)
                {
                    linkModels(this, m);
                    qInfo("AutomatableModel::loadSettings target found '%s' "
                          "for '%s'",
                          qPrintable(m->fullDisplayName()),
                          qPrintable(fullDisplayName()));
                }
                else
                {
                    qInfo("AutomatableModel::loadSettings target NOT found "
                          "'%s' for '%s'",
                          qPrintable(target), qPrintable(fullDisplayName()));
                }
                linkElement = linkElement.nextSiblingElement();
            }
        }
    }
    else
    {
        if(element.hasAttribute("uuid"))
            setUuid(element.attribute("uuid"));

        setScaleType(Linear);

        if(element.hasAttribute(name))
        // attribute => read the element's value from the attribute list
        {
            QString v = element.attribute(name);
            v.replace(',', '.');
#ifdef REAL_IS_FLOAT
            setInitValue(v.toFloat());
#endif
#ifdef REAL_IS_DOUBLE
            setInitValue(v.toDouble());
#endif
        }
        else
        {
            if(required)
            {
                // BACKTRACE
                qWarning(
                        "AutomatableModel: missing value for "
                        "attribute '%s' in element '%s'",
                        qPrintable(name), qPrintable(element.tagName()));
            }
            reset();
        }
    }
}

void AutomatableModel::setRandomRatio(const real_t _ratio)
{
    real_t rr = bound(0., abs(_ratio), 1.);
    if(rr != 0. && m_randomDistribution == nullptr)
        m_randomDistribution = WaveFormStandard::SHARPGAUSS;
    m_randomRatio = rr;
    emit propertiesChanged();
}

void AutomatableModel::setValue(const real_t value)
{
    const real_t oldval = m_value;
    const real_t newval = fittedValue(value);

    if(oldval != newval)
    {
        m_oldValue     = m_value;
        m_value        = newval;
        m_valueChanged = true;
        emit dataChanged();
        propagateValue();
    }
    else
    {
        emit dataUnchanged();
    }
}

void AutomatableModel::propagateValue()
{
    ++m_setValueDepth;

    // add changes to history so user can undo it
    // addJournalCheckPoint();

    // notify linked models
    for(QVector<AutomatableModel*>::Iterator it = m_linkedModels.begin();
        it != m_linkedModels.end(); ++it)
    {
        if((*it)->m_setValueDepth < 1
           && (*it)->fittedValue(m_value) != (*it)->m_value)
        {
            bool journalling = (*it)->testAndSetJournalling(isJournalling());
            (*it)->setValue(m_value);
            (*it)->setJournalling(journalling);
        }
    }
    // m_valueChanged = true;
    --m_setValueDepth;
}

void AutomatableModel::setAutomatedValue(const real_t value)
{
    const real_t oldval = m_value;
    const real_t newval = fittedValue(scaledValue(value));

    if(oldval != newval)
    {
        m_oldValue     = m_value;
        m_value        = newval;
        m_valueChanged = true;
        propagateAutomatedValue();
        emit dataChanged();
    }
    /*
    else
    {
            emit dataUnchanged();
    }

    m_oldValue = m_value;
    ++m_setValueDepth;

    const real_t newValue=fittedValue(scaledValue( value ));
    if( m_value != newvalue )
    {
            // notify linked models
            for( QVector<AutomatableModel *>::Iterator it =
    m_linkedModels.begin(); it != m_linkedModels.end(); ++it )
            {
                    if( (*it)->m_setValueDepth < 1 &&
                            (*it)->fittedValue( m_value ) != (*it)->m_value )
                    {
                            (*it)->setAutomatedValue( value );
                    }
            }
            //m_valueChanged = true;
            emit dataChanged();
    }
    --m_setValueDepth;
    */
}

void AutomatableModel::setControlledValue(const real_t value)
{
    setAutomatedValue(inverseNormalizedValue(value));
}

void AutomatableModel::propagateAutomatedValue()
{
    ++m_setValueDepth;
    // notify linked models
    for(QVector<AutomatableModel*>::Iterator it = m_linkedModels.begin();
        it != m_linkedModels.end(); ++it)
    {
        if((*it)->m_setValueDepth < 1
           && (*it)->fittedValue(m_value) != (*it)->m_value)
        {
            (*it)->setAutomatedValue(m_value);
        }
    }
    --m_setValueDepth;
}

void AutomatableModel::propagateAutomatedBuffer()
{
    if(!m_hasSampleExactData)
        return;

    ++m_setValueDepth;
    // notify linked models
    for(QVector<AutomatableModel*>::Iterator it = m_linkedModels.begin();
        it != m_linkedModels.end(); ++it)
    {
        if((*it)->m_setValueDepth < 1)
        {
            (*it)->setAutomatedBuffer(&m_valueBuffer);
        }
    }
    setAutomatedValue(m_valueBuffer.value(0));
    --m_setValueDepth;
}

/*
template <class T>
T AutomatableModel::logToLinearScale(T value) const
{
    return castValue<T>(::logToLinearScale(minValue<real_t>(),
                                           maxValue<real_t>(),
                                           static_cast<real_t>(value)));
}
*/

real_t AutomatableModel::scaledValue(real_t value) const
{
    return m_scaleType == Linear
                   ? value
                   : ::logToLinearScale(minValue<real_t>(),
                                        maxValue<real_t>(), value);
    //: logToLinearScale<real_t>((value - minValue<real_t>()) / range());
}

real_t AutomatableModel::inverseScaledValue(real_t value) const
{
    return m_scaleType == Linear
                   ? value
                   : ::linearToLogScale(minValue<real_t>(),
                                        maxValue<real_t>(), value);
}

real_t AutomatableModel::normalizedValue(real_t value) const
{
    real_t r = value;
    if(m_scaleType != Linear)
        r = ::logToLinearScale(minValue<real_t>(), maxValue<real_t>(), r);
    r = (value - minValue<real_t>()) / range();
    return bound(0., r, 1.);
}

real_t AutomatableModel::inverseNormalizedValue(real_t value) const
{
    real_t r = bound(0., value, 1.) * range() + minValue<real_t>();
    if(m_scaleType != Linear)
        r = ::linearToLogScale(minValue<real_t>(), maxValue<real_t>(), r);
    return r;
}

/*
//! @todo: this should be moved into a maths header
template <class T>
void roundAt(T& value, const T& where, const T& step_size)
{
    // SILENCE
    if(qAbs<T>(value - where) <= T(0.001) * qAbs<T>(step_size))
    {
        value = where;
    }
}

template <class T>
void AutomatableModel::roundAt(T& value, const T& where) const
{
    ::roundAt(value, where, m_step);
}
*/

void AutomatableModel::setRange(const real_t min,
                                const real_t max,
                                const real_t step)
{
    if((m_maxValue != max) || (m_minValue != min) || (m_step != step))
    {
        m_minValue = min;
        m_maxValue = max;
        m_step     = step;

        if(m_minValue > m_maxValue)
            qSwap<real_t>(m_minValue, m_maxValue);

        // re-adjust value
        setValue(m_value);

        emit propertiesChanged();
    }
}

void AutomatableModel::setStep(const real_t step)
{
    if(m_step != step)
    {
        m_step = step;
        emit propertiesChanged();
    }
}

real_t AutomatableModel::fittedValue(real_t value) const
{
    value = bound(m_minValue, value, m_maxValue);

    if(m_step != real_t(0.))
    {
        real_t magnet;
        if(m_hasStrictStepSize)
        {
            // nearbyintf
            value = m_minValue
                    + floor((value - m_minValue) / m_step) * m_step;
            magnet = m_step / 2.;
        }
        else
            magnet = m_step / 1000.;

        /*
        roundAt<real_t>(value, m_initValue);
        roundAt<real_t>(value, m_centerValue);
        roundAt<real_t>(value, m_maxValue);
        roundAt<real_t>(value, m_minValue);
        roundAt<real_t>(value, 0.);
        */
        roundat(value, m_initValue, magnet);
        roundat(value, m_centerValue, magnet);
        roundat(value, m_maxValue, magnet);
        roundat(value, m_minValue, magnet);
        roundat(value, 0., magnet);

        value = bound(m_minValue, value, m_maxValue);
    }

    return value;
}

void AutomatableModel::fitValues(ValueBuffer* _vb) const
{
    // TODO: this loop could be improved using pointer arithmetic
    real_t* v = _vb->values();
    for(int i = _vb->length() - 1; i >= 0; --i)
        v[i] = fittedValue(v[i]);
}

real_t AutomatableModel::randomizedValue(real_t) const
{
    real_t r = m_value;
    if(m_randomRatio != 0.)
    {
        const real_t            rr = m_randomRatio * range();
        const WaveFormStandard* d  = m_randomDistribution;

        real_t x = 2. * fastrand01inc() - 1.;
        if(d != nullptr)
            x *= d->f(abs(x));
        r += rr * x;
    }
    return r;
}

void AutomatableModel::randomizeValues(ValueBuffer* _vb) const
{
    if(m_randomRatio != 0.)
    {
        const real_t            rr = m_randomRatio * range();
        const WaveFormStandard* d  = m_randomDistribution;

        // TODO: this loop could be improved using pointer arithmetic
        real_t* v = _vb->values();
        if(d != nullptr)
        {
            for(int i = _vb->length() - 1; i >= 0; --i)
            {
                const real_t x = 2. * fastrand01inc() - 1.;
                v[i] += rr * x * d->f(abs(x));
            }
        }
        else
        {
            for(int i = _vb->length() - 1; i >= 0; --i)
            {
                const real_t x = 2. * fastrand01inc() - 1.;
                v[i] += rr * x;
            }
        }
    }
}

void AutomatableModel::linkModel(AutomatableModel* model, bool propagate)
{
    if(!hasUuid())
        uuid();

    if(model && !m_linkedModels.contains(model) && model != this)
    {
        if(!model->hasUuid())
            model->uuid();

        m_linkedModels.append(model);

        if(!model->hasLinkedModels())  // ???
        {
            connect(this, SIGNAL(dataChanged()), model,
                    SIGNAL(dataChanged()));
            // connect( this, SIGNAL( controllerValueChanged(real_t) ),
            //         model, SLOT( setControlledValue(real_t) ) );
        }

        if(propagate)
            propagateValue();

        model->emit propertiesChanged();
        emit        propertiesChanged();
    }
}

void AutomatableModel::unlinkModel(AutomatableModel* model)
{
    if(model && m_linkedModels.contains(model) && model != this)
    {
        disconnect(model);
        emit propertiesChanged();
        emit model->propertiesChanged();

        QVector<AutomatableModel*>::Iterator it
                = qFind(m_linkedModels.begin(), m_linkedModels.end(), model);
        if(it != m_linkedModels.end())
        {
            m_linkedModels.erase(it);
        }
    }
}

void AutomatableModel::linkModels(AutomatableModel* model1,
                                  AutomatableModel* model2)
{
    model1->linkModel(model2, true);
    model2->linkModel(model1, false);
}

void AutomatableModel::unlinkModels(AutomatableModel* model1,
                                    AutomatableModel* model2)
{
    model1->unlinkModel(model2);
    model2->unlinkModel(model1);
}

void AutomatableModel::unlinkAllModels()
{
    for(AutomatableModel* model: m_linkedModels)
    {
        unlinkModels(this, model);
    }
}

bool AutomatableModel::hasCableFrom(Model* _m) const
{
    AutomatableModel* am = dynamic_cast<AutomatableModel*>(_m);
    if(am != nullptr && m_linkedModels.contains(am)
       && fullDisplayName() < am->fullDisplayName())
        return true;

    Controller* cm = m_controllerConnection != nullptr
                             ? m_controllerConnection->getController()
                             : nullptr;
    if(cm != nullptr && cm->type() != Controller::DummyController
       && cm == _m)  //>hasModel(_m))
        return true;

    return false;
}

void AutomatableModel::setControllerConnection(ControllerConnection* c)
{
    m_controllerConnection = c;
    if(c)
    {
        connect(m_controllerConnection,
                SIGNAL(controlledValueChanged(real_t)), this,
                SLOT(setControlledValue(real_t)));
        connect(m_controllerConnection,
                SIGNAL(controlledBufferChanged(const ValueBuffer*)), this,
                SLOT(setControlledBuffer(const ValueBuffer*)));
        connect(m_controllerConnection, SIGNAL(destroyed()), this,
                SLOT(unlinkControllerConnection()));
        // m_valueChanged = true;
        emit propertiesChanged();
        emit dataChanged();
    }
}

/*
void AutomatableModel::onControllerValueChanged()
{
        //setValue(controllerValue(0,false));
        //qInfo("AutomatableModel::onControllerValueChanged");
        emit controllerValueChanged();
        //emit dataChanged();

        //if( m_controllerConnection &&
        //    m_controllerConnection->getController() )
        if( hasLinkedModels() )
        {
                for(AutomatableModel* m: m_linkedModels)
                        m->emit dataChanged();
        }
}
*/

/*
real_t AutomatableModel::controllerValue( int frameOffset, bool recording )
const
{
        if( m_controllerConnection &&
            (recording || m_controllerConnection->hasChanged() ))
        {
                real_t v = 0.;

                switch(m_scaleType)
                {
                case Linear:
                        v = minValue<real_t>() + ( range() *
controllerConnection()->currentValue( frameOffset ) ); break; case
Logarithmic: v = logToLinearScale( controllerConnection()->currentValue(
frameOffset )); break; default:
                        qFatal("AutomatableModel::controllerValue(int)"
                                "lacks implementation for a scale type");
                        break;
                }

                if( typeInfo<real_t>::isEqual( m_step, 1 ) &&
m_hasStrictStepSize ) v=qRound( v );

                return v;
        }

        AutomatableModel* lm = nullptr;
        if( hasLinkedModels() )
        {
                lm = m_linkedModels.first();
        }
        if(lm)
        {
                real_t v=0.;

                if( lm->controllerConnection() &&
                    (recording || lm->controllerConnection()->hasChanged() ))
                        v= fittedValue( lm->controllerValue( frameOffset,
false ) ); else v=fittedValue( lm->m_value );

                return v;
        }

        //if(recording) return 0.5;
        //qWarning("AutomatableModel::controllerValue should not reach");
        return m_value;
}
*/

ValueBuffer* AutomatableModel::valueBuffer()
{
    // QMutexLocker m(const_cast<QMutex*>(&m_valueBufferMutex));
    QMutexLocker m(&m_valueBufferMutex);
    return m_hasSampleExactData ? &m_valueBuffer : nullptr;
}

/*
ValueBuffer * AutomatableModel::valueBuffer()
{
        QMutexLocker m( &m_valueBufferMutex );
        // if we've already calculated the valuebuffer this period, return the
cached buffer if( m_lastUpdatedPeriod == s_periodCounter )
        {
                return m_hasSampleExactData
                        ? &m_valueBuffer
                        : nullptr;
        }

        real_t oldval = m_value;
        real_t newval = oldval; // make sure our m_value doesn't change midway

        ValueBuffer * vb;
        if( m_controllerConnection &&
            m_controllerConnection->getController()->isSampleExact() &&
            m_controllerConnection->hasChanged() )

        {
                vb = m_controllerConnection->valueBuffer();
                if( vb )
                {
                        real_t * values = vb->values();
                        real_t * nvalues = m_valueBuffer.values();
                        //bool changed=false;
                        switch( m_scaleType )
                        {
                        case Linear:
                                for( int i = 0; i < m_valueBuffer.length();
i++ )
                                {
                                        //real_t newval
                                        newval=minValue<real_t>() + ( range()
* values[i] );
                                        //if(nvalues[i] != newval)
                                        {
                                                nvalues[i] = newval;
                                                //changed=true;
                                        }
                                }
                                break;
                        case Logarithmic:
                                for( int i = 0; i < m_valueBuffer.length();
i++ )
                                {
                                        //real_t newval
                                        newval=logToLinearScale( values[i] );
                                        //if(nvalues[i] != newval)
                                        {
                                                nvalues[i] = newval;
                                                //changed=true;
                                        }
                                }
                                break;
                        default:
                                qFatal("AutomatableModel::valueBuffer() "
                                        "lacks implementation for a scale
type"); break;
                        }

                        m_value=fittedValue(nvalues[0]);
                        //m_value=fittedValue(v);
                        //m_value=fittedValue(newval);
                        m_lastUpdatedPeriod = s_periodCounter;
                        m_hasSampleExactData = true;
                        //if(changed) emit dataChanged();
                        return &m_valueBuffer;
                }
        }
        AutomatableModel* lm = nullptr;
        if( hasLinkedModels() )
        {
                lm = m_linkedModels.first();
        }
        if( lm &&
            lm->controllerConnection() &&
            lm->controllerConnection()->getController()->isSampleExact() &&
            lm->controllerConnection()->hasChanged() )
        {
                vb = lm->valueBuffer();
                if(vb)
                {
                        real_t * values = vb->values();
                        real_t * nvalues = m_valueBuffer.values();
                        //bool changed=false;
                        for( int i = 0; i < vb->length(); i++ )
                        {
                                //real_t newval
                                newval=fittedValue( values[i] );
                                //if(nvalues[i] != newval)
                                {
                                        nvalues[i] = newval;
                                        //changed=true;
                                }
                        }

                        m_value=fittedValue(nvalues[0]);
                        m_lastUpdatedPeriod = s_periodCounter;
                        m_hasSampleExactData = true;
                        //if(changed) emit dataChanged();
                        return &m_valueBuffer;
                }
        }

        if( m_oldValue != newval )
        {
                m_valueBuffer.interpolate( m_oldValue, newval );
                m_oldValue = newval;
                m_lastUpdatedPeriod = s_periodCounter;
                m_hasSampleExactData = true;
                //emit dataChanged();
                return &m_valueBuffer;
        }

        // if we have no sample-exact source for a ValueBuffer, return nullptr
to signify that no data is available at the moment
        // in which case the recipient knows to use the static value() instead
        m_lastUpdatedPeriod = s_periodCounter;
        m_hasSampleExactData = false;
        return nullptr;
}
*/

/*
void AutomatableModel::setBuffer(const ValueBuffer* _vb)
{
        const fpp_t FPP=Engine::mixer()->framesPerPeriod();
        if(_vb==nullptr || _vb->length()!=FPP || m_valueBuffer.length()!=FPP)
        {
                qWarning("AutomatableModel::setBuffer");
                return;
        }

        QMutexLocker m( &m_valueBufferMutex );
        m_valueBuffer.copyFrom(_vb);
        real_t* v=m_valueBuffer.values();
        for(int i=m_valueBuffer.length()-1;i>=0;--i)
                v[i]=fittedValue(v[i]);
        m_lastUpdatedPeriod = s_periodCounter;
        m_hasSampleExactData = true;
        setAutomatedValue(m_valueBuffer.value(0));
}
*/

void AutomatableModel::setAutomatedBuffer(const ValueBuffer* _vb)
{
    long vbp = _vb->period();

    if(Engine::song() == nullptr || !Engine::song()->isPlaying())
        return;

    if(vbp != -1 && vbp > s_periodCounter)
    {
        postponeUpdate(this, _vb);
        return;
    }

    if(vbp != -1 && vbp < s_periodCounter
       && m_lastUpdatedPeriod == s_periodCounter)
    {
        m_hasSampleExactData = false;  // TMP
        if(_vb != nullptr && _vb->length() > 0)
            setAutomatedValue(_vb->value(0));
        // qWarning("AutomatableModel::setAutomatedBuffer old vbp");
        return;
    }

    if(m_lastUpdatedPeriod == s_periodCounter)
    {
        if(vbp == -1)
        {
            qWarning(
                    "AutomatableModel::setAutomatedBuffer already updated "
                    "(%s)",
                    qPrintable(fullDisplayName()));
        }
        return;
    }

    const fpp_t FPP = Engine::mixer()->framesPerPeriod();
    if(_vb == nullptr || _vb->length() != FPP
       || m_valueBuffer.length() != FPP)
    {
        qWarning("AutomatableModel::setAutomatedBuffer bad length");
        return;
    }

    DEBUG_THREAD_CHECK
    QMutexLocker m(&m_valueBufferMutex);

    m_valueBuffer.copyFrom(_vb);

    real_t* v = m_valueBuffer.values();
    for(int i = FPP - 1; i >= 0; --i)
        v[i] = scaledValue(v[i]);

    randomizeValues(&m_valueBuffer);
    fitValues(&m_valueBuffer);

    m_lastUpdatedPeriod  = s_periodCounter;
    m_hasSampleExactData = true;
    propagateAutomatedBuffer();
}

void AutomatableModel::setControlledBuffer(const ValueBuffer* _vb)
{
    if(m_lastUpdatedPeriod == s_periodCounter)
        return;

    const fpp_t FPP = Engine::mixer()->framesPerPeriod();
    if(_vb == nullptr || _vb->length() != FPP
       || m_valueBuffer.length() != FPP)
    {
        qWarning("AutomatableModel::setControlledBuffer");
        return;
    }

    DEBUG_THREAD_CHECK
    QMutexLocker m(&m_valueBufferMutex);

    m_valueBuffer.copyFrom(_vb);

    real_t* v = m_valueBuffer.values();
    for(int i = FPP - 1; i >= 0; --i)
        v[i] = scaledValue(inverseNormalizedValue(v[i]));

    randomizeValues(&m_valueBuffer);
    fitValues(&m_valueBuffer);

    m_lastUpdatedPeriod  = s_periodCounter;
    m_hasSampleExactData = true;
    propagateAutomatedBuffer();
}

void AutomatableModel::unlinkControllerConnection()
{
    if(m_controllerConnection != nullptr)
        m_controllerConnection->disconnect(this);

    m_controllerConnection = nullptr;
}

void AutomatableModel::setInitValue(const real_t value)
{
    m_initValue      = fittedValue(value);
    bool journalling = testAndSetJournalling(false);
    setValue(value);
    m_oldValue = m_value;
    setJournalling(journalling);
    emit initValueChanged(value);
    emit propertiesChanged();
}

void AutomatableModel::reset()
{
    setValue(initValue<real_t>());
}

real_t AutomatableModel::globalAutomationValueAt(const MidiTime& time)
{
    // get patterns that connect to this model
    QVector<AutomationPattern*> patterns
            = AutomationPattern::patternsForModel(this);
    if(patterns.isEmpty())
    {
        // if no such patterns exist, return current value
        return m_value;
    }
    else
    {
        // of those patterns:
        // find the patterns which overlap with the miditime position
        QVector<AutomationPattern*> patternsInRange;
        for(QVector<AutomationPattern*>::ConstIterator it = patterns.begin();
            it != patterns.end(); it++)
        {
            int s = (*it)->startPosition();
            int e = (*it)->endPosition();
            if(s <= time && e >= time)
            {
                patternsInRange += (*it);
            }
        }

        AutomationPattern* latestPattern = nullptr;

        if(!patternsInRange.isEmpty())
        {
            // if there are more than one overlapping patterns, just use the
            // first one because multiple pattern behaviour is undefined
            // anyway
            latestPattern = patternsInRange[0];
        }
        else
        // if we find no patterns at the exact miditime, we need to search for
        // the last pattern before time and use that
        {
            int latestPosition = 0;

            for(QVector<AutomationPattern*>::ConstIterator it
                = patterns.begin();
                it != patterns.end(); it++)
            {
                int e = (*it)->endPosition();
                if(e <= time && e > latestPosition)
                {
                    latestPosition = e;
                    latestPattern  = (*it);
                }
            }
        }

        if(latestPattern)
        {
            // scale/fit the value appropriately and return it
            const real_t value = latestPattern->valueAt(
                    time - latestPattern->startPosition());
            const real_t scaled_value = scaledValue(value);
            return fittedValue(scaled_value);
        }
        // if we still find no pattern, the value at that time is undefined so
        // just return current value as the best we can do
        else
            return m_value;
    }
}

long AutomatableModel::periodCounter()
{
    return s_periodCounter;
}

void AutomatableModel::incrementPeriodCounter()
{
    ++s_periodCounter;
    for(AutomatableModel* m: s_postponedUpdates.keys())
    {
        const ValueBuffer* vb = s_postponedUpdates.value(m);
        if(vb != nullptr)
        {
            const long vbp = vb->period();
            if(vbp == s_periodCounter)
                // QTimer::singleShot(0, [m, vb]() {
                // m->setAutomatedBuffer(vb); });
                QMetaObject::invokeMethod(m, "setAutomatedBuffer",
                                          Qt::BlockingQueuedConnection,
                                          Q_ARG(const ValueBuffer*, vb));
            if(vbp <= s_periodCounter)
                s_postponedUpdates.remove(m);
        }
    }
    // s_postponedUpdates.clear();
}

void AutomatableModel::resetPeriodCounter()
{
    s_periodCounter = 0;
}

QHash<AutomatableModel*, const ValueBuffer*>
        AutomatableModel::s_postponedUpdates;

void AutomatableModel::postponeUpdate(AutomatableModel*  _m,
                                      const ValueBuffer* _vb)
{
    s_postponedUpdates.insert(_m, _vb);
}

FloatModel::FloatModel(real_t         val,
                       real_t         min,
                       real_t         max,
                       real_t         step,
                       Model*         parent,
                       const QString& displayName,
                       const QString& objectName,
                       bool           defaultConstructed) :
      TypedAutomatableModel(val,
                            min,
                            max,
                            step,
                            parent,
                            displayName,
                            objectName,
                            defaultConstructed),
      m_digitCount(6)
{
    setDigitCount();
}

real_t FloatModel::getRoundedValue() const
{
    // return qRound( value() / step<real_t>() ) * step<real_t>();
    return minValue() + round((value() - minValue()) / step()) * step();
}

void FloatModel::setRange(const real_t min,
                          const real_t max,
                          const real_t step)
{
    AutomatableModel::setRange(min, max, step);
    setDigitCount();
}

void FloatModel::setStep(const real_t step)
{
    AutomatableModel::setStep(step);
    setDigitCount();
}

int FloatModel::getDigitCount() const
{
    return m_digitCount;
}

void FloatModel::setDigitCount()
{
    real_t t = abs(step());
    bool   b = false;
    if(t >= 1.)
    {
        b = true;
#ifdef REAL_IS_FLOAT
        t = fmodf(t, 1.f);
#endif
#ifdef REAL_IS_DOUBLE
        t = fmod(t, 1.);
#endif
    }
    if(t <= 0.000001)
    {
        m_digitCount = (b ? 0 : 6);
    }
    else
    {
#ifdef REAL_IS_FLOAT
        int v = int(floorf(1000000.f * t));
#endif
#ifdef REAL_IS_DOUBLE
        int v = int(floor(1000000. * t));
#endif
        int digits = 6;
        while(v % 10 == 0)
        {
            v /= 10;
            digits--;
        }
        m_digitCount = digits;
    }
}

QString FloatModel::displayValue(const real_t val) const
{
    return QString::number(castValue<real_t>(scaledValue(val)), 'f',
                           getDigitCount());
}

QString IntModel::displayValue(const real_t val) const
{
    return QString::number(castValue<int>(scaledValue(val)));
}

QString BoolModel::displayValue(const real_t val) const
{
    return QString::number(castValue<bool>(scaledValue(val)));
}
