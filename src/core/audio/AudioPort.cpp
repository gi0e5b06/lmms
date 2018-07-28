/*
 * AudioPort.cpp - base-class for objects providing sound at a port
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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


#include <QDir>

#include "AudioPort.h"
#include "AudioDevice.h"
#include "EffectChain.h"
#include "FxMixer.h"
#include "Engine.h"
#include "Mixer.h"
#include "MixHelpers.h"
#include "Song.h"
#include "BufferManager.h"
#include "SampleBuffer.h"
#include "NotePlayHandle.h"


AudioPort::AudioPort( const QString & _name, bool _has_effect_chain,
		FloatModel * volumeModel, FloatModel * panningModel,
                      BoolModel * mutedModel, BoolModel * frozenModel,
                      BoolModel * clippingModel) :
	m_bufferUsage( false ),
	m_portBuffer( BufferManager::acquire() ),
	m_extOutputEnabled( false ),
	m_nextFxChannel( 0 ),
	m_name( "unnamed port" ),
	m_effects( _has_effect_chain ? new EffectChain( NULL ) : NULL ),
	m_volumeModel( volumeModel ),
	m_panningModel( panningModel ),
	m_mutedModel( mutedModel ),
	m_frozenModel( frozenModel ),
	m_clippingModel( clippingModel ),
        m_frozenBuf( NULL )
{
	Engine::mixer()->addAudioPort( this );
	setExtOutputEnabled( true );
}




AudioPort::~AudioPort()
{
	setExtOutputEnabled( false );
	Engine::mixer()->removeAudioPort( this );
	delete m_effects;
	BufferManager::release( m_portBuffer );

        if(m_frozenBuf) delete m_frozenBuf;
}




void AudioPort::setExtOutputEnabled( bool _enabled )
{
	if( _enabled != m_extOutputEnabled )
	{
		m_extOutputEnabled = _enabled;
		if( m_extOutputEnabled )
		{
			Engine::mixer()->audioDev()->registerPort( this );
		}
		else
		{
			Engine::mixer()->audioDev()->unregisterPort( this );
		}
	}
}




void AudioPort::setName( const QString & _name )
{
	m_name = _name;
	Engine::mixer()->audioDev()->renamePort( this );
}




bool AudioPort::processEffects()
{
	if( m_effects )
	{
		bool more = m_effects->processAudioBuffer( m_portBuffer, Engine::mixer()->framesPerPeriod(), m_bufferUsage );
		return more;
	}
	return false;
}


void AudioPort::doProcessing()
{
	if( m_mutedModel && m_mutedModel->value() )
	{
		return;
	}

        const Song*   song=Engine::getSong();
	const fpp_t   fpp =Engine::mixer()->framesPerPeriod();
        const f_cnt_t af  =song->getPlayPos().absoluteFrame();

        if(m_frozenModel&&
           m_frozenModel->value()&&
           (song->playMode() == Song::Mode_PlaySong)&&
           song->isPlaying())
        {
                /*
                if(m_frozenBuf&&(af+fpp<=m_frozenLen))
                {
                        //qInfo("AudioPort::doProcessing use frozen buffer"
                        //  " af=%d s=%p t=%p",
                        //  af,m_portBuffer,this);
                        //memset(buf,0,frames*BYTES_PER_FRAME);
                        for(f_cnt_t f=0;f<fpp;++f)
                                for(int c=0; c<2; ++c)
                                        m_portBuffer[f][c]=m_frozenBuf[af+f][c];

                        // send output to fx mixer
                        Engine::fxMixer()->mixToChannel( m_portBuffer, m_nextFxChannel );
                        // TODO: improve the flow here - convert to pull model
                        m_bufferUsage = false;
                        return;
                }
                */
                if(m_frozenBuf)
                {
                        /*
                          qInfo("AudioPort::doProcessing use frozen buffer"
                              " fb=%p af=%d s=%p ap=%p",
                                m_frozenBuf,af,m_portBuffer,this);
                        */
                        for(f_cnt_t f=0;f<fpp;++f)
                        {
                                sample_t vch0,vch1;
                                m_frozenBuf->getDataFrame(af+f,vch0,vch1);
                                m_portBuffer[f][0]=vch0;
                                m_portBuffer[f][1]=vch1;

                                if(af+f>=1000 && af+f<1005)
                                        qInfo("AudioPort::doProcessing use frozen buffer"
                                              " fb=%p af=%d s=%p ap=%p vch0=%f vch1=%f",
                                              m_frozenBuf,af+f,m_portBuffer,this,vch0,vch1);
                        }

                        // send output to fx mixer
                        Engine::fxMixer()->mixToChannel( m_portBuffer, m_nextFxChannel );
                        // TODO: improve the flow here - convert to pull model
                        m_bufferUsage = false;
                        return;
                }
        }

	// clear the buffer
	BufferManager::clear( m_portBuffer );

        //qInfo("AudioPort::doProcessing #1");
	//qDebug( "Playhandles: %d", m_playHandles.size() );
	for( PlayHandle * ph : m_playHandles ) // now we mix all playhandle buffers into the audioport buffer
	{
		if( ph->buffer() )
		{
			if( ph->usesBuffer() )
			{
				m_bufferUsage = true;
				MixHelpers::add( m_portBuffer, ph->buffer(), fpp );
			}

                        // gets rid of playhandle's buffer and sets
                        // pointer to null, so if it doesn't get re-acquired we know to skip it next time
			ph->releaseBuffer();
		}
	}

        //qInfo("AudioPort::doProcessing #2");
	if( m_bufferUsage )
	{
                //qInfo("AudioPort::doProcessing #3");
		// handle volume and panning
		// has both vol and pan models
		if( m_volumeModel && m_panningModel )
		{
			ValueBuffer * volBuf = m_volumeModel->valueBuffer();
			ValueBuffer * panBuf = m_panningModel->valueBuffer();

			// both vol and pan have s.ex.data:
			if( volBuf && panBuf )
			{
				for( f_cnt_t f = 0; f < fpp; ++f )
				{
					float v = volBuf->values()[ f ] * 0.01f;
					float p = panBuf->values()[ f ] * 0.01f;
					m_portBuffer[f][0] *= ( p <= 0 ? 1.0f : 1.0f - p ) * v;
					m_portBuffer[f][1] *= ( p >= 0 ? 1.0f : 1.0f + p ) * v;
				}
			}

			// only vol has s.ex.data:
			else if( volBuf )
			{
				float p = m_panningModel->value() * 0.01f;
				float l = ( p <= 0 ? 1.0f : 1.0f - p );
				float r = ( p >= 0 ? 1.0f : 1.0f + p );
				for( f_cnt_t f = 0; f < fpp; ++f )
				{
					float v = volBuf->values()[ f ] * 0.01f;
					m_portBuffer[f][0] *= v * l;
					m_portBuffer[f][1] *= v * r;
				}
			}

			// only pan has s.ex.data:
			else if( panBuf )
			{
				float v = m_volumeModel->value() * 0.01f;
				for( f_cnt_t f = 0; f < fpp; ++f )
				{
					float p = panBuf->values()[ f ] * 0.01f;
					m_portBuffer[f][0] *= ( p <= 0 ? 1.0f : 1.0f - p ) * v;
					m_portBuffer[f][1] *= ( p >= 0 ? 1.0f : 1.0f + p ) * v;
				}
			}

			// neither has s.ex.data:
			else
			{
				float p = m_panningModel->value() * 0.01f;
				float v = m_volumeModel->value() * 0.01f;
				for( f_cnt_t f = 0; f < fpp; ++f )
				{
					m_portBuffer[f][0] *= ( p <= 0 ? 1.0f : 1.0f - p ) * v;
					m_portBuffer[f][1] *= ( p >= 0 ? 1.0f : 1.0f + p ) * v;
				}
			}
		}

		// has vol model only
		else if( m_volumeModel )
		{
			ValueBuffer * volBuf = m_volumeModel->valueBuffer();

			if( volBuf )
			{
				for( f_cnt_t f = 0; f < fpp; ++f )
				{
					float v = volBuf->values()[ f ] * 0.01f;
					m_portBuffer[f][0] *= v;
					m_portBuffer[f][1] *= v;
				}
			}
			else
			{
				float v = m_volumeModel->value() * 0.01f;
				for( f_cnt_t f = 0; f < fpp; ++f )
				{
					m_portBuffer[f][0] *= v;
					m_portBuffer[f][1] *= v;
				}
			}
		}
	}
	// as of now there's no situation where we only have panning model but no volume model
	// if we have neither, we don't have to do anything here - just pass the audio as is

	// handle effects
	const bool me = processEffects();
        //qInfo("AudioPort::doProcessing #4 me=%d",me);
	if( me || m_bufferUsage )
	{
                /*
                qInfo("AudioPort::doProcessing #5 frozen=%d fb=%p song=%d playing==%d",
                      m_frozenModel->value(),m_frozenBuf,
                      (song->playMode() == Song::Mode_PlaySong),
                      song->isPlaying());
                */

                if(m_frozenModel&&
                   !m_frozenModel->value()&&
                   m_frozenBuf&&
                   (((song->playMode() == Song::Mode_PlaySong)&&song->isPlaying()) ||
                    (Engine::getSong()->isExporting() &&
                     Engine::mixer()->processingSampleRate()==Engine::mixer()->baseSampleRate())))
                {
                        //qInfo("AudioPort::doProcessing #6");
                        /*
                          qInfo("AudioPort::doProcessing freeze"
                              " af=%d s=%p t=%p",
                              af,m_portBuffer,this);
                        for(f_cnt_t f=0; f <fpp; ++f)
                        for(int c=0; c<2; ++c)
                        {
                                if(af+f<m_frozenLen)
                                        m_frozenBuf[af+f][c]=m_portBuffer[f][c];
                        }
                        */
                        for(f_cnt_t f=0; f <fpp; ++f)
                        {
                                m_frozenBuf->setDataFrame(af+f,
                                                          m_portBuffer[f][0],
                                                          m_portBuffer[f][1]);
                                if(af+f>=1000 && af+f<1005)
                                        qInfo("AudioPort::doProcessing freeze to buffer"
                                              " fb=%p af=%d s=%p ap=%p vch0=%f vch1=%f",
                                              m_frozenBuf,af+f,m_portBuffer,this,
                                              m_portBuffer[f][0],m_portBuffer[f][1]);
                        }
                }

                //if(MixHelpers::sanitize(m_portBuffer,fpp))
                //        qInfo("AudioPort: sanitize done!!!");

                if(m_clippingModel && MixHelpers::isClipping(m_portBuffer,fpp))
                        m_clippingModel->setValue(true);

                // send output to fx mixer
		Engine::fxMixer()->mixToChannel( m_portBuffer, m_nextFxChannel );
                // TODO: improve the flow here - convert to pull model
		m_bufferUsage = false;
	}
}


