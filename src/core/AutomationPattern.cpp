/*
 * AutomationPattern.cpp - implementation of class AutomationPattern which
 *                         holds dynamic values
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2006-2008 Javier Serrano Polo
 * <jasp00/at/users.sourceforge.net>
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

#include "AutomationPattern.h"

#include "AutomationEditor.h"
#include "AutomationPatternView.h"
#include "AutomationTrack.h"
#include "BBTrackContainer.h"
#include "GuiApplication.h"
#include "Note.h"
#include "ProjectJournal.h"
#include "Song.h"
#include "WaveFormStandard.h"

#include <cmath>

int          AutomationPattern::s_quantization    = 1;
const real_t AutomationPattern::DEFAULT_MIN_VALUE = 0.;
const real_t AutomationPattern::DEFAULT_MAX_VALUE = 1.;

AutomationPattern::AutomationPattern(AutomationTrack* _auto_track) :
      TrackContentObject(_auto_track, "Automation tile"),
      m_autoTrack(_auto_track), m_objects(), m_tension(1.),
      m_waveBank(WaveFormStandard::ZERO_BANK),
      m_waveIndex(WaveFormStandard::ZERO_INDEX), m_waveRatio(0.5),
      m_waveSkew(0.), m_waveAmplitude(0.1), m_waveRepeat(0.),
      m_progressionType(DiscreteProgression), m_dragging(false),
      m_isRecording(false), m_lastRecordedValue(0)
{
    changeLength(MidiTime(1, 0));
    setAutoResize(false);
}

AutomationPattern::AutomationPattern(const AutomationPattern& _other) :
        TrackContentObject(_other.m_autoTrack, _other.displayName()),
      m_autoTrack(_other.m_autoTrack), m_objects(_other.m_objects),
      m_tension(_other.m_tension), m_progressionType(_other.m_progressionType)
{
    for(timeMap::const_iterator it = _other.m_timeMap.begin();
        it != _other.m_timeMap.end(); ++it)
    {
        m_timeMap[it.key()]  = it.value();
        m_tangents[it.key()] = _other.m_tangents[it.key()];
    }

    switch(getTrack()->trackContainer()->type())
    {
        case TrackContainer::BBContainer:
            setAutoResize(true);
            break;

        case TrackContainer::SongContainer:
            // move down
        default:
            setAutoResize(false);
            break;
    }
}

AutomationPattern::~AutomationPattern()
{
}

bool AutomationPattern::isEmpty() const
{
    return m_timeMap.isEmpty();
}

QString AutomationPattern::defaultName() const
{
    if(!m_objects.isEmpty() && m_objects.first() != nullptr)
        return m_objects.first()->fullDisplayName();

    return TrackContentObject::defaultName();
}

void AutomationPattern::setRecording(const bool b)
{
    m_isRecording = b;
    if(b)
        connect(this, SIGNAL(recordValue(MidiTime, real_t)), this,
                SLOT(onRecordValue(MidiTime, real_t)));
    else
        disconnect(this, SIGNAL(recordValue(MidiTime, real_t)), this,
                   SLOT(onRecordValue(MidiTime, real_t)));
}

bool AutomationPattern::addObject(AutomatableModel* _obj, bool _search_dup)
{
    if(_search_dup && m_objects.contains(_obj))
    {
        return false;
    }

    // the automation track is unconnected and there is nothing in the track
    if(m_objects.isEmpty() && hasAutomation() == false)
    {
        // then initialize first value
        // putValue(MidiTime(0),
        // _obj->inverseScaledValue(_obj->value<real_t>()), false);
        putValue(MidiTime(0),
                 _obj->inverseScaledValue(_obj->rawValue<real_t>()));
    }

    m_objects += QPointer<AutomatableModel>(_obj);

    connect(_obj, SIGNAL(destroyed(jo_id_t)), this,
            SLOT(objectDestroyed(jo_id_t)), Qt::DirectConnection);

    emit dataChanged();

    return true;
}

void AutomationPattern::setProgressionType(ProgressionTypes _progressionType)
{
    if(_progressionType == DiscreteProgression
       || _progressionType == LinearProgression
       || _progressionType == CubicHermiteProgression
       || _progressionType == ParabolicProgression)
    {
        m_progressionType = _progressionType;
        emit dataChanged();
    }
}

void AutomationPattern::setTension(const real_t _tension)
{
    // qInfo("AutomationPattern::setTension tension=%f",_tension);
    if(_tension >= -10. && _tension <= 10.)  // nt > -0.01 && nt < 1.01 )
    {
        m_tension = _tension;
        generateTangents();
        emit dataChanged();
    }
}

void AutomationPattern::setWaveBank(const int _waveBank)
{
    // qInfo("AutomationPattern::setWaveBank waveBank=%d",_waveBank);
    if(_waveBank >= WaveFormStandard::MIN_BANK
       && _waveBank <= WaveFormStandard::MAX_BANK)
    {
        m_waveBank = _waveBank;
        emit dataChanged();
    }
}

void AutomationPattern::setWaveIndex(const int _waveIndex)
{
    // qInfo("AutomationPattern::setWaveIndex waveIndex=%d",_waveIndex);
    if(_waveIndex >= WaveFormStandard::MIN_INDEX
       && _waveIndex <= WaveFormStandard::MAX_INDEX)
    {
        m_waveIndex = _waveIndex;
        emit dataChanged();
    }
}

void AutomationPattern::setWaveRatio(const real_t _waveRatio)
{
    // qInfo("AutomationPattern::setWaveRatio waveRatio=%f",_waveRatio);
    if(_waveRatio >= 0. && _waveRatio <= 1.)
    {
        m_waveRatio = _waveRatio;
        emit dataChanged();
    }
}

void AutomationPattern::setWaveSkew(const real_t _waveSkew)
{
    // qInfo("AutomationPattern::setWaveSkew waveSkew=%f",_waveSkew);
    if(_waveSkew >= 0. && _waveSkew <= 1.)
    {
        m_waveSkew = _waveSkew;
        emit dataChanged();
    }
}

void AutomationPattern::setWaveAmplitude(const real_t _waveAmplitude)
{
    // qInfo("AutomationPattern::setWaveAmplitude
    // waveAmplitude=%f",_waveAmplitude);
    if(_waveAmplitude >= -10. && _waveAmplitude <= 10.)
    {
        m_waveAmplitude = _waveAmplitude;
        emit dataChanged();
    }
}

void AutomationPattern::setWaveRepeat(const real_t _waveRepeat)
{
    // qInfo("AutomationPattern::setWaveRepeat waveRepeat=%f",_waveRepeat);
    if(_waveRepeat >= -10. && _waveRepeat <= 20.)
    {
        m_waveRepeat = _waveRepeat;
        emit dataChanged();
    }
}

const AutomatableModel* AutomationPattern::firstObject() const
{
    AutomatableModel* m;
    if(!m_objects.isEmpty() && (m = m_objects.first()) != nullptr)
        return m;

    static FloatModel _fm(0, DEFAULT_MIN_VALUE, DEFAULT_MAX_VALUE, 0.001);
    return &_fm;
}

const AutomationPattern::objectVector& AutomationPattern::objects() const
{
    return m_objects;
}

MidiTime AutomationPattern::timeMapLength() const
{
    if(m_timeMap.isEmpty())
        return 0;
    timeMap::const_iterator it = m_timeMap.end();
    // return MidiTime(MidiTime((it - 1).key()).nextFullTact(), 0);
    tick_t t = (it - 1).key();
    tick_t n = MidiTime::ticksPerTact() / 8;
    return qMax(n, t);
}

tick_t AutomationPattern::unitLength() const
{
    tick_t t = timeMapLength().getTicks();
    tick_t n = MidiTime::ticksPerTact() / 8;
    return qMax(n, t);
}

/*
MidiTime AutomationPattern::beatLength() const
{
        Track* t=getTrack();
        BBTrackContainer* c=dynamic_cast<BBTrackContainer*>
                (t->trackContainer());
        return c ? c->beatLengthOfBB(startPosition()
                                     / DefaultTicksPerTact) : 0;
}
*/

