/*
 * PadsGDX.cpp - sample player for pads
 *
 * Copyright (c) 2017 gi0e5b06
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


#include <QHash>
#include <QPainter>
#include <QBitmap>
#include <QDomDocument>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>
#include <QDropEvent>

#include <samplerate.h>

#include "PadsGDX.h"
#include "PadsGDXView.h"
#include "ConfigManager.h"
#include "Engine.h"
#include "Song.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "NotePlayHandle.h"
#include "interpolation.h"
#include "gui_templates.h"
#include "ToolTip.h"
#include "StringPairDrag.h"
#include "DataFile.h"

#include "embed.h"

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT padsgdx_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"PadsGDX",
	QT_TRANSLATE_NOOP( "pluginBrowser",
                           "One sample per key. Intended for pads. "
                           "Limited support for SFZ soundfonts." ),
	"gi0e5b06",
	0x0100,
	Plugin::Instrument,
	new PluginPixmapLoader( "logo" ),
	"wav,ogg,ds,spx,au,voc,aif,aiff,flac,raw",
	NULL
} ;

}




PadsGDX::PadsGDX( InstrumentTrack * _instrument_track ) :
	Instrument( _instrument_track, &padsgdx_plugin_descriptor ),
        m_currentKey(-1),
        m_checking( false ),
        m_loading( false ),
        m_SFZFile( "" ),
	m_nextPlayStartPoint( 0 ),
	m_nextPlayBackwards( false )
{
	for(int i=0;i<128;i++)
        {
                m_sampleBuffer       [i]=NULL;
                m_startPointModel    [i]=NULL;
                m_endPointModel      [i]=NULL;
                m_loopStartPointModel[i]=NULL;
                m_loopEndPointModel  [i]=NULL;
                m_reverseModel       [i]=NULL;
                m_loopModel          [i]=NULL;
                m_stutterModel       [i]=NULL;
        }

        if(instrumentTrack() &&
           instrumentTrack()->baseNoteModel() )
                setCurrentKey(instrumentTrack()->baseNoteModel()->value());
        else
                setCurrentKey(69);

        setAudioFile("bassloops/briff01.ogg");
	//onPointChanged();
}




PadsGDX::~PadsGDX()
{
}




void PadsGDX::playNote(NotePlayHandle* _n, sampleFrame* _buffer )
{
        if(!_n) return;
        int key=_n->key();
        if(key<0 || key>127) return;

        //int base=instrumentTrack()->baseNoteModel()->value();
        //qInfo("PadsGDX::playNote origin=%d",_n->origin());
        //key+=69-base;

        if((_n->origin()==NotePlayHandle::OriginMidiInput)&&
           (_n->generation()==0))
                setCurrentKey(key);

        SampleBuffer* sample=m_sampleBuffer[key];
        //qInfo("PadsGDX::play key=%d base=%d s=%p",key,base,sample);
        if(!sample) return;

	const fpp_t frames = _n->framesLeftForCurrentPeriod();
	const f_cnt_t offset = _n->noteOffset();

	// Magic key - a frequency < 20 (say, the bottom piano note if using
	// a A4 base tuning) restarts the start point. The note is not actually
	// played.
	if( m_stutterModel[key]&&
            m_stutterModel[key]->value()==true&&
            _n->frequency()<20.0 )
	{
		m_nextPlayStartPoint = sample->startFrame();
		m_nextPlayBackwards = false;
		return;
	}

	if( !_n->m_pluginData )
	{
		if( m_stutterModel[key]&&
                    m_stutterModel[key]->value()==true&&
                    m_nextPlayStartPoint >= sample->endFrame() )
		{
			// Restart playing the note if in stutter mode, not in loop mode,
			// and we're at the end of the sample.
			m_nextPlayStartPoint = sample->startFrame();
			m_nextPlayBackwards = false;
		}

		// set interpolation mode for libsamplerate
		int srcmode = SRC_LINEAR;
                /*
		switch( m_interpolationModel[key]->value() )
		{
			case 0:
				srcmode = SRC_ZERO_ORDER_HOLD;
				break;
			case 1:
				srcmode = SRC_LINEAR;
				break;
			case 2:
				srcmode = SRC_SINC_MEDIUM_QUALITY;
				break;
		}
                */
		_n->m_pluginData = new handleState( _n->hasDetuningInfo(), srcmode );
		((handleState*)_n->m_pluginData)->setFrameIndex( m_nextPlayStartPoint );
		((handleState*)_n->m_pluginData)->setBackwards( m_nextPlayBackwards );

                // debug code
                /*
                  qDebug( "frames %d", sample->frames() );
                  qDebug( "startframe %d", sample->startFrame() );
                  qDebug( "nextPlayStartPoint %d", m_nextPlayStartPoint );
                */
	}

	if( ! _n->isFinished() )
	{
		if( sample->play( _buffer + offset,
                                  (handleState*)_n->m_pluginData,
                                  frames, 440, //force _n->frequency(),
                                  static_cast<SampleBuffer::LoopMode>( m_loopModel[key]->value() ) ) )
		{
			applyRelease( _buffer, _n );
			instrumentTrack()->processAudioBuffer(_buffer, frames+offset, _n);

                        if(key==currentKey())
                                emit isPlaying( ((handleState*)_n->m_pluginData)->frameIndex() );
		}
		else
		{
			memset(_buffer, 0, (frames+offset)*sizeof(sampleFrame) );
                        if(key==currentKey())
                                emit isPlaying( 0 );
		}
	}
	else
	{
                if(key==currentKey())
                        emit isPlaying( 0 );
	}

	if( m_stutterModel[key]->value() == true )
	{
		m_nextPlayStartPoint=((handleState*)_n->m_pluginData)->frameIndex();
		m_nextPlayBackwards =((handleState*)_n->m_pluginData)->isBackwards();
	}
}




