/*
 * Note.cpp - implementation of class note
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Note.h"

#include "DetuningHelper.h"
#include "lmms_math.h"

#include <QDomElement>

#include <cmath>

Note::Note(const MidiTime& length,
           const MidiTime& pos,
           int             key,
           volume_t        volume,
           panning_t       panning,
           DetuningHelper* detuning) :
      m_selected(false),
      m_oldKey(qBound(0, key, NumKeys)), m_oldPos(pos), m_oldLength(length),
      m_isPlaying(false), m_key(qBound(0, key, NumKeys)),
      m_volume(qBound(MinVolume, volume, MaxVolume)),
      m_panning(qBound(PanningLeft, panning, PanningRight)),
      m_probability(1.), m_legato(false), m_marcato(false), m_staccato(false),
      m_length(length), m_pos(pos), m_detuning(nullptr)
{
    if(detuning)
    {
        m_detuning = sharedObject::ref(detuning);
    }
    else
    {
        createDetuning();
    }
}

Note::Note(const Note& note) :
      SerializingObject(note), m_selected(note.m_selected),
      m_oldKey(note.m_oldKey), m_oldPos(note.m_oldPos),
      m_oldLength(note.m_oldLength), m_isPlaying(note.m_isPlaying),
      m_key(note.m_key), m_volume(note.m_volume), m_panning(note.m_panning),
      m_probability(note.m_probability), m_legato(note.m_legato),
      m_marcato(note.m_marcato), m_staccato(note.m_staccato),
      m_length(note.m_length), m_pos(note.m_pos), m_detuning(nullptr)
{
    if(note.m_detuning != nullptr)
        m_detuning = sharedObject::ref(note.m_detuning);
}

Note::~Note()
{
    // qInfo("deleting note %p",this);
    if(m_detuning != nullptr)
        sharedObject::unref(m_detuning);
}

void Note::setLength(const MidiTime& length)
{
    m_length = length;
}

void Note::setPos(const MidiTime& pos)
{
    m_pos = pos;
}

void Note::setKey(const int key)
{
    const int k = qBound(0, key, NumKeys - 1);
    m_key       = k;
}

void Note::setVolume(volume_t volume)
{
    const volume_t v = qBound(MinVolume, volume, MaxVolume);
    m_volume         = v;
}

void Note::setPanning(panning_t panning)
{
    const panning_t p = qBound(PanningLeft, panning, PanningRight);
    m_panning         = p;
}

void Note::setProbability(real_t _probability)
{
    m_probability = bound(0., _probability, 1.);
}

void Note::setLegato(bool _legato)
{
    m_legato = _legato;
}

void Note::setMarcato(bool _marcato)
{
    m_marcato = _marcato;
}

void Note::setStaccato(bool _staccato)
{
    m_staccato = _staccato;
}

MidiTime Note::quantized(const MidiTime& m, const int qGrid)
{
    real_t p = (real_t(m) / real_t(qGrid));
    if(fraction(p) < 0.5)
        return static_cast<int>(p) * qGrid;

    return static_cast<int>(p + 1) * qGrid;
}

void Note::quantizeLength(const int qGrid)
{
    setLength(quantized(length(), qGrid));
    if(length() == 0)
        setLength(qGrid);
}

void Note::quantizePos(const int qGrid)
{
    setPos(quantized(pos(), qGrid));
}

void Note::saveSettings(QDomDocument& doc, QDomElement& parent)
{
    parent.setAttribute("key", m_key);
    parent.setAttribute("vol", m_volume);
    parent.setAttribute("pan", m_panning);
    parent.setAttribute("len", m_length);
    parent.setAttribute("pos", m_pos);

    if(m_probability < 1.)
        parent.setAttribute("probability", formatNumber(m_probability));
    if(m_legato)
        parent.setAttribute("legato", 1);
    if(m_marcato)
        parent.setAttribute("marcato", 1);
    if(m_staccato)
        parent.setAttribute("staccato", 1);

    if(m_detuning && m_length)
    {
        m_detuning->saveSettings(doc, parent);
    }
}

void Note::loadSettings(const QDomElement& _this)
{
    if(_this.hasAttribute("key"))
        m_key = _this.attribute("key").toInt();
    else if(_this.hasAttribute("tone") && _this.hasAttribute("oct"))
        m_key = _this.attribute("tone").toInt()
                + _this.attribute("oct").toInt() * KeysPerOctave;
    else
        m_key = DefaultKey;

    m_volume  = _this.attribute("vol").toInt();
    m_panning = _this.attribute("pan").toInt();
    m_length  = _this.attribute("len").toInt();
    m_pos     = _this.attribute("pos").toInt();

    m_probability = _this.hasAttribute("probability")
                            ? _this.attribute("probability").toFloat()
                            : 1.;
    m_legato = _this.hasAttribute("legato")
                       ? _this.attribute("legato").toInt()
                       : false;
    m_marcato = _this.hasAttribute("marcato")
                        ? _this.attribute("marcato").toInt()
                        : false;
    m_staccato = _this.hasAttribute("staccato")
                         ? _this.attribute("staccato").toInt()
                         : false;

    if(_this.hasChildNodes())
    {
        createDetuning();
        m_detuning->loadSettings(_this);
    }
}

void Note::createDetuning()
{
    if(m_detuning == nullptr)
    {
        m_detuning = sharedObject::ref(new DetuningHelper);
        m_detuning->automationPattern();
        m_detuning->setRange(-MaxDetuning, MaxDetuning, 0.5);
        m_detuning->automationPattern()->setProgressionType(
                AutomationPattern::LinearProgression);
    }
}

bool Note::hasDetuningInfo() const
{
    return m_detuning && m_detuning->hasAutomation();
}

bool Note::withinRange(tick_t tickStart, tick_t tickEnd) const
{
    tick_t p = pos().ticks();
    return p >= tickStart && p <= tickEnd && length().ticks() != 0;
}

static QHash<QString, int> MIDI_KEYS_N2I;
static QVector<QString>    MIDI_KEYS_I2N;

void Note::buildKeyTables()
{
    if(MIDI_KEYS_N2I.size() == 0)
    {
        const char* NOK[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                               "F#", "G",  "G#", "A",  "A#", "B"};
        for(int o = 0; o < NumKeys / KeysPerOctave; o++)
            for(int n = 0; n < KeysPerOctave; n++)
            {
                int i = o * KeysPerOctave + n;
                if(i >= NumKeys)
                    continue;
                QString k = QString("%1%2").arg(NOK[n]).arg(o - 1);
                MIDI_KEYS_N2I.insert(k, i);
                MIDI_KEYS_I2N.insert(i, k);
            }
    }
}

void Note::fillRootModel(ComboBoxModel& _model, bool _none)
{
    // _model.setDisplayName(_model.tr("Root"));
    // _model.clear();
    if(_none)
    {
        static ComboBoxModel::Items items;
        if(items.isEmpty())
        {
            items.append(ComboBoxModel::Item(-1, QObject::tr("None"), nullptr,
                                             -1));
            for(int n = 0; n < KeysPerOctave; n++)
            {
                QString text = Note::findNoteName(n);
                items.append(ComboBoxModel::Item(n, text, nullptr, n));
            }
        }
        _model.setItems(&items);
    }
    else
    {
        static ComboBoxModel::Items items;
        if(items.isEmpty())
        {
            for(int n = 0; n < KeysPerOctave; n++)
            {
                QString text = Note::findNoteName(n);
                items.append(ComboBoxModel::Item(n, text, nullptr, n));
            }
        }
        _model.setItems(&items);
    }
}

int Note::findKeyNum(QString& _name)
{
    buildKeyTables();
    int r = MIDI_KEYS_N2I.value(_name.toUpper(), -1);
    if((r >= 0) || r < NumKeys)
        return r;
    r = _name.toInt();
    if((r > 0) || (r < NumKeys) || (_name == "0"))
        return r;
    return -1;
}

QString Note::findKeyName(int _num)
{
    if((_num < 0) || (_num >= NumKeys))
        return "";
    buildKeyTables();
    return MIDI_KEYS_I2N.value(_num, "");
}

QString Note::findNoteName(int _num)
{
    if((_num < 0) || (_num >= NumKeys))
        return "";
    return Note::findKeyName(_num % KeysPerOctave).replace("-1", "");
}