void AutomationPattern::updateLength()
{
    tick_t len;
    if(autoResize() && !isEmpty())
        len = timeMapLength();
    else
        len = length();

    TrackContentObject::updateLength(len);
}

/*
void AutomationPattern::updateBBTrack()
{
        if( getTrack()->trackContainer() == Engine::getBBTrackContainer() )
        {
                Engine::getBBTrackContainer()->updateBBTrack( this );
        }

        if( gui &&
            gui->automationEditor() &&
            gui->automationEditor()->currentPattern() == this )
        {
                gui->automationEditor()->update();
        }
}
*/

MidiTime AutomationPattern::putValue(const MidiTime& time,
                                     const real_t    value,
                                     const bool      quantPos,
                                     const bool      ignoreSurrounding)
{
    cleanObjects();

    const int    q = quantization();
    const tick_t t = quantPos ? Note::quantized(time, q) : time;

    /*
    if((t > 0) && !ignoreSurrounding && (valueAt(t - 1) ==
    value))
    {
        if(m_timeMap.contains(t))
            removeValue(t);
        return t;
    }
    */

    // Remove control points that are covered by the new points
    // quantization value. Control Key to override
    if(quantPos && !ignoreSurrounding)
    {
        // qInfo("putval q=%d t=%d", q, t);
        for(tick_t i = t - q + 1; i < t + q; ++i)
        {
            // AutomationPattern::removeValue(i);
            if(m_timeMap.contains(i))
            {
                m_timeMap.remove(i);
                m_tangents.remove(i);
            }
        }
    }

    // m_timeMap[t] = value;
    m_timeMap.insert(t, value);

    timeMap::const_iterator it = m_timeMap.find(t);
    if(it != m_timeMap.begin())
        --it;
    generateTangents(it, 3);

    // we need to maximize our length in case we're part of a hidden
    // automation track as the user can't resize this pattern
    if(getTrack() && getTrack()->type() == Track::HiddenAutomationTrack)
        updateLength();

    emit dataChanged();
    return t;
}

