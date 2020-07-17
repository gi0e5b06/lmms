/*
 * EffectChain.cpp - class for processing and effects chain
 *
 * Copyright (c) 2006-2008 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2008-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "EffectChain.h"

#include "DummyEffect.h"
#include "Effect.h"
#include "MixHelpers.h"
#include "Song.h"

#include <QDomElement>

EffectChain::EffectChain(Model* _parent) :
      Model(_parent, "Effect chain", "chain"), SerializingObject(),
      m_enabledModel(false, this, tr("Effects enabled"), "enabled")
{
}

EffectChain::~EffectChain()
{
    qInfo("EffectChain::~EffectChain START");
    clear();
    qInfo("EffectChain::~EffectChain END");
}

void EffectChain::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    //_this.setAttribute( "enabled", m_enabledModel.value() );
    m_enabledModel.saveSettings(_doc, _this, "enabled");

    _this.setAttribute("numofeffects",
                       m_effects.size());  // m_effects.count());

    // for(Effect* effect: m_effects)
    m_effects.map([&_doc, &_this](Effect* effect) {
        if(DummyEffect* dummy = dynamic_cast<DummyEffect*>(effect))
        {
            _this.appendChild(dummy->originalPluginData());
        }
        else
        {
            effect->saveState(_doc, _this);
            /*
            QDomElement ef = effect->saveState( _doc, _this );
            ef.setAttribute( "name", QString::fromUtf8(
            effect->descriptor()->name ) ); ef.appendChild(
            effect->key().saveXML( _doc ) );
            */
        }
    });
}

void EffectChain::loadSettings(const QDomElement& _this)
{
    clear();

    // TODO This method should probably also lock the mixer

    // m_enabledModel.setValue( _this.attribute( "enabled" ).toInt() );
    m_enabledModel.loadSettings(_this, "enabled");

    const int plugin_cnt = _this.attribute("numofeffects").toInt();

    QDomNode node      = _this.firstChild();
    int      fx_loaded = 0;
    while(!node.isNull() && fx_loaded < plugin_cnt)
    {
        if(node.isElement() && node.nodeName() == "effect")
        {
            QDomElement effectData = node.toElement();

            const QString name = effectData.attribute("name");
            EffectKey     key(
                    effectData.elementsByTagName("key").item(0).toElement());

            Effect* e = Effect::instantiate(name.toUtf8(), this, &key);

            if(e != nullptr && e->isOkay()
               && e->nodeName() == node.nodeName())
            {
                e->restoreState(effectData);
            }
            else
            {
                delete e;
                e = new DummyEffect(parentModel(), effectData);
            }

            // m_effects.push_back(e);
            m_effects.append(e);
            ++fx_loaded;
        }
        node = node.nextSibling();
    }

    emit dataChanged();
}

void EffectChain::appendEffect(Effect* _effect)
{
    // Engine::mixer()->requestChangeInModel();
    m_effects.append(_effect);
    // Engine::mixer()->doneChangeInModel();

    emit dataChanged();
}

void EffectChain::removeEffect(Effect* _effect)
{
    /*
        Engine::mixer()->requestChangeInModel();

        Effect ** found = qFind( m_effects.begin(), m_effects.end(), _effect
       ); if( found == m_effects.end() )
        {
                Engine::mixer()->doneChangeInModel();
                return;
        }
        m_effects.erase( found );

        Engine::mixer()->doneChangeInModel();
    */

    if(m_effects.removeOne(_effect))
        emit dataChanged();
}

void EffectChain::moveUp(Effect* _effect)
{
    /*
    if(_effect != m_effects.first())
    {
        QVector<Effect*>::Iterator it
                = qFind(m_effects.begin(), m_effects.end(), _effect);
        it = m_effects.erase(it);
        m_effects.insert(it - 1, _effect);
    }
    */

    if(m_effects.moveUp(_effect))
        emit dataChanged();
}

void EffectChain::moveDown(Effect* _effect)
{
    /*
    if(_effect != m_effects.last())
    {
        QVector<Effect*>::Iterator it
                = qFind(m_effects.begin(), m_effects.end(), _effect);
        it = m_effects.erase(it);
        m_effects.insert(it + 1, _effect);
    }
    */
    if(m_effects.moveDown(_effect))
        emit dataChanged();
}

void EffectChain::moveTop(Effect* _effect)
{
    /*
    if(_effect != m_effects.first())
    {
        m_effects.removeOne(_effect);
        m_effects.prepend(_effect);
    }
    */
    if(m_effects.moveTop(_effect))
        emit dataChanged();
}

void EffectChain::moveBottom(Effect* _effect)
{
    /*
    if(_effect != m_effects.last())
    {
        m_effects.removeOne(_effect);
        m_effects.append(_effect);
    }
    */
    if(m_effects.moveBottom(_effect))
        emit dataChanged();
}

bool EffectChain::processAudioBuffer(sampleFrame* _buf,
                                     const fpp_t  _frames,
                                     bool         _hasInput)
{
    if(m_enabledModel.value() == false)
        return false;

    const bool exporting = Engine::song()->isExporting();
    if(exporting)  // strip infs/nans if exporting
        MixHelpers::sanitize(_buf, _frames);

    // qInfo("EffectChain::pAB 1
    // silent=%d",MixHelpers::isSilent(_buf,_frames));

    bool moreEffects = false;
    /*
    for(EffectList::Iterator it = m_effects.begin(); it != m_effects.end();
        ++it)
    {
        if(_hasInput || (*it)->isRunning())
        {
            if(_hasInput && !(*it)->isRunning())
                (*it)->startRunning();

            moreEffects |= (*it)->processAudioBuffer(_buf, _frames);
            if(exporting)  // strip infs/nans if exporting
                MixHelpers::sanitize(_buf, _frames);
        }
    }
    */

    m_effects.map(
            [_buf, _frames, _hasInput, &moreEffects, exporting](Effect* e) {
                if(_hasInput || e->isRunning())
                {
                    if(_hasInput && !e->isRunning())
                        e->startRunning();

                    moreEffects |= e->processAudioBuffer(_buf, _frames);
                    if(exporting)  // strip infs/nans if exporting
                        MixHelpers::sanitize(_buf, _frames);
                }
            });

    return moreEffects;
}

void EffectChain::startRunning()
{
    if(m_enabledModel.value() == false)
        return;

    /*
    for( EffectList::Iterator it = m_effects.begin();
                                            it != m_effects.end(); it++ )
    {
            ( *it )->startRunning();
    }
    */
}

void EffectChain::clear()
{
    emit aboutToClear();

    // Engine::mixer()->requestChangeInModel();

    m_enabledModel.setValue(false);

    /*
    while(m_effects.count())
    {
        Effect* e = m_effects[m_effects.count() - 1];
        m_effects.pop_back();
        delete e;
    }
    */

    qInfo("EffectChain::clear START");
    /*
      m_effects.map(
            [](Effect* e) {
                qInfo("EffectChain::clear delete %s START",
      qPrintable(e->displayName())); delete e; qInfo("EffectChain::clear
      delete %s END", qPrintable(e->displayName()));
            },
            true);
    */
    while(!m_effects.isEmpty())
    {
        Effect* e = m_effects.last();
        removeEffect(e);
        e->deleteLater();
    }

    qInfo("EffectChain::clear END");
    // Engine::mixer()->doneChangeInModel();
}