void AudioPort::addPlayHandle( PlayHandle * handle )
{
	m_playHandleLock.lock();
        m_playHandles.append( handle );
	m_playHandleLock.unlock();
}


void AudioPort::removePlayHandle( PlayHandle * handle )
{
	m_playHandleLock.lock();
        PlayHandleList::Iterator it =	qFind( m_playHandles.begin(), m_playHandles.end(), handle );
        if( it != m_playHandles.end() )
	{
                NotePlayHandle* nph=dynamic_cast<NotePlayHandle*>(*it);
                if(nph && !nph->isReleased()) nph->noteOff(0);
                m_playHandles.erase( it );
        }
	m_playHandleLock.unlock();
}


void AudioPort::updateFrozenBuffer(f_cnt_t _len)
{
        if(m_frozenModel)
        {
                if((m_frozenBuf==NULL)||
                   (_len!=m_frozenBuf->frames()))
                {
                        if(m_frozenBuf) delete m_frozenBuf;
                        m_frozenBuf=new SampleBuffer(_len);
                        qInfo("AudioPort::updateFrozenBuffer len=%d",_len);
                }
        }
}




void AudioPort::cleanFrozenBuffer(f_cnt_t _len)
{
        if(m_frozenModel)
        {
                if((m_frozenBuf==NULL)||
                   (_len!=m_frozenBuf->frames())||
                   m_frozenBuf->m_mmapped)
                {
                        if(m_frozenBuf) delete m_frozenBuf;
                        m_frozenBuf=new SampleBuffer(_len);
                        qInfo("AudioPort::cleanFrozenBuffer len=%d",_len);
                }
        }
}