void AutomationPattern::removeValue(const MidiTime& time)
{
    cleanObjects();

    m_timeMap.remove(time);
    m_tangents.remove(time);
    timeMap::const_iterator it = m_timeMap.lowerBound(time);
    if(it != m_timeMap.begin())
    {
        --it;
    }
    generateTangents(it, 3);

    if(getTrack() && getTrack()->type() == Track::HiddenAutomationTrack)
        updateLength();

    emit dataChanged();
}

void AutomationPattern::onRecordValue(MidiTime time, real_t value)
{
    // removeValue(time);

    /*
    if((time > 0) && (valueAt(time - 1) == value))
    {
        removeValue(time);
        return;
    }
    */

    // if(value != m_lastRecordedValue)
    {
        putValue(time, value, true, false);
        m_lastRecordedValue = value;
    }
    // else
    // if(valueAt(time) != value)
    //{
    //    removeValue(time);
    //}
}

/**
 * @brief Set the position of the point that is being dragged.
 *        Calling this function will also automatically set m_dragging to
 * true, which applyDragValue() have to be called to m_dragging.
 * @param the time(x position) of the point being dragged
 * @param the value(y position) of the point being dragged
 * @param true to snip x position
 * @return
 */
MidiTime AutomationPattern::setDragValue(const MidiTime& time,
                                         const real_t    value,
                                         const bool      quantPos,
                                         const bool      ignoreSurrounding)
{
    if(m_dragging == false)
    {
        MidiTime newTime
                = quantPos ? Note::quantized(time, quantization()) : time;
        this->removeValue(newTime);
        m_oldTimeMap = m_timeMap;
        m_dragging   = true;
    }

    // Restore to the state before it the point were being dragged
    m_timeMap = m_oldTimeMap;

    for(timeMap::const_iterator it = m_timeMap.begin(); it != m_timeMap.end();
        ++it)
    {
        generateTangents(it, 3);
    }

    return /*this->*/ putValue(time, value, quantPos, ignoreSurrounding);
}

/**
 * @brief After the point is dragged, this function is called to apply the
 * change.
 */
void AutomationPattern::applyDragValue()
{
    m_dragging = false;
}