void PadsGDX::deleteNotePluginData(NotePlayHandle* _n)
{
	delete (handleState*)_n->m_pluginData;
}


void PadsGDX::loadFile( const QString & _file )
{
	setAudioFile( _file );
}


QString PadsGDX::nodeName( void ) const
{
	return padsgdx_plugin_descriptor.name;
}


int PadsGDX::getBeatLen( NotePlayHandle * _n ) const
{
        if(!_n) return 0;
        int key=_n->key();
        if(key<0 || key>127) return 0;

        SampleBuffer* sample=m_sampleBuffer[key];
        if(!sample) return 0;

	const float freq_factor = BaseFreq / _n->frequency() *
                Engine::mixer()->processingSampleRate() / Engine::mixer()->baseSampleRate();

	return static_cast<int>( floorf( ( sample->endFrame() - sample->startFrame() ) * freq_factor ) );
}


PluginView* PadsGDX::instantiateView( QWidget* _parent )
{
	PadsGDXView* r=new PadsGDXView( this, _parent );
        r->doConnections();
        return r;
}


void PadsGDX::createKey(int _key,const QString& _fileName,double _semitones)
{
        if(_key<0 || _key>127) return;
        createKey(_key,new SampleBuffer(_fileName),_semitones);
}

void PadsGDX::createKey(int _key,SampleBuffer* _sample,double _semitones)
{
        if(_key<0 || _key>127) return;
        if(!_sample) return;

        SampleBuffer* old=m_sampleBuffer[_key];
        if(old) destroyKey(_key);

        //m_semitones          [_key]=_semitones;
        m_sampleBuffer       [_key]=_sample;
	m_ampModel           [_key]=new FloatModel( 100.f, 0.f, 500.f, 0.1f,       this, tr( "Amplify" ) );
	m_tuneModel          [_key]=new FloatModel(   0.f, -144.f, 144.f, 0.01f,   this, tr( "Tune" ) );
	m_startPointModel    [_key]=new FloatModel(   0.f, 0.f,   1.f, 0.0000001f, this, tr( "Start of sample" ) );
	m_endPointModel      [_key]=new FloatModel(   1.f, 0.f,   1.f, 0.0000001f, this, tr( "End of sample" ) );
	m_loopStartPointModel[_key]=new FloatModel(   0.f, 0.f,   1.f, 0.0000001f, this, tr( "Start of loop" ) );
	m_loopEndPointModel  [_key]=new FloatModel(   1.f, 0.f,   1.f, 0.0000001f, this, tr( "End of loop" ) );
	m_reverseModel       [_key]=new BoolModel ( false, this, tr( "Reverse sample" ) );
	m_loopModel          [_key]=new IntModel  ( 0, 0, 2, this, tr( "Loop mode" ) );
	m_stutterModel       [_key]=new BoolModel ( false, this, tr( "Stutter" ) );
	//m_interpolationModel( this, tr( "Interpolation mode" ) )

        connect( _sample, SIGNAL( sampleUpdated() ),
                 this, SLOT( onSampleUpdated() ) );

	connect( m_ampModel[_key], SIGNAL( dataChanged() ),
		 this, SLOT( onAmpModelChanged() ) );
	connect( m_tuneModel[_key], SIGNAL( dataChanged() ),
		 this, SLOT( onTuneModelChanged() ) );
        connect( m_reverseModel[_key], SIGNAL( dataChanged() ),
                 this, SLOT( onReverseModelChanged() ) );
	connect( m_startPointModel[_key], SIGNAL( dataChanged() ),
		 this, SLOT( onStartPointChanged() ) );
	connect( m_endPointModel[_key], SIGNAL( dataChanged() ),
		 this, SLOT( onEndPointChanged() ) );
	connect( m_loopStartPointModel[_key], SIGNAL( dataChanged() ),
		 this, SLOT( onLoopStartPointChanged() ) );
	connect( m_loopEndPointModel[_key], SIGNAL( dataChanged() ),
		 this, SLOT( onLoopEndPointChanged() ) );
	connect( m_stutterModel[_key], SIGNAL( dataChanged() ),
		 this, SLOT( onStutterModelChanged() ) );

        if(_semitones!=0.) m_tuneModel[_key]->setValue(_semitones);
        //_sample->retune(_semitones);

	//interpolation modes
	//m_interpolationModel.addItem( tr( "None" ) );
	//m_interpolationModel.addItem( tr( "Linear" ) );
	//m_interpolationModel.addItem( tr( "Sinc" ) );
	//m_interpolationModel[key]->setValue( 0 );
}


