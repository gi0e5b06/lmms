/*
 * LadspaSubPluginFeatures.cpp - derivation from
 *                               Plugin::Descriptor::SubPluginFeatures for
 *                               hosting LADSPA-plugins
 *
 * Copyright (c) 2006-2007 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "LadspaSubPluginFeatures.h"

#include "AudioDevice.h"
#include "Engine.h"
#include "Ladspa2LMMS.h"
#include "LadspaBase.h"
#include "Mixer.h"

#include <QHBoxLayout>
#include <QLabel>

LadspaSubPluginFeatures::LadspaSubPluginFeatures(Plugin::PluginTypes _type) :
      SubPluginFeatures(_type)
{
}

void LadspaSubPluginFeatures::fillDescriptionWidget(QWidget*   _parent,
                                                    const Key* _key) const
{
    const ladspa_key_t& lkey = subPluginKeyToLadspaKey(_key);
    Ladspa2LMMS*        lm   = Engine::getLADSPAManager();

    QLabel* label = new QLabel(_parent);
    label->setText("<p><b>" + QWidget::tr("Name") + ":</b> "
                   + lm->getName(lkey) + "</p>");

    QLabel* fileInfo = new QLabel(_parent);
    fileInfo->setText("<p><b>" + QWidget::tr("File") + ":</b> " + lkey.first
                      + "</p>");

    QLabel* maker = new QLabel(_parent);
    maker->setText("<p><b>" + QWidget::tr("Maker") + ":</b> "
                   + lm->getMaker(lkey) + "</p>");

    QLabel* copyright = new QLabel(_parent);
    copyright->setText("<p><b>" + QWidget::tr("Copyright") + ":</b> "
                       + lm->getCopyright(lkey) + "</p>");

    /*
    QWidget * maker = new QWidget( _parent );
    QHBoxLayout * l = new QHBoxLayout( maker );
    l->setMargin( 0 );
    l->setSpacing( 0 );

    QLabel * maker_label = new QLabel( maker );
    maker_label->setText( "<p><b>" + QWidget::tr( "Maker: " ) );
    maker_label->setAlignment( Qt::AlignTop );
    QLabel * maker_content = new QLabel( maker );
    maker_content->setText( lm->getMaker( lkey ) );
    maker_content->setWordWrap( true );
    l->addWidget( maker_label );
    l->addWidget( maker_content, 1 );

    QWidget * copyright = new QWidget( _parent );
    l = new QHBoxLayout( copyright );
    l->setMargin( 0 );
    l->setSpacing( 0 );

    copyright->setMinimumWidth( _parent->minimumWidth() );
    QLabel * copyright_label = new QLabel( copyright );
    copyright_label->setText( "<p><b>" + QWidget::tr( "Copyright: " ) );
    copyright_label->setAlignment( Qt::AlignTop );

    QLabel * copyright_content = new QLabel( copyright );
    copyright_content->setText( lm->getCopyright( lkey ) );
    copyright_content->setWordWrap( true );
    l->addWidget( copyright_label );
    l->addWidget( copyright_content, 1 );
    */

    QLabel* requiresRealTime = new QLabel(_parent);
    requiresRealTime->setText(
            "<p><b>" + QWidget::tr("Requires Real Time") + ":</b> "
            + (lm->hasRealTimeDependency(lkey) ? QWidget::tr("Yes")
                                               : QWidget::tr("No"))
            + "</p>");

    QLabel* realTimeCapable = new QLabel(_parent);
    realTimeCapable->setText(
            "<p><b>" + QWidget::tr("Real Time Capable") + ":</b> "
            + (lm->isRealTimeCapable(lkey) ? QWidget::tr("Yes")
                                           : QWidget::tr("No"))
            + "</p>");

    QLabel* inplaceBroken = new QLabel(_parent);
    inplaceBroken->setText("<p><b>" + QWidget::tr("In Place Broken")
                           + ":</b> "
                           + (lm->isInplaceBroken(lkey) ? QWidget::tr("Yes")
                                                        : QWidget::tr("No"))
                           + "</p>");

    QLabel* channelsIn = new QLabel(_parent);
    channelsIn->setText(
            "<p><b>" + QWidget::tr("Channels In") + ":</b> "
            + QString::number(lm->getDescription(lkey)->inputChannels)
            + "</p>");

    QLabel* channelsOut = new QLabel(_parent);
    channelsOut->setText(
            "<p><b>" + QWidget::tr("Channels Out") + ":</b> "
            + QString::number(lm->getDescription(lkey)->outputChannels)
            + "</p>");
}

void LadspaSubPluginFeatures::listSubPluginKeys(
        const Plugin::Descriptor* _desc, KeyList& _kl) const
{
    Ladspa2LMMS* lm = Engine::getLADSPAManager();

    l_sortable_plugin_t plugins;
    switch(m_type)
    {
        case Plugin::Instrument:
            plugins = lm->getInstruments();
            break;
        case Plugin::Effect:
            plugins = lm->getValidEffects();
            // plugins += lm->getInvalidEffects();
            break;
        case Plugin::Tool:
            plugins = lm->getAnalysisTools();
            break;
        case Plugin::Other:
            plugins = lm->getOthers();
            break;
        default:
            break;
    }

    for(l_sortable_plugin_t::const_iterator it = plugins.begin();
        it != plugins.end(); ++it)
    {
        if(lm->getDescription((*it).second)->inputChannels
           <= Engine::mixer()->audioDev()->channels())
        {
            _kl.push_back(ladspaKeyToSubPluginKey(_desc, (*it).first,
                                                  (*it).second));
        }
    }
}

ladspa_key_t LadspaSubPluginFeatures::subPluginKeyToLadspaKey(const Key* _key)
{
    QString file = _key->attributes["file"];
    return (ladspa_key_t(
            file.remove(QRegExp("\\.so$")).remove(QRegExp("\\.dll$")) +
#ifdef LMMS_BUILD_WIN32
                    ".dll"
#else
                    ".so"
#endif
            ,
            _key->attributes["plugin"]));
}