real_t AutomationPattern::valueAt(const MidiTime& _time) const
{
    if(m_timeMap.isEmpty())
    {
        return 0.;
    }

    /*
    if( m_timeMap.contains( _time ) )
    {
            return m_timeMap[_time];
    }
    */
    tick_t time = _time.getTicks();

    if(autoRepeat())
    {
        tick_t ul = unitLength();
        time %= ul;
    }

    timeMap::ConstIterator v = m_timeMap.upperBound(time);

    if(v == m_timeMap.begin())
    {
        return 0.;
    }

    if(v == m_timeMap.end())
    {
        return (v - 1).value();
    }

    return valueAt(v - 1, time - (v - 1).key(),
                   Engine::mixer()->criticalXRuns());
}

real_t AutomationPattern::valueAt(timeMap::const_iterator v,
                                  tick_t                  offset,
                                  bool                    xruns) const
{
    real_t r = 0.;

    ProgressionTypes pt = m_progressionType;
    if(xruns && pt != DiscreteProgression)
        pt = LinearProgression;

    if(v == m_timeMap.end())  //|| offset == 0 )
        r = v.value();
    else
        switch(m_progressionType)
        {
            case DiscreteProgression:
                r = v.value();
                break;
            case LinearProgression:
            {
                real_t slope = ((v + 1).value() - v.value())
                               / ((v + 1).key() - v.key());
                r = v.value() + offset * slope;
            }
            break;
            case CubicHermiteProgression:
                // Implements a Cubic Hermite spline as explained at:
                // http://en.wikipedia.org/wiki/Cubic_Hermite_spline#Unit_interval_.280.2C_1.29
                //
                // Note that we are not interpolating a 2 dimensional point
                // over time as the article describes.  We are interpolating a
                // single value: y.  To make this work we map the values of x
                // that this segment spans to values of t for t = 0.0 -> 1.0
                // and scale the tangents _m1 and _m2
                {
                    const int    numValues = ((v + 1).key() - v.key());
                    const real_t t = (real_t)offset / (real_t)numValues;
                    const real_t m1
                            = (m_tangents[v.key()]) * numValues * m_tension;
                    const real_t m2 = (m_tangents[(v + 1).key()]) * numValues
                                      * m_tension;

                    const real_t t3 = pow(t, 3);
                    const real_t t2 = pow(t, 2);
                    r               = (2 * t3 - 3 * t2 + 1) * v.value()
                        + (t3 - 2 * t2 + t) * m1
                        + (-2 * t3 + 3 * t2) * (v + 1).value()
                        + (t3 - t2) * m2;
                }
                break;
            case ParabolicProgression:
            {
                // v1=v
                timeMap::const_iterator v2 = v + 1;
                real_t                  x0, x1, x2, x3;
                real_t                  y0, y1, y2, y3;
                x1 = v.key();
                x2 = v2.key();
                y1 = v.value();
                y2 = v2.value();
                if(v == m_timeMap.begin())
                {
                    x0 = 2 * x1 - x2;
                    y0 = 2 * y1 - y2;
                }
                else
                {
                    timeMap::const_iterator v0 = v - 1;
                    x0                         = v0.key();
                    y0                         = v0.value();
                }
                if(v2 == m_timeMap.end())
                {
                    x3 = 2 * x2 - x1;
                    y3 = 2 * y2 - y1;
                }
                else
                {
                    timeMap::const_iterator v3 = v + 2;
                    x3                         = v3.key();
                    y3                         = v3.value();
                }
                real_t x = x1 + real_t(offset);
                // if(offset==0) qInfo("X %f %f %f %f X=%f",x0,x1,x2,x3,x);
                // if(offset==0) qInfo("Y %f %f %f %f",y0,y1,y2,y3);
                real_t a0 = ((x - x1) * (x - x2) * (x - x3))
                            / ((x0 - x1) * (x0 - x2) * (x0 - x3));
                real_t a1 = ((x - x0) * (x - x2) * (x - x3))
                            / ((x1 - x0) * (x1 - x2) * (x1 - x3));
                real_t a2 = ((x - x0) * (x - x1) * (x - x3))
                            / ((x2 - x0) * (x2 - x1) * (x2 - x3));
                real_t a3 = ((x - x0) * (x - x1) * (x - x2))
                            / ((x3 - x0) * (x3 - x1) * (x3 - x2));
                // if(offset==0) qInfo("A %f %f %f %f",a0,a1,a2,a3);
                r = a0 * y0 + a1 * y1 + a2 * y2 + a3 * y3;
                // if(offset==0) qInfo("R=%f",r);
            }
            break;
        }

    if(xruns)
        return r;

    const AutomatableModel* m = firstObject();

    const WaveFormStandard* wf
            = WaveFormStandard::get(m_waveBank, m_waveIndex);
    if(wf != WaveFormStandard::ZERO)
    {
        if(offset == 0)
        {
            // real_t x  = 0.;
            real_t dy = (v + 1).value() - v.value();
            real_t my = m->range();
            real_t w0 = wf->f(0.);
            // real_t w1 = wf->f(1.);
            r += abs((1. - m_waveRatio) * dy + m_waveRatio * my) * m_waveSkew
                 * w0 * m_waveAmplitude;
        }
        else if(v != m_timeMap.end())
        {
            real_t rx = (v + 1).key() - v.key();
            if(rx > 0.)
            {
                real_t x  = real_t(offset) / rx;
                real_t dy = (v + 1).value() - v.value();
                real_t my = m->range();
                real_t w0 = wf->f(0.);
                real_t w1 = wf->f(1.);
                real_t nw = rx * fastexp2(m_waveRepeat) / 4. / 192.;
                if(nw >= 1.)
                    nw = qMax(1., round(nw));
                else if(nw > SILENCE)
                    nw = 1. / qMax(1., round(1. / nw));
                else
                    nw = 0.;

                real_t ph = fmod(x * nw, 1.);
                r += abs((1. - m_waveRatio) * dy + m_waveRatio * my)
                     * (wf->f(ph) + (1. - m_waveSkew) * (-w0 - x * (w1 - w0)))
                     * m_waveAmplitude;
            }
        }
    }

    return qBound(m->minValue<real_t>(), r, m->maxValue<real_t>());
}