void PadsGDX::destroyKey(int _key)
{
        if(_key<0 || _key>127) return;

        SampleBuffer* old=m_sampleBuffer[_key];
        if(!old) return;

        m_sampleBuffer[_key]=NULL;
        if(old)
        {
                disconnect( old, SIGNAL( sampleUpdated() ),
                            this, SLOT( onSampleUpdated() ) );
                delete old;
        }
        if(m_startPointModel    [_key]) { delete m_startPointModel    [_key]; m_startPointModel    [_key]=NULL; }
	if(m_endPointModel      [_key]) { delete m_endPointModel      [_key]; m_endPointModel      [_key]=NULL; }
	if(m_loopStartPointModel[_key]) { delete m_loopStartPointModel[_key]; m_loopStartPointModel[_key]=NULL; }
	if(m_loopEndPointModel  [_key]) { delete m_loopEndPointModel  [_key]; m_loopEndPointModel  [_key]=NULL; }
	if(m_reverseModel       [_key]) { delete m_reverseModel       [_key]; m_reverseModel       [_key]=NULL; }
	if(m_loopModel          [_key]) { delete m_loopModel          [_key]; m_loopModel          [_key]=NULL; }
	if(m_stutterModel       [_key]) { delete m_stutterModel       [_key]; m_stutterModel       [_key]=NULL; }
}


int PadsGDX::currentKey()
{
        return m_currentKey;
}


void PadsGDX::setCurrentKey(int _key)
{
        if(_key<0 || _key>127) return;

        if(_key!=m_currentKey)
        {
                //if(instrumentTrack()->baseNoteModel()->value()!=_key)
                //instrumentTrack()->baseNoteModel()->setValue(_key);
                m_currentKey=_key;
                //qInfo("PadsGDX::setCurrentKey emit from %p keyUpdated",this);
                emit keyUpdated( _key );
                //qInfo("PadsGDX::setCurrentKey emit dataChanged");
                //emit dataChanged( );
                //qInfo("PadsGDX::setCurrentKey emit sampleUpdated");
                //emit sampleUpdated( );
        }
}


