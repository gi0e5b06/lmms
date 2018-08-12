/*
 * MidiAlsaGdx.h - an ALSA sequencer client with 16 ports and multiplexing
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

#ifndef MIDI_ALSA_GDX_H
#define MIDI_ALSA_GDX_H

#include "lmmsconfig.h"

#ifdef LMMS_HAVE_ALSA
#include <alsa/asoundlib.h>

#include <QMutex>
#include <QThread>
#include <QTimer>

#include "MidiClient.h"


struct pollfd;
//class QLineEdit;


class MidiAlsaGdx : public QThread, public MidiClient, public MidiEventProcessor
{
	Q_OBJECT
public:
	MidiAlsaGdx();
	virtual ~MidiAlsaGdx();

	static QString probeDevice();


	inline static QString name()
	{
		return QT_TRANSLATE_NOOP( "MidiSetupWidget",
					  "ALSA GDX-MIDI (multiplexing sequencer)" );
	}

	inline static QString configSection()
	{
		return "Midialsagdx";
	}



	virtual void processOutEvent( const MidiEvent & _me,
				      const MidiTime & _time,
				      const MidiPort * _port );

	virtual void applyPortMode( MidiPort * _port );
	virtual void applyPortName( MidiPort * _port );

	virtual void removePort( MidiPort * _port );


	// list seq-ports from ALSA
	virtual QStringList readablePorts() const
	{
		return m_readablePorts;
	}

	virtual QStringList writablePorts() const
	{
		return m_writablePorts;
	}

	// return name of port which specified MIDI event came from
	virtual QString sourcePortName( const MidiEvent & ) const;

	// (un)subscribe given MidiPort to/from destination-port
	virtual void subscribeReadablePort( MidiPort * _port,
						const QString & _dest,
						bool _subscribe = true );
	virtual void subscribeWritablePort( MidiPort * _port,
						const QString & _dest,
						bool _subscribe = true );
	virtual void connectRPChanged( QObject * _receiver,
							const char * _member )
	{
		connect( this, SIGNAL( readablePortsChanged() ),
							_receiver, _member );
	}

	virtual void connectWPChanged( QObject * _receiver,
							const char * _member )
	{
		connect( this, SIGNAL( writablePortsChanged() ),
							_receiver, _member );
	}


private slots:
	void changeQueueTempo( bpm_t _bpm );
	void updatePortList();


private:
	virtual void run();

#ifdef LMMS_HAVE_ALSA
	QMutex m_seqMutex;
	snd_seq_t * m_seqHandle;
	//snd_seq_addr_t getAddr(const QString& s);
	//snd_seq_addr_t getAddr(const MidiPort* port);
	int getFD(const MidiPort* port,int i);
	void setFD(const MidiPort* port,int i,int v);
	void SHOW_MIDI_MAP(); //tmp

	MidiPort* s_lmmsPorts[16];
	void createLmmsPorts();
	void destroyLmmsPorts();
	virtual void processInEvent( const MidiEvent& event, const MidiTime& time = MidiTime(), f_cnt_t offset = 0 );
	virtual void processOutEvent( const MidiEvent& event, const MidiTime& time = MidiTime(), f_cnt_t offset = 0 );
	QString m_clientName;
	int m_clientID;
	int m_queueID;

	void updateAlsaPortList();
	void alsaConnectOut(int _pout,QString _pextin);
	void alsaConnectIn(int _pin,QString _pextout);
	void alsaConnect(const snd_seq_addr_t& sender, const snd_seq_addr_t& dest);
	QString alsaPortName(snd_seq_client_info_t* _cinfo, snd_seq_port_info_t* _pinfo) const;
	QString alsaPortName(const snd_seq_addr_t& _addr) const;
#endif

	volatile bool m_quit;

	QTimer m_portListUpdateTimer;
	QStringList m_readablePorts;
	QStringList m_writablePorts;

	int m_pipe[2];


signals:
	void readablePortsChanged();
	void writablePortsChanged();

} ;

#endif

#endif