real_t* AutomationPattern::valuesAfter(const MidiTime& _time) const
{
    tick_t time = _time.getTicks();

    if(autoRepeat())
    {
        tick_t ul = unitLength();
        time %= ul;
    }

    timeMap::ConstIterator v = m_timeMap.lowerBound(time);
    if(v == m_timeMap.end() || (v + 1) == m_timeMap.end())
    {
        return nullptr;
    }

    int     numValues = (v + 1).key() - v.key();
    real_t* ret       = new real_t[numValues];

    for(int i = 0; i < numValues; i++)
    {
        ret[i] = valueAt(v, i);
    }

    return ret;
}

void AutomationPattern::flipHorizontally()
{
    if(isEmpty())
        return;

    timeMap tempMap;
    // instrumentTrack()->lock();
    tick_t len = autoRepeat() ? unitLength() : timeMapLength().getTicks();
    bool   modified = false;
    for(tick_t p: m_timeMap.keys())
    {
        real_t v = m_timeMap.value(p);
        // p = (len - p) % (len+1);
        // if(p < 0) p += len;
        p = len - p;
        tempMap.insert(p, v);
        modified = true;
    }
    if(modified)
    {
        m_timeMap.clear();
        m_timeMap = tempMap;
        updateLength();
        generateTangents();
        emit dataChanged();
        Engine::getSong()->setModified();
    }
}

void AutomationPattern::flipVertically()
{
    if(isEmpty())
        return;

    const AutomatableModel* m    = firstObject();
    const real_t            vmin = m->minValue<real_t>();
    const real_t            vmax = m->maxValue<real_t>();

    timeMap tempMap;
    bool    modified = false;
    for(tick_t p: m_timeMap.keys())
    {
        real_t v = m_timeMap.value(p);
        // p = (len - p) % (len+1);
        // if(p < 0) p += len;
        v = vmax - (v - vmin);
        tempMap.insert(p, v);
        modified = true;
    }
    if(modified)
    {
        m_timeMap.clear();
        m_timeMap = tempMap;
        // updateLength();
        generateTangents();
        emit dataChanged();
        Engine::getSong()->setModified();
    }
}