SampleBuffer* PadsGDX::currentSample()
{
        int key=currentKey();//instrumentTrack()->baseNoteModel()->value();
        //if(key<0 || key>127) return NULL;
        return m_sampleBuffer[key];
}


/*
void PadsGDX::setCurrentSample(SampleBuffer* _sample)
{
        int key=currentKey();//instrumentTrack()->baseNoteModel()->value();
        //qInfo("PadsGDX::setCurrentSample key=%d sample=%p",key,_sample);
        //if(key<0 || key>127) return;

        /
        SampleBuffer* old=m_sampleBuffer[key];
        m_sampleBuffer[key]=_sample;
        if(old) delete old;
        //qInfo("PadsGDX::setCurrentSample emit dataChanged");
        emit dataChanged();
        /
        destroyKey(key);
        createKey(key,_sample);
}
*/

const QString PadsGDX::audioFile()
{
        SampleBuffer* sample=currentSample();
        return (sample ? sample->audioFile() : "");
}


void PadsGDX::setAudioFile(const QString& _audioFile, bool _rename)
{
        SampleBuffer* sample=currentSample();

        //qInfo("PadsGDX::setAudioFile f=%s s=%p",qPrintable(_audioFile),sample);

        if(!sample)
        {
                /*
                sample=new SampleBuffer(_audioFile);
                connect( sample, SIGNAL( sampleUpdated() ),
                         this, SLOT( onSampleUpdated() ) );
                setCurrentSample(sample);
                */
                int key=currentKey();
                destroyKey(key);
                createKey(key,_audioFile);
                emit dataChanged();
                onSampleUpdated();
        }
        else sample->setAudioFile(_audioFile);

        /*
	// is current channel-name equal to previous-filename??
	if( _rename &&
		( instrumentTrack()->name() ==
			QFileInfo( sample->audioFile() ).fileName() ||
				sample->audioFile().isEmpty() ) )
	{
		// then set it to new one
		instrumentTrack()->setName( QFileInfo( _audioFile).fileName() );
	}
	// else we don't touch the track-name, because the user named it self
        */
}


void PadsGDX::onSampleUpdated()
{
        if(m_loading) return;
        //qInfo("PadsGDX::onSampleUpdated");
        onPointChanged();
        emit sampleUpdated();
}


void PadsGDX::onReverseModelChanged()
{
        if(m_loading) return;

        int key=currentKey();
        SampleBuffer* sample=currentSample();

        if(sample)
        {
                sample->setReversed(m_reverseModel[key]->value());
                m_nextPlayStartPoint=sample->startFrame();
                m_nextPlayBackwards = false;
                emit dataChanged();
        }
}


void PadsGDX::onAmpModelChanged()
{
        if(m_loading) return;

        int key=currentKey();
        SampleBuffer* sample=currentSample();

        if(sample)
        {
                sample->setAmplification(m_ampModel[key]->value()/100.0f);
                emit dataChanged();
        }
}


void PadsGDX::onTuneModelChanged()
{
        if(m_loading) return;

        int key=currentKey();
        SampleBuffer* sample=currentSample();

        if(sample)
        {
                qInfo("PadsGDX::onTuneModelChanged t=%f",
                      m_tuneModel[key]->value());
                sample->retune(m_tuneModel[key]->value());
                emit dataChanged();
        }
}


void PadsGDX::onStutterModelChanged()
{
        if(m_loading) return;
        SampleBuffer* sample=currentSample();

        if(!sample)
        {
                m_nextPlayStartPoint=sample->startFrame();
                m_nextPlayBackwards=false;
        }
}


