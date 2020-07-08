/*
 * SplitGDXControls.cpp - controls for splitting effect (tree node)
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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

#include "SplitGDXControls.h"

#include "Engine.h"
#include "Song.h"
#include "SplitGDX.h"

#include <QDomElement>

SplitGDXControls::SplitGDXControls(SplitGDXEffect* effect) :
      EffectControls(effect), m_effect(effect),
      m_splitModel(0., 0., 1., 0.00001, this, tr("Sub dry")),
      m_wetModel(0.5, 0., 1., 0.00001, this, tr("Sub wet")),
      m_remModel(0.5, 0., 1., 0.00001, this, tr("Remainder"))
{
}

void SplitGDXControls::changeControl()
{
    //	engine::getSong()->setModified();
}

void SplitGDXControls::loadSettings(const QDomElement& _this)
{
    // m_effect->m_splitChain->loadSettings(_this);
    // m_effect->m_wetChain->loadSettings(_this);
    // m_effect->m_remChain->loadSettings(_this);

    QDomNode node = _this.firstChild();
    while(!node.isNull())
    {
        if(node.isElement() && node.nodeName() == "fxchain")
        {
            QDomElement chainData = node.toElement();

            const QString name = chainData.attribute("name");
            if(name == "sub_dry_chain")
                m_effect->m_splitChain->restoreState(chainData);
            else if(name == "sub_wet_chain")
                m_effect->m_wetChain->restoreState(chainData);
            else if(name == "sub_rem_chain")
                m_effect->m_remChain->restoreState(chainData);
        }
        node = node.nextSibling();
    }

    m_splitModel.loadSettings(_this, "sub_dry");
    m_wetModel.loadSettings(_this, "sub_wet");
    m_remModel.loadSettings(_this, "sub_rem");
}

void SplitGDXControls::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    m_splitModel.saveSettings(_doc, _this, "sub_dry");
    m_wetModel.saveSettings(_doc, _this, "sub_wet");
    m_remModel.saveSettings(_doc, _this, "sub_rem");

    QDomElement splitec = m_effect->m_splitChain->saveState(_doc, _this);
    splitec.setAttribute("name", "sub_dry_chain");
    QDomElement wetec = m_effect->m_wetChain->saveState(_doc, _this);
    wetec.setAttribute("name", "sub_wet_chain");
    QDomElement remec = m_effect->m_remChain->saveState(_doc, _this);
    remec.setAttribute("name", "sub_rem_chain");

    // m_effect->m_splitChain->saveSettings(_doc, _this);
    // m_effect->m_wetChain->saveSettings(_doc, _this);
    // m_effect->m_remChain->saveSettings(_doc, _this);
}