void AutomationPattern::flipY(int min, int max)
{
    timeMap                tempMap   = m_timeMap;
    timeMap::ConstIterator iterate   = m_timeMap.lowerBound(0);
    real_t                 tempValue = 0;

    int numPoints = 0;

    for(int i = 0; (iterate + i + 1) != m_timeMap.end()
                   && (iterate + i) != m_timeMap.end();
        i++)
    {
        numPoints++;
    }

    for(int i = 0; i <= numPoints; i++)
    {

        if(min < 0)
        {
            tempValue = valueAt((iterate + i).key()) * -1;
            putValue(MidiTime((iterate + i).key()), tempValue, false);
        }
        else
        {
            tempValue = max - valueAt((iterate + i).key());
            putValue(MidiTime((iterate + i).key()), tempValue, false);
        }
    }

    generateTangents();
    emit dataChanged();
}

void AutomationPattern::flipY()
{
    flipY(getMin(), getMax());
}

void AutomationPattern::flipX(int length)
{
    timeMap tempMap;

    timeMap::ConstIterator iterate   = m_timeMap.lowerBound(0);
    real_t                 tempValue = 0;
    int                    numPoints = 0;

    for(int i = 0; (iterate + i + 1) != m_timeMap.end()
                   && (iterate + i) != m_timeMap.end();
        i++)
    {
        numPoints++;
    }

    real_t realLength = (iterate + numPoints).key();

    if(length != -1 && length != realLength)
    {
        if(realLength < length)
        {
            tempValue = valueAt((iterate + numPoints).key());
            putValue(MidiTime(length), tempValue, false);
            numPoints++;
            for(int i = 0; i <= numPoints; i++)
            {
                tempValue        = valueAt((iterate + i).key());
                MidiTime newTime = MidiTime(length - (iterate + i).key());
                tempMap[newTime] = tempValue;
            }
        }
        else
        {
            for(int i = 0; i <= numPoints; i++)
            {
                tempValue = valueAt((iterate + i).key());
                MidiTime newTime;

                if((iterate + i).key() <= length)
                {
                    newTime = MidiTime(length - (iterate + i).key());
                }
                else
                {
                    newTime = MidiTime((iterate + i).key());
                }
                tempMap[newTime] = tempValue;
            }
        }
    }
    else
    {
        for(int i = 0; i <= numPoints; i++)
        {
            tempValue = valueAt((iterate + i).key());
            cleanObjects();
            MidiTime newTime = MidiTime(realLength - (iterate + i).key());
            tempMap[newTime] = tempValue;
        }
    }

    m_timeMap.clear();
    m_timeMap = tempMap;

    generateTangents();
    emit dataChanged();
}

void AutomationPattern::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    //_this.setAttribute("pos", startPosition());
    //_this.setAttribute("len", length());
    //_this.setAttribute("name", name());
    //_this.setAttribute("mute", QString::number(isMuted()));

    TrackContentObject::saveSettings(_doc, _this);

    _this.setAttribute("prog", QString::number(progressionType()));
    _this.setAttribute("tens", QString::number(tension()));  // obsolete
    _this.setAttribute("tension", QString::number(tension()));
    _this.setAttribute("wave_bank", QString::number(waveBank()));
    _this.setAttribute("wave_index", QString::number(waveIndex()));
    _this.setAttribute("wave_ratio", QString::number(waveRatio()));
    _this.setAttribute("wave_skew", QString::number(waveSkew()));
    _this.setAttribute("wave_amplitude", QString::number(waveAmplitude()));
    _this.setAttribute("wave_repeat", QString::number(waveRepeat()));

    for(timeMap::const_iterator it = m_timeMap.begin(); it != m_timeMap.end();
        ++it)
    {
        QDomElement element = _doc.createElement("time");
        element.setAttribute("pos", QString::number(it.key()));
        element.setAttribute("value", QString::number(it.value()));
        _this.appendChild(element);
    }

    for(objectVector::const_iterator it = m_objects.begin();
        it != m_objects.end(); ++it)
    {
        if(*it)
        {
            QDomElement element = _doc.createElement("object");
            element.setAttribute("id", ProjectJournal::idToSave((*it)->id()));
            _this.appendChild(element);
        }
    }
}