bool PadsGDX::checkPointBounds(int _key)
{
        if(m_loading) return false;
        if(m_checking) return false;
        if(_key<0 || _key>127) return false;

        SampleBuffer* sample=m_sampleBuffer[_key];
        if(!sample) return false;

        m_checking=true;
        setJournalling(false);

        float old_start    =m_startPointModel    [_key]->value();
        float old_end      =m_endPointModel      [_key]->value();
        float old_loopStart=m_loopStartPointModel[_key]->value();
        float old_loopEnd  =m_loopEndPointModel  [_key]->value();

        const float MINSZ=0.0001f;

        // check if start is over end and swap values if so
        if( m_startPointModel[_key]->value() > m_endPointModel[_key]->value() )
        {
                float tmp = m_endPointModel[_key]->value();
                m_endPointModel[_key]->setValue( m_startPointModel[_key]->value() );
                m_startPointModel[_key]->setValue( tmp );
        }

        //m_startPointModel[_key]->setValue(qBound(0.f,m_startPointModel[_key]->value(),1.f));
        //m_endPointModel[_key]->setValue(qBound(0.f,m_endPointModel[_key]->value(),1.f));

        if( qAbs(m_startPointModel[_key]->value()-m_endPointModel[_key]->value())<MINSZ )
        {
                m_endPointModel[_key]->setValue(m_startPointModel[_key]->value()+MINSZ);
                m_startPointModel[_key]->setValue(m_endPointModel[_key]->value()-MINSZ);
                //m_startPointModel[_key]->setValue(qBound(0.f,m_startPointModel[_key]->value(),1.f));
                //m_endPointModel[_key]->setValue(qBound(0.f,m_endPointModel[_key]->value(),1.f));
        }

        // check if start is over end and swap values if so
        if( m_loopStartPointModel[_key]->value() > m_loopEndPointModel[_key]->value() )
        {
                float tmp = m_loopEndPointModel[_key]->value();
                m_loopEndPointModel[_key]->setValue( m_loopStartPointModel[_key]->value() );
                m_loopStartPointModel[_key]->setValue( tmp );
        }

        m_loopStartPointModel[_key]->setValue(qBound(m_startPointModel[_key]->value(),
                                                     m_loopStartPointModel[_key]->value(),
                                                     m_endPointModel[_key]->value()));

        m_loopEndPointModel[_key]->setValue(qBound(m_startPointModel[_key]->value(),
                                                   m_loopEndPointModel[_key]->value(),
                                                   m_endPointModel[_key]->value()));

        if( qAbs(m_loopStartPointModel[_key]->value()-m_loopEndPointModel[_key]->value())<MINSZ )
        {
                m_loopEndPointModel[_key]->setValue(m_loopStartPointModel[_key]->value()+MINSZ);
                m_loopStartPointModel[_key]->setValue(m_loopEndPointModel[_key]->value()-MINSZ);

                m_loopStartPointModel[_key]->setValue(qBound(m_startPointModel[_key]->value(),
                                                             m_loopStartPointModel[_key]->value(),
                                                             m_endPointModel[_key]->value()));

                m_loopEndPointModel[_key]->setValue(qBound(m_startPointModel[_key]->value(),
                                                           m_loopEndPointModel[_key]->value(),
                                                           m_endPointModel[_key]->value()));
        }

        setJournalling(true);
        m_checking=false;

        return  (old_start    !=m_startPointModel    [_key]->value()) ||
                (old_end      !=m_endPointModel      [_key]->value()) ||
                (old_loopStart!=m_loopStartPointModel[_key]->value()) ||
                (old_loopEnd  !=m_loopEndPointModel  [_key]->value());
}


void PadsGDX::onStartPointChanged( void )
{
        onPointChanged();
}


void PadsGDX::onEndPointChanged()
{
        onPointChanged();
}


void PadsGDX::onLoopStartPointChanged()
{
        onPointChanged();
}

void PadsGDX::onLoopEndPointChanged()
{
        onPointChanged();
}

void PadsGDX::onPointChanged()
{
        if(m_loading) return;

        int key=currentKey();
        if(key<0 || key>127) return;

        checkPointBounds(key);

        SampleBuffer* sample=currentSample();
        //qInfo("PadsGDX::onPointChanged sample=%p",sample);
        if(!sample) return;

        const int lastFrame=sample->frames()-1;
        f_cnt_t f_start    =static_cast<f_cnt_t>( m_startPointModel    [key]->value()*lastFrame);
        f_cnt_t f_end      =static_cast<f_cnt_t>( m_endPointModel      [key]->value()*lastFrame);
        f_cnt_t f_loopStart=static_cast<f_cnt_t>( m_loopStartPointModel[key]->value()*lastFrame);
        f_cnt_t f_loopEnd  =static_cast<f_cnt_t>( m_loopEndPointModel  [key]->value()*lastFrame);

	m_nextPlayStartPoint=f_start;
	m_nextPlayBackwards =false;

        sample->setAllPointFrames(f_start,f_end,f_loopStart,f_loopEnd);
        //qInfo("POINTS s=%d e=%d ls=%d le=%d",f_start,f_end,f_loopStart,f_loopEnd);

	emit dataChanged();
}


