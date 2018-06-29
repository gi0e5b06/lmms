/*
 * LV2SubPluginFeatures.cpp - derivation from
 *                               Plugin::Descriptor::SubPluginFeatures for
 *                               hosting LV2-plugins
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
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

#include <QHBoxLayout>
#include <QLabel>

#include "LV2EffectGDXSubPluginFeatures.h"
#include "AudioDevice.h"
#include "Engine.h"
#include "LV22LMMS.h"
#include "LV2Base.h"
#include "Mixer.h"


LV2EffectGDXSubPluginFeatures::LV2EffectGDXSubPluginFeatures( Plugin::PluginTypes _type ) :
	SubPluginFeatures( _type )
{
}




void LV2EffectGDXSubPluginFeatures::fillDescriptionWidget( QWidget* _parent,
                                                           const Key * _key  ) const
{
	const lv2_key_t& lkey=subPluginKeyToLV2Key( _key);
	LV2Manager* lm=Engine::getLV2Manager();

	QLabel* label=new QLabel(_parent);
	label->setText(QWidget::tr("Name: ") + lm->getName(lkey));

	QLabel* uri = new QLabel(_parent);
	uri->setText(QWidget::tr("URI: %1").arg(lkey));

	QWidget * maker = new QWidget(_parent);
	QHBoxLayout * l = new QHBoxLayout(maker);
	l->setMargin(0);
	l->setSpacing(0);

	QLabel * maker_label = new QLabel(maker);
	maker_label->setText(QWidget::tr("Maker: "));
	maker_label->setAlignment(Qt::AlignTop);
	QLabel * maker_content = new QLabel(maker);
	maker_content->setText(lm->getMaker(lkey));
	maker_content->setWordWrap(true);
	l->addWidget(maker_label);
	l->addWidget(maker_content, 1);

	QWidget * copyright = new QWidget(_parent);
	l = new QHBoxLayout(copyright);
	l->setMargin(0);
	l->setSpacing(0);

	copyright->setMinimumWidth(_parent->minimumWidth());
	QLabel * copyright_label = new QLabel(copyright);
	copyright_label->setText(QWidget::tr("Copyright: "));
	copyright_label->setAlignment(Qt::AlignTop);

	QLabel * copyright_content = new QLabel(copyright);
	copyright_content->setText(lm->getCopyright(lkey));
	copyright_content->setWordWrap(true);
	l->addWidget(copyright_label);
	l->addWidget(copyright_content, 1);

        /*
	QLabel * requiresRealTime = new QLabel(_parent);
	requiresRealTime->setText(QWidget::tr("Requires Real Time: ") +
					(lm->hasRealTimeDependency(lkey) ?
							QWidget::tr("Yes") :
							QWidget::tr("No")));

	QLabel * realTimeCapable = new QLabel(_parent);
	realTimeCapable->setText(QWidget::tr("Real Time Capable: ") +
					(lm->isRealTimeCapable(lkey) ?
							QWidget::tr("Yes") :
							QWidget::tr("No")));

	QLabel * inplaceBroken = new QLabel(_parent);
	inplaceBroken->setText(QWidget::tr("In Place Broken: ") +
					(lm->isInplaceBroken(lkey) ?
							QWidget::tr("Yes") :
							QWidget::tr("No")));
        */

        /*
	QLabel * channelsIn = new QLabel(_parent);
	channelsIn->setText(QWidget::tr("Channels In: ") +
		QString::number(lm->getDescription(lkey)->inputChannels));

	QLabel * channelsOut = new QLabel(_parent);
	channelsOut->setText(QWidget::tr("Channels Out: ") +
		QString::number(lm->getDescription(lkey)->outputChannels));
        */

        QLabel* lbPorts=new QLabel(_parent);
        lbPorts->setText(QWidget::tr("Ports:"));
        for(int i=0;i<lm->getPortCount(lkey);i++)
        {
                QLabel* lb=new QLabel(_parent);
                lb->setText(QString("- ")+
                            lm->getPortName(lkey,i)+" ("+
                            (lm->isPortInput(lkey,i) ? " input" : "")+
                            (lm->isPortOutput(lkey,i) ? " output" : "")+
                            (lm->isPortAudio(lkey,i) ? " audio" : "")+
                            (lm->isPortControl(lkey,i) ? " control" : "")+")");
        }
}


void LV2EffectGDXSubPluginFeatures::listSubPluginKeys
  (const Plugin::Descriptor* _desc,
   KeyList& _kl) const
{
	LV22LMMS* lm = Engine::getLV2Manager();

	l_lv2_sortable_plugin_t plugins;
	switch(m_type)
	{
		case Plugin::Instrument:
			plugins = lm->getInstruments();
			break;
		case Plugin::Effect:
			plugins = lm->getValidEffects();
			//plugins += lm->getInvalidEffects();
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

	for(l_lv2_sortable_plugin_t::const_iterator it = plugins.begin();
            it != plugins.end(); ++it)
	{
                //qWarning("LV2EffectGDXSubPluginFeatures::listSubPluginKeys %s",qPrintable(*it));
		//if(lm->getDescription((*it).second)->inputChannels <= 
                //Engine::mixer()->audioDev()->channels())
		//{
                _kl.push_back(lv2KeyToSubPluginKey(_desc, lm->getName(*it), *it));
		//}
	}
}



lv2_key_t LV2EffectGDXSubPluginFeatures::subPluginKeyToLV2Key(const Key* _key)
{
        return _key->attributes["uri"];
}