void AutomationPattern::loadSettings(const QDomElement& _this)
{
    clear();

    // movePosition(_this.attribute("pos").toInt());
    // setName(_this.attribute("name"));
    // setMuted(_this.attribute("mute", QString::number(false)).toInt());
    TrackContentObject::loadSettings(_this);

    setProgressionType(
            static_cast<ProgressionTypes>(_this.attribute("prog").toInt()));
    setTension(_this.attribute("tens").toFloat());  // obsolete
    setTension(_this.attribute("tension").toFloat());

    setWaveBank(_this.attribute("wave_bank").toInt());
    setWaveIndex(_this.attribute("wave_index").toInt());
    setWaveRatio(_this.attribute("wave_ratio").toFloat());
    setWaveSkew(_this.attribute("wave_skew").toFloat());
    setWaveAmplitude(_this.attribute("wave_amplitude").toFloat());
    setWaveRepeat(_this.attribute("wave_repeat").toFloat());

    for(QDomNode node = _this.firstChild(); !node.isNull();
        node          = node.nextSibling())
    {
        QDomElement element = node.toElement();
        if(element.isNull())
        {
            continue;
        }
        if(element.tagName() == "time")
        {
            m_timeMap[element.attribute("pos").toInt()]
                    = element.attribute("value").toFloat();
        }
        else if(element.tagName() == "object")
        {
            m_idsToResolve << element.attribute("id").toInt();
        }
    }

    int len = _this.attribute("len").toInt();
    if(len <= 0)
    {
        // TODO: Handle with an upgrade method
        updateLength();
    }
    else
    {
        changeLength(len);
    }
    generateTangents();
}

const QString AutomationPattern::name() const
{
    if(!TrackContentObject::name().isEmpty())
    {
        return TrackContentObject::name();
    }
    if(!m_objects.isEmpty() && m_objects.first() != nullptr)
    {
        return m_objects.first()->fullDisplayName();
    }
    return tr("Drag a control while pressing <%1>").arg(UI_CTRL_KEY);
}

TrackContentObjectView* AutomationPattern::createView(TrackView* _tv)
{
    return new AutomationPatternView(this, _tv);
}