void PadsGDX::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
	QDomElement samples=_doc.createElement("samples");
	_this.appendChild(samples);
        samples.setAttribute("key",currentKey());
	for(int i=0;i<128;i++)
        {
                SampleBuffer* sample=m_sampleBuffer[i];
                if(sample==NULL) continue;

                QDomElement e=_doc.createElement("sample");
                samples.appendChild(e);
                e.setAttribute("key",i);

                QString f=sample->audioFile();
                if(f=="")
                {
                        QString s;
                        e.setAttribute("data",sample->toBase64(s));
                }
                else
                {
                        f=SampleBuffer::tryToMakeRelative(f);
                        e.setAttribute("src",f);
                }

                //if(m_semitones[i]!=0.)
                //        e.setAttribute("semitones",m_semitones[i]);
                m_tuneModel           [i]->saveSettings( _doc, e, "semitones" );
                m_reverseModel        [i]->saveSettings( _doc, e, "reversed" );
                m_loopModel           [i]->saveSettings( _doc, e, "looped" );
                m_ampModel            [i]->saveSettings( _doc, e, "amp" );
                m_stutterModel        [i]->saveSettings( _doc, e, "stutter" );
                //m_interpolationModel[i]->saveSettings( _doc, e, "interp" );
                m_startPointModel     [i]->saveSettings( _doc, e, "start" );
                m_endPointModel       [i]->saveSettings( _doc, e, "end" );
                m_loopStartPointModel [i]->saveSettings( _doc, e, "loopstart" );
                m_loopEndPointModel   [i]->saveSettings( _doc, e, "loopend" );
        }
}




void PadsGDX::loadSettings( const QDomElement & _this )
{
        m_loading=true;
	QDomNode samples=_this.firstChildElement("samples");
	if( !samples.isNull() && samples.isElement() )
        {
		QDomElement e=samples.firstChildElement("sample");
		while(!e.isNull())
		{
                        int i=e.attribute("key").toInt();
                        if(i<0 || i>127)
                        {
                                //qInfo("PadsGDX::loadSettings invalid key=%d",i);
                                e=e.nextSibling().toElement();
                                continue;
                        }

                        destroyKey(i);

                        double semitones=e.attribute("semitones").toDouble();
                        QString     file=e.attribute("src");
                        QString     data=e.attribute("data");

                        if(file!="")
                        {
                                QString p=SampleBuffer::tryToMakeAbsolute(file);
                                if(!QFileInfo(p).exists())
                                {
                                        QString message=tr("Sample not found: %1").arg(file);
                                        Engine::getSong()->collectError(message);
                                        e=e.nextSibling().toElement();
                                        continue;
                                }
                        }
                        else
                        if(data=="")
                        {
                                e=e.nextSibling().toElement();
                                continue;
                        }

                        SampleBuffer* sample=NULL;
                        if(file!="")
                        {
                                sample=new SampleBuffer(file);
                        }
                        else
                        if(data!="")
                        {
                                sample=new SampleBuffer();
                                sample->loadFromBase64(data);
                        }
                        else
                        {
                                e=e.nextSibling().toElement();
                                continue;
                        }

                        createKey(i,sample,semitones);

                        m_ampModel            [i]->loadSettings( _this, "amp" );
                        m_tuneModel           [i]->loadSettings( _this, "semitones" );
                        m_reverseModel        [i]->loadSettings( _this, "reversed" );
                        m_loopModel           [i]->loadSettings( _this, "looped" );
                        m_stutterModel        [i]->loadSettings( _this, "stutter" );
                        //m_interpolationModel[i]->loadSettings( _this, "interp" );
                        m_startPointModel     [i]->loadSettings( _this, "start" );
                        m_endPointModel       [i]->loadSettings( _this, "end" );
                        m_loopStartPointModel [i]->loadSettings( _this, "loopStart" );
                        m_loopEndPointModel   [i]->loadSettings( _this, "loopEnd" );

                        /*
                        if(!m_sampleBuffer[i])
                        {
                                m_sampleBuffer[i]=sample;
                                connect( sample, SIGNAL( sampleUpdated() ),
                                         this, SLOT( onSampleUpdated() ) );
                        }
                        */

                        e=e.nextSibling().toElement();
                }

                m_loading=false;
                int k=samples.toElement().attribute("key").toInt();
                if(k!=currentKey())
                        setCurrentKey(k);
                else
                {
                        onSampleUpdated();
                        onPointChanged();
                }
        }
}