void AudioPort::readFrozenBuffer(QString _uuid)
{
        if(m_frozenModel&&
           //m_frozenModel->value()&&
           m_frozenBuf)
        {
                delete m_frozenBuf;
                m_frozenBuf=NULL;
                QString d=Engine::getSong()->projectDir()
                        +QDir::separator()+"tracks"
                        +QDir::separator()+"frozen";
                if(QFileInfo(d).exists())
                {
                        QString f=d+QDir::separator()+_uuid
                                +"."+SampleBuffer::rawStereoSuffix();
                        qInfo("AudioPort::readFrozenBuffer f=%s",
                              qPrintable(f));
                        QFile fi(f);
                        if(fi.exists())
                        {
                                if(fi.size()==0)
                                        fi.remove();
                                else
                                        m_frozenBuf=new SampleBuffer(f);
                        }
                }
        }
}




void AudioPort::writeFrozenBuffer(QString _uuid)
{
        if(m_frozenModel&&
           //m_frozenModel->value()&&
           m_frozenBuf)
        {
                QString d=Engine::getSong()->projectDir()
                        +QDir::separator()+"tracks"
                        +QDir::separator()+"frozen";
                if(QFileInfo(d).exists())
                {
                        QString f=d+QDir::separator()+_uuid
                                +"."+SampleBuffer::rawStereoSuffix();
                        qInfo("AudioPort::writeFrozenBuffer f=%s",
                              qPrintable(f));
                        m_frozenBuf->writeCacheData(f);
                }
        }
}