bool AutomationPattern::isAutomated(const AutomatableModel* _m)
{
    Tracks l;
    l += Engine::getSong()->tracks();
    l += Engine::getBBTrackContainer()->tracks();
    l += Engine::getSong()->globalAutomationTrack();

    for(Tracks::ConstIterator it = l.begin(); it != l.end(); ++it)
    {
        if((*it)->type() == Track::AutomationTrack
           || (*it)->type() == Track::HiddenAutomationTrack)
        {
            const Track::tcoVector& v = (*it)->getTCOs();
            for(Track::tcoVector::ConstIterator j = v.begin(); j != v.end();
                ++j)
            {
                const AutomationPattern* a
                        = dynamic_cast<const AutomationPattern*>(*j);
                if(a && a->hasAutomation())
                {
                    for(objectVector::const_iterator k = a->m_objects.begin();
                        k != a->m_objects.end(); ++k)
                    {
                        if(*k == _m)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

/*! \brief returns a list of all the automation patterns everywhere that are
 * connected to a specific model \param _m the model we want to look for
 */
QVector<AutomationPattern*>
        AutomationPattern::patternsForModel(const AutomatableModel* _m)
{
    QVector<AutomationPattern*> patterns;
    Tracks                      l;
    l += Engine::getSong()->tracks();
    l += Engine::getBBTrackContainer()->tracks();
    l += Engine::getSong()->globalAutomationTrack();

    // go through all tracks...
    for(Tracks::ConstIterator it = l.begin(); it != l.end(); ++it)
    {
        // we want only automation tracks...
        if((*it)->type() == Track::AutomationTrack
           || (*it)->type() == Track::HiddenAutomationTrack)
        {
            // get patterns in those tracks....
            const Track::tcoVector& v = (*it)->getTCOs();
            // go through all the patterns...
            for(Track::tcoVector::ConstIterator j = v.begin(); j != v.end();
                ++j)
            {
                AutomationPattern* a = dynamic_cast<AutomationPattern*>(*j);
                // check that the pattern has automation
                if(a && a->hasAutomation())
                {
                    // now check is the pattern is connected to the model we
                    // want by going through all the connections of the
                    // pattern
                    bool has_object = false;
                    for(objectVector::const_iterator k = a->m_objects.begin();
                        k != a->m_objects.end(); ++k)
                    {
                        if(*k == _m)
                        {
                            has_object = true;
                        }
                    }
                    // if the patterns is connected to the model, add it to
                    // the list
                    if(has_object)
                    {
                        patterns += a;
                    }
                }
            }
        }
    }
    return patterns;
}

AutomationPattern*
        AutomationPattern::globalAutomationPattern(AutomatableModel* _m)
{
    AutomationTrack* t = Engine::getSong()->globalAutomationTrack();
    Track::tcoVector v = t->getTCOs();
    for(Track::tcoVector::const_iterator j = v.begin(); j != v.end(); ++j)
    {
        AutomationPattern* a = dynamic_cast<AutomationPattern*>(*j);
        if(a)
        {
            for(objectVector::const_iterator k = a->m_objects.begin();
                k != a->m_objects.end(); ++k)
            {
                if(*k == _m)
                {
                    return a;
                }
            }
        }
    }

    AutomationPattern* a = new AutomationPattern(t);
    a->addObject(_m, false);
    return a;
}

void AutomationPattern::resolveAllIDs()
{
    Tracks l = Engine::getSong()->tracks()
               + Engine::getBBTrackContainer()->tracks();
    l += Engine::getSong()->globalAutomationTrack();
    for(Tracks::iterator it = l.begin(); it != l.end(); ++it)
    {
        if((*it)->type() == Track::AutomationTrack
           || (*it)->type() == Track::HiddenAutomationTrack)
        {
            Track::tcoVector v = (*it)->getTCOs();
            for(Track::tcoVector::iterator j = v.begin(); j != v.end(); ++j)
            {
                AutomationPattern* a = dynamic_cast<AutomationPattern*>(*j);
                if(a)
                {
                    for(QVector<jo_id_t>::Iterator k
                        = a->m_idsToResolve.begin();
                        k != a->m_idsToResolve.end(); ++k)
                    {
                        JournallingObject* o
                                = Engine::projectJournal()->journallingObject(
                                        *k);
                        if(o && dynamic_cast<AutomatableModel*>(o))
                        {
                            a->addObject(dynamic_cast<AutomatableModel*>(o),
                                         false);
                        }
                    }
                    a->m_idsToResolve.clear();
                    a->emit dataChanged();
                }
            }
        }
    }
}

void AutomationPattern::clear()
{
    m_timeMap.clear();
    m_tangents.clear();

    emit dataChanged();
}

void AutomationPattern::objectDestroyed(jo_id_t _id)
{
    // TODO: distict between temporary removal (e.g. LADSPA controls
    // when switching samplerate) and real deletions because in the latter
    // case we had to remove ourselves if we're the global automation
    // pattern of the destroyed object
    m_idsToResolve += _id;

    for(objectVector::Iterator objIt = m_objects.begin();
        objIt != m_objects.end(); objIt++)
    {
        Q_ASSERT(!(*objIt).isNull());
        if((*objIt)->id() == _id)
        {
            // Assign to objIt so that this loop work even break; is removed.
            objIt = m_objects.erase(objIt);
            break;
        }
    }

    emit dataChanged();
}

void AutomationPattern::cleanObjects()
{
    for(objectVector::iterator it = m_objects.begin(); it != m_objects.end();)
    {
        if(*it)
        {
            ++it;
        }
        else
        {
            it = m_objects.erase(it);
        }
    }
}

void AutomationPattern::generateTangents()
{
    generateTangents(m_timeMap.begin(), m_timeMap.size());
}

void AutomationPattern::generateTangents(timeMap::const_iterator it,
                                         int numToGenerate)
{
    if(numToGenerate <= 0)
        return;

    if(m_timeMap.size() < 2)
    {
        m_tangents[it.key()] = 0.;
        return;
    }

    for(int i = 0; i < numToGenerate; i++)
    {
        if(it == m_timeMap.begin())
        {
            m_tangents[it.key()] = 0.;
            // ( (it+1).value() - (it).value() ) /
            //   ( (it+1).key() - (it).key() );
        }
        else if(it + 1 == m_timeMap.end())
        {
            m_tangents[it.key()] = 0.;
            return;
        }
        else
        {
            if(m_progressionType == CubicHermiteProgression)
                m_tangents[it.key()] = ((it + 1).value() - (it - 1).value())
                                       / ((it + 1).key() - (it - 1).key());
            else
                m_tangents[it.key()] = 0.;
        }
        it++;
    }
}