const QString PadsGDX::SFZFile()
{
        return m_SFZFile;
}


void PadsGDX::loadSFZ(const QString& _file)
{
        m_SFZFile=_file;

        QFile in(_file);
        if(!in.open(QIODevice::ReadOnly))
        {
                qWarning("PadsGDX::loadSFZ can not open file %s",
                         _file.toUtf8().constData());
                in.close();
        }
        else
        {
                QTextStream tin(&in);
                QString s=tin.readAll();
                QRegExp rx2("\n[ \r\t\f\n]*\n+");
                s.replace(rx2,"\n");
                QRegExp rx1("(\\s|^)/[^\n]*\n*");
                s.replace(rx1,"\n");
                s=s.trimmed();
                //qInfo("[S]%s\n---\n",s.toUtf8().constData());
                QStringList groups=s.split("<group>");
                foreach(QString g,groups)
                {
                        g=g.trimmed();
                        //qInfo("[G]%s\n---",g.toUtf8().constData());
                        QStringList regions=g.split("<region>");
                        bool first=true;
                        QHash<QString,QString> tg;

                        foreach(QString r,regions)
                        {
                                QHash<QString,QString> tr;
                                r=r.trimmed();
                                qInfo("[R]%s\n---",r.toUtf8().constData());
                                QStringList opcodes=r.split(QRegExp("\\b(?=[_a-z]+=)"));
                                foreach(QString o,opcodes)
                                {
                                        o=o.trimmed();
                                        if(o=="") continue;
                                        qInfo("[O]%s\n---",o.toUtf8().constData());
                                        QStringList elements=o.split("=");
                                        /*
                                        foreach(QString e,elements)
                                        {
                                                e=e.trimmed();
                                                qWarning("[E]%s",e.toUtf8().constData());
                                        }
                                        */
                                        if(elements.size()!=2)
                                        {
                                                qWarning("SFZ bad format: %s",o.toUtf8().constData());
                                                continue;
                                        }
                                        QString k=elements[0];
                                        QString v=elements[1];
                                        /*
                                        qWarning("k[%s]=v[%s]",
                                                 k.toUtf8().constData(),
                                                 v.toUtf8().constData());
                                        */
                                        if(v=="") continue;
                                        if(first) tg.insert(k,v);
                                        else      tr.insert(k,v);
                                }

                                if(first) { first=false; continue; }

                                foreach(QString k,tg.keys())
                                        tr.insert(k,tg.value(k));

                                QString f_sample("");
                                int     f_lokey=-1;
                                int     f_hikey=-1;
                                int     f_pitch_keycenter=-1;
                                int     f_transpose=0;
                                int     f_tune=0;
                                //lovel=0 hivel=127 tune=0

                                foreach(QString k,tr.keys())
                                {
                                        QString v=tr.value(k);
                                        if(k=="sample"         ) f_sample         =v;
                                        if(k=="lokey"          ) f_lokey          =Note::findKeyNum(v);
                                        if(k=="hikey"          ) f_hikey          =Note::findKeyNum(v);
                                        if(k=="pitch_keycenter") f_pitch_keycenter=Note::findKeyNum(v);
                                        if(k=="transpose"      ) f_transpose      =v.toInt();
                                        if(k=="tune"           ) f_tune           =v.toInt();
                                }

                                if(f_sample=="") continue;
                                f_sample.replace('\\','/');
                                QFileInfo fi(QFileInfo(QFileInfo(in).dir(),f_sample));
                                if(!fi.exists())
                                {
                                        qWarning("PadsGDX: sample file not found: %s",
                                                 fi.absoluteFilePath().toUtf8().constData());
                                        continue;
                                }
                                if((f_lokey==-1)||(f_hikey==-1)||(f_pitch_keycenter==-1)||
                                   (f_lokey>f_hikey)||(f_lokey<0)||(f_hikey>127))
                                {
                                        qWarning("PadsGDX: info missing: lo=%d hi=%d kc=%d",
                                                 f_lokey,f_hikey,f_pitch_keycenter);
                                        continue;
                                }
                                for(int i=f_lokey;i<=f_hikey;++i)
                                {
                                        createKey(i,fi.absoluteFilePath(),
                                                  f_transpose+f_tune/100.+i-f_pitch_keycenter);
                                }
                                qInfo("[end of region]\n");
                                //first=false;
                        }
                        qInfo("[end of group]\n");
                }
                in.close();
        }
}


