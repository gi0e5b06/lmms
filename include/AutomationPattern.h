/*
 * AutomationPattern.h - declaration of class AutomationPattern, which
 * contains all information about an automation pattern
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2006-2008 Javier Serrano Polo
 * <jasp00/at/users.sourceforge.net>
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

#ifndef AUTOMATION_PATTERN_H
#define AUTOMATION_PATTERN_H

#include "SafeList.h"
#include "Track.h"

#include <QMap>
#include <QPointer>

class AutomationTrack;
class AutomationPattern;
class MidiTime;

typedef QVector<AutomationPattern*> AutomationPatterns;

class EXPORT AutomationPattern : public Tile
{
    Q_OBJECT

  public:
    enum ProgressionTypes
    {
        DiscreteProgression,
        LinearProgression,
        CubicHermiteProgression,
        ParabolicProgression
    };

    typedef QMap<tick_t, real_t> timeMap;
    // typedef QVector<QPointer<AutomatableModel>> Objects;
    // typedef SafeList<QPointer<AutomatableModel>> Objects;
    typedef SafeList<AutomatableModel*> Objects;

    AutomationPattern(AutomationTrack* _automationTrack);
    AutomationPattern(const AutomationPattern& _other);
    virtual ~AutomationPattern();

    bool    isEmpty() const override;
    QString defaultName() const override;
    tick_t  unitLength() const override;
    void    rotate(tick_t _ticks) override;
    void    splitEvery(tick_t _ticks) override;
    void    splitAt(tick_t _tick) override;

    bool addObject(AutomatableModel* _obj);  //, bool _search_dup = true);

    const AutomatableModel* firstObject() const;

    const Objects& objects() const
    {
        return m_objects;
    }

    MidiTime putValue(const MidiTime& time,
                      const real_t    value,
                      const bool      quantPos                = true,
                      const bool      ignoreSurroundingPoints = true);

    void removeValue(const MidiTime& time);

    // progression-type stuff
    INLINE ProgressionTypes progressionType() const
    {
        return m_progressionType;
    }
    void setProgressionType(ProgressionTypes _progressionType);

    INLINE real_t tension() const
    {
        return m_tension;
    }
    void setTension(const real_t _tension);

    INLINE int waveBank() const
    {
        return m_waveBank;
    }
    void setWaveBank(const int _waveBank);

    INLINE int waveIndex() const
    {
        return m_waveIndex;
    }
    void setWaveIndex(const int _waveIndex);

    INLINE real_t waveRatio() const
    {
        return m_waveRatio;
    }
    void setWaveRatio(const real_t _waveRatio);

    INLINE real_t waveSkew() const
    {
        return m_waveSkew;
    }
    void setWaveSkew(const real_t _waveSkew);

    INLINE real_t waveAmplitude() const
    {
        return m_waveAmplitude;
    }
    void setWaveAmplitude(const real_t _waveAmplitude);

    INLINE real_t waveRepeat() const
    {
        return m_waveRepeat;
    }
    void setWaveRepeat(const real_t _waveRepeat);

    MidiTime timeMapLength() const;

    // virtual MidiTime beatLength() const;
    // virtual void changeLength( const MidiTime & _length );
    virtual void updateLength();
    // virtual void updateBBTrack();

    MidiTime setDragValue(const MidiTime& time,
                          const real_t    value,
                          const bool      quantPos   = true,
                          const bool      controlKey = false);

    void applyDragValue();

    bool isDragging() const
    {
        return m_dragging;
    }

    INLINE const timeMap& getTimeMap() const
    {
        return m_timeMap;
    }

    INLINE timeMap& getTimeMap()
    {
        return m_timeMap;
    }

    INLINE const timeMap& getTangents() const
    {
        return m_tangents;
    }

    INLINE timeMap& getTangents()
    {
        return m_tangents;
    }

    INLINE real_t getMin() const
    {
        return firstObject()->minValue<real_t>();
    }

    INLINE real_t getMax() const
    {
        return firstObject()->maxValue<real_t>();
    }

    INLINE bool hasAutomation() const
    {
        return m_timeMap.isEmpty() == false;
    }

    real_t  valueAt(const MidiTime& _time) const;
    real_t* valuesAfter(const MidiTime& _time) const;

    // insert the values from the models into the map
    virtual void automatedValuesAt(const MidiTime&    _time,
                                   AutomatedValueMap& _map);

    virtual QString name() const;

    // settings-management
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    static INLINE const QString classNodeName()
    {
        return "automationpattern";
    }

    virtual QString nodeName() const
    {
        return classNodeName();
    }

    INLINE AutomationTrack* automationTrack() const
    {
        return m_automationTrack;
    }

    virtual TileView* createView(TrackView* _tv);

    static bool               isAutomated(const AutomatableModel* _m);
    static AutomationPatterns patternsForModel(const AutomatableModel* _m);
    static AutomationPattern* globalAutomationPattern(AutomatableModel* _m);
    static void               resolveAllIDs();

    bool isRecording() const
    {
        return m_isRecording;
    }

    void setRecording(bool b);

    static int quantizationX()
    {
        return s_quantizationX;
    }

    static void setQuantizationX(int q)
    {
        s_quantizationX = q;
    }

    static real_t quantizationY()
    {
        return s_quantizationY;
    }

    static void setQuantizationY(real_t q)
    {
        s_quantizationY = q;
    }

  public slots:
    void clear() override;
    void flipHorizontally() override;
    void flipVertically() override;

    virtual void moveAbsUp();
    virtual void moveAbsDown();
    virtual void moveRelUp();
    virtual void moveRelDown();

    void objectDestroyed(jo_id_t);
    void flipY(int min, int max);
    void flipY();
    void flipX(int length = -1);

    void onRecordValue(MidiTime time, real_t value);

  signals:
    void recordValue(MidiTime time, real_t value);

  protected:
    AutomatableModel* dummyObject() const;

  private:
    void   cleanObjects();
    void   generateTangents();
    void   generateTangents(timeMap::const_iterator it, int numToGenerate);
    real_t valueAt(timeMap::const_iterator v,
                   int                     offset,
                   bool                    xruns = false) const;

    AutomationTrack* m_automationTrack;
    QVector<jo_id_t> m_idsToResolve;
    QVector<QString> m_uuidsToResolve;
    Objects          m_objects;
    timeMap          m_timeMap;  // actual values
    timeMap m_oldTimeMap;        // old values for storing the values before
                                 // setDragValue() is called.
    timeMap m_tangents;          // slope at each point for calculating spline

    real_t m_tension;
    int    m_waveBank;
    int    m_waveIndex;
    real_t m_waveRatio;
    real_t m_waveSkew;
    real_t m_waveAmplitude;
    real_t m_waveRepeat;

    bool             m_hasAutomation;
    ProgressionTypes m_progressionType;

    bool m_dragging;

    bool   m_isRecording;
    real_t m_lastRecordedValue;

    static int    s_quantizationX;  // ticks
    static real_t s_quantizationY;
    // static FloatModel s_dummyFirstObject;
    // static const real_t DEFAULT_MIN_VALUE;
    // static const real_t DEFAULT_MAX_VALUE;

    friend class AutomationPatternView;
};

#endif