void PadsGDX::saveSFZ(const QString& _file)
{
        m_SFZFile=_file;

        QFile out(_file);
        if(!out.open(QIODevice::WriteOnly))
        {
                out.close();
        }
        else
        {
                QString s=QString("// %1\n").arg(_file);
                out.write(s.toUtf8());

                s=QString("\n\n<group>\n\n");
                //loop_mode=one_shot\nseq_position=1\n"
                //        "seq_length=2\nampeg_sustain=0\nampeg_decay=1\nampeg_vel2decay=10\n");
                out.write(s.toUtf8());

                for(int i=0;i<128;i++)
                {
                        SampleBuffer* sample=m_sampleBuffer[i];
                        if(sample==NULL) continue;

                        QString f=sample->audioFile();
                        if(f=="")
                        {
                                continue;
                                //QString s;
                                //e.setAttribute("data",sample->toBase64(s));
                        }
                        else
                        {
                                //TODO relative to sfz file
                                f=SampleBuffer::tryToMakeRelative(f);
                                //e.setAttribute("src",f);
                        }

                        /*
                        m_reverseModel        [i]->saveSettings( _doc, e, "reversed" );
                        m_loopModel           [i]->saveSettings( _doc, e, "looped" );
                        m_ampModel            [i]->saveSettings( _doc, e, "amp" );
                        m_stutterModel        [i]->saveSettings( _doc, e, "stutter" );
                        //m_interpolationModel[i]->saveSettings( _doc, e, "interp" );
                        m_startPointModel     [i]->saveSettings( _doc, e, "start" );
                        m_endPointModel       [i]->saveSettings( _doc, e, "end" );
                        m_loopStartPointModel [i]->saveSettings( _doc, e, "loopstart" );
                        m_loopEndPointModel   [i]->saveSettings( _doc, e, "loopend" );
                        */

                        double semitones=m_tuneModel[i]->value();

                        s=QString("\n<region>"
                                  "\nsample=%1"
                                  "\nlokey=%2\nhikey=%2\npitch_keycenter=%2"
                                  "\ntranspose=%3"
                                  "\ntune=%4"
                                  "\n")
                                .arg(f)
                                .arg(Note::findKeyName(i))
                                .arg((int)(semitones))
                                .arg((int)(100.*(semitones-(int)semitones)));
                        out.write(s.toUtf8());
                }

                //QTextStream out(&file); out << mystring;
                out.close();
        }

        /*
<group>
loop_mode=one_shot
seq_position=1
seq_length=2
ampeg_sustain=0
ampeg_decay=1
ampeg_vel2decay=10

<region>
sample=Samples\1st Violins\1st-violins-piz-rr1-g3.wav
lokey=g3
hikey=g#3
pitch_keycenter=g3
        */
}


extern "C"
{

// necessary for getting instance out of shared lib
Plugin * PLUGIN_EXPORT lmms_plugin_main( Model *, void * _data )
{
	return new PadsGDX(static_cast<InstrumentTrack*>( _data ));
}

}
