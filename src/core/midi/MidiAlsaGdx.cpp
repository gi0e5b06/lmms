/*
 * MidiAlsaGdx.cpp - an ALSA sequencer client with 16 ports and multiplexing
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

#include "MidiAlsaGdx.h"

#include "ConfigManager.h"
#include "Engine.h"
//#include "Mixer.h"
//#include "gui_templates.h"
#include "MidiPort.h"
#include "Song.h"
//#include "Note.h"

//#include "debug.h"
//#include "Backtrace.h"
//#include "PerfLog.h"

//#include <QRegExp>
#include <QMultiHash>
//#include <QQueue>

#ifdef LMMS_HAVE_ALSA

const int EventPollTimeOut = 50;  // 5 //250

QString MidiAlsaGdx::alsaPortName(snd_seq_client_info_t* _cinfo,
                                  snd_seq_port_info_t*   _pinfo) const
{
    int     cid = snd_seq_port_info_get_client(_pinfo);
    QString cnm = snd_seq_client_info_get_name(_cinfo);

    // always use 128 and LMMS for internal ports
    if(cid == m_clientID)
        cid = 128;
    if(cnm == m_clientName)
        cnm = "LMMS";

    return QString("%1:%2 %3:%4")
            .arg(cid)
            .arg(snd_seq_port_info_get_port(_pinfo))
            .arg(cnm)
            .arg(snd_seq_port_info_get_name(_pinfo));

    /*
    QString cn(snd_seq_client_info_get_name( _cinfo ));
    QString pn(snd_seq_port_info_get_name( _pinfo ));
    if(pn.indexOf(cn)==0) pn=pn.remove(0,cn.size()).trimmed();
    if(pn.indexOf("MIDI ")==0) pn=pn.remove(0,5).trimmed();
    return QString("%1:%2 %3:%4").
            arg(snd_seq_port_info_get_client( _pinfo ),3,10,'0').
            arg(snd_seq_port_info_get_port( _pinfo )  ,2,10,'0').
            arg( cn ).
            arg( pn );
    */
}

QString MidiAlsaGdx::alsaPortName(
        const snd_seq_addr_t& _addr) const  // snd_seq_t * _seq,
{
    snd_seq_client_info_t* cinfo;
    snd_seq_port_info_t*   pinfo;

    snd_seq_client_info_malloc(&cinfo);
    snd_seq_port_info_malloc(&pinfo);

    snd_seq_get_any_port_info(m_seqHandle, _addr.client, _addr.port, pinfo);
    snd_seq_get_any_client_info(m_seqHandle, _addr.client, cinfo);

    const QString name = alsaPortName(cinfo, pinfo);

    snd_seq_client_info_free(cinfo);
    snd_seq_port_info_free(pinfo);

    return name;
}

/*
QString MidiAlsaGdx::gdxPortName(int _port)
{
        return QString("%1:%2 %3:%4")
                .arg(128)
                .arg(_port)
                .arg("LMMS")
                .
}
*/

static QMultiHash<QString, QString> s_alsaReadablePorts;
static QMultiHash<QString, QString> s_alsaWritablePorts;

MidiAlsaGdx::MidiAlsaGdx() :
      MidiClient(), m_seqMutex(), m_seqHandle(nullptr), m_queueID(-1),
      m_quit(false), m_portListUpdateTimer(this)
{
    setObjectName("midi alsa gdx");

    int err;
    if((err = snd_seq_open(&m_seqHandle, probeDevice().toLatin1().constData(),
                           SND_SEQ_OPEN_DUPLEX, 0))
       < 0)
    {
        qCritical("MidiAlsaGdx: Cannot open sequencer: %s",
                  snd_strerror(err));
        return;
    }

    m_clientID = 128;
    snd_seq_client_info_t* cinfo;
    snd_seq_client_info_malloc(&cinfo);
    if(snd_seq_get_client_info(m_seqHandle, cinfo))
        qWarning("MidiAlsaGdx: snd_seq_get_client_info failed");
    else
        m_clientID = snd_seq_client_info_get_client(cinfo);
    snd_seq_client_info_free(cinfo);

    m_queueID = snd_seq_alloc_queue(m_seqHandle);
    snd_seq_queue_tempo_t* tempo;
    snd_seq_queue_tempo_malloc(&tempo);
    snd_seq_queue_tempo_set_tempo(tempo,
                                  60000000 / Engine::getSong()->getTempo());
    snd_seq_queue_tempo_set_ppq(tempo, 48);
    snd_seq_set_queue_tempo(m_seqHandle, m_queueID, tempo);
    snd_seq_queue_tempo_free(tempo);

    snd_seq_start_queue(m_seqHandle, m_queueID, nullptr);
    changeQueueTempo(Engine::getSong()->getTempo());
    connect(Engine::getSong(), SIGNAL(tempoChanged(bpm_t)), this,
            SLOT(changeQueueTempo(bpm_t)));

    updateAlsaPortList();

    int n = 1;
    do
    {
        if(n == 1)
            m_clientName = "LMMS";
        else
            m_clientName = QString("LMMS%1").arg(n);
        n++;
    } while(s_alsaReadablePorts.contains(m_clientName)
            || s_alsaWritablePorts.contains(m_clientName));

    snd_seq_set_client_name(m_seqHandle, qPrintable(m_clientName));

    createLmmsPorts();
    updatePortList();

    // connect( &m_portListUpdateTimer, SIGNAL( timeout() ),
    //	 this, SLOT( updatePortList() ) );
    // we check for port-changes every second
    // m_portListUpdateTimer.start( 1000 );

    // use a pipe to detect shutdown
    if(pipe(m_pipe) == -1)
    {
        qCritical("MidiAlsaGdx: pipe");
    }

    start(QThread::LowPriority);  // IdlePriority );
    // start( QThread::HighestPriority ); //GDX
}

MidiAlsaGdx::~MidiAlsaGdx()
{
    qInfo("MidiAlsaGdx::~MidiAlsaGdx 1");
    destroyLmmsPorts();
    qInfo("MidiAlsaGdx::~MidiAlsaGdx 2");

    if(isRunning())
    {
        m_quit = true;
        wait(EventPollTimeOut * 2);

        m_seqMutex.lock();
        snd_seq_stop_queue(m_seqHandle, m_queueID, nullptr);
        snd_seq_free_queue(m_seqHandle, m_queueID);
        snd_seq_close(m_seqHandle);
        m_seqMutex.unlock();
    }

    qInfo("MidiAlsaGdx::~MidiAlsaGdx 3");
}

QString MidiAlsaGdx::probeDevice()
{
    QString dev = ConfigManager::inst()->value("Midialsagdx", "device");
    if(dev.isEmpty())
    {
        if(getenv("MIDIDEV") != nullptr)
        {
            return getenv("MIDIDEV");
        }
        return "default";
    }
    return dev;
}

// static QHash<const MidiPort*,snd_seq_addr_t> m_Addr;
static QHash<const MidiPort*, int> m_FD0;
static QHash<const MidiPort*, int> m_FD1;

uint qHash(const MidiPort*& a)
{
    return (uint)(((unsigned long long)a) & 0xFFFFFFFFl);
}

/*
snd_seq_addr_t MidiAlsaGdx::getAddr(const QString& _s)
{
        snd_seq_addr_t r;
        if( !_s.isNull() &&
            !_s.isEmpty() &&
            snd_seq_parse_address( m_seqHandle, &r,
                                   _s.section(' ',0,0).toLatin1().constData()
) ) qFatal("MidiAlsaGdx: Error#1 parsing address '%s'",qPrintable(_s));

        qWarning("getAddr %d:%d",r.client,r.port);
        return r;
}
*/

int MidiAlsaGdx::getFD(const MidiPort* _port, int _i)
{
    if(_port == nullptr)
    {
        qFatal("MidiAlsaGdx::getFD port is null");
    }

    switch(_i)
    {
        case 1:
            if(m_FD1.contains(_port) && (m_FD1.value(_port) != -1))
                return m_FD1.value(_port);
            else
                return -1;
        case 0:
            if(m_FD0.contains(_port))
                return m_FD0.value(_port);
            else
                return -1;
        default:
            qFatal("MidiAlsaGdx::getFD invalid i=%d", _i);
    }
}

void MidiAlsaGdx::setFD(const MidiPort* _port, int _i, int _v)
{
    if(_port == nullptr)
    {
        qFatal("MidiAlsaGdx::setFD port is null");
    }

    switch(_i)
    {
        case 1:
            if(_v == -1)
                m_FD1.remove(_port);
            else
                m_FD1.insert(_port, _v);
            break;
        case 0:
            if(_v == -1)
                m_FD0.remove(_port);
            else
                m_FD0.insert(_port, _v);
            break;
        default:
            qFatal("MidiAlsaGdx::setFD invalid i=%d", _i);
    }

    assert(getFD(_port, _i) == _v);
}

static QString TYPICAL_CLIENT_NAME[16]
        = {"MPKmini2", "Inst2",        "APC MINI",        "Inst4",
           "Mixxx",    "LMMS:ch5",     "Inst7",           "Inst8",
           "Inst9",    "MPD218",       "Inst11",          ":MTC",
           "MIDI Mix", "Midi Through", "System:Announce", "System:Timer"};
void MidiAlsaGdx::createLmmsPorts()
{
    for(int _num = 0; _num < 16; _num++)
    {
        QString name = QString("LMMS %1").arg(
                TYPICAL_CLIENT_NAME[_num].replace(':', ' ').trimmed());
        s_lmmsPorts[_num]
                = new MidiPort(name, this, this, nullptr, MidiPort::Duplex);
    }

    for(int _num = 0; _num < 16; _num++)
    {
        int     caps = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
        QString name = QString("out to %1")
                               .arg(TYPICAL_CLIENT_NAME[_num]
                                            .replace(':', ' ')
                                            .trimmed());
        int pout = snd_seq_create_simple_port(
                m_seqHandle, qPrintable(name), caps,
                SND_SEQ_PORT_TYPE_MIDI_GENERIC
                        | SND_SEQ_PORT_TYPE_APPLICATION);
        if(pout < 0)
        {
            qFatal("Fail to create alsa out port %d:%d", m_clientID, pout);
            setFD(s_lmmsPorts[_num], 1, -1);
        }
        else
        {
            // qInfo("Alsa out port created %d:%d",m_clientID,pout);
            setFD(s_lmmsPorts[_num], 1, pout);
            alsaConnectOut(pout, TYPICAL_CLIENT_NAME[_num]);
        }
    }

    for(int _num = 0; _num < 16; _num++)
    {
        int     caps = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;
        QString name = QString("in from %1")
                               .arg(TYPICAL_CLIENT_NAME[_num]
                                            .replace(':', ' ')
                                            .trimmed());
        int pin = snd_seq_create_simple_port(
                m_seqHandle, qPrintable(name), caps,
                SND_SEQ_PORT_TYPE_MIDI_GENERIC
                        | SND_SEQ_PORT_TYPE_APPLICATION);
        if(pin < 0)
        {
            qFatal("Fail to create alsa in port %d:%d", m_clientID, pin);
            setFD(s_lmmsPorts[_num], 0, -1);
        }
        else
        {
            // qInfo("Alsa in port created %d:%d",m_clientID,pin);
            setFD(s_lmmsPorts[_num], 0, pin);
            alsaConnectIn(pin, TYPICAL_CLIENT_NAME[_num]);
        }
    }
}

void MidiAlsaGdx::destroyLmmsPorts()
{
    for(int _num = 0; _num < 16; _num++)
    {
        int pout = getFD(s_lmmsPorts[_num], 1);
        if((pout != -1)
           && (snd_seq_delete_simple_port(m_seqHandle, pout) < 0))
            qWarning("MidiAlsaGdx::destroy fail pout=%d", pout);
        setFD(s_lmmsPorts[_num], 1, -1);

        int pin = getFD(s_lmmsPorts[_num], 0);
        if((pin != -1) && (snd_seq_delete_simple_port(m_seqHandle, pin) < 0))
            qWarning("MidiAlsaGdx::destroy fail pin=%d", pin);
        setFD(s_lmmsPorts[_num], 0, -1);

        DELETE_HELPER(s_lmmsPorts[_num]);
        // delete s_lmmsPorts[_num];
        // s_lmmsPorts[_num] = nullptr;
    }
}

void MidiAlsaGdx::alsaConnectOut(int _pout, QString _pextin)
{
    qInfo("MidiAlsaGdx::alsaConnectOut %d %s", _pout, qPrintable(_pextin));

    // 20:0 MIDI Mix:MIDI Mix MIDI 1
    // MIDI Mix:MIDI Mix MIDI 1
    // MIDI Mix
    // MIDI Mix MIDI 1

    if(!s_alsaReadablePorts.contains(_pextin))
    {
        for(QString s: s_alsaReadablePorts.keys())
            if(s.indexOf(_pextin) >= 0)
            {
                _pextin = s;
                break;
            }
    }

    if(!s_alsaReadablePorts.contains(_pextin))
    {
        // qWarning("MidiAlsaGdx::alsaConnectOut %s is not an input
        // port",qPrintable(_pextin));
        return;
    }

    snd_seq_addr_t sender;
    sender.client = m_clientID;
    sender.port   = _pout;

    snd_seq_addr_t dest;
    if(snd_seq_parse_address(m_seqHandle, &dest,
                             s_alsaReadablePorts.value(_pextin)
                                     .section(' ', 0, 0)
                                     .toLatin1()
                                     .constData()))
    {
        qWarning("MidiAlsaGdx: Error parsing dest-address %s",
                 qPrintable(_pextin));
        return;
    }

    alsaConnect(sender, dest);
}

void MidiAlsaGdx::alsaConnectIn(int _pin, QString _pextout)
{
    // qInfo("MidiAlsaGdx::alsaConnectIn %d %s",_pin,qPrintable(_pextout));

    if(!s_alsaWritablePorts.contains(_pextout))
    {
        for(QString s: s_alsaWritablePorts.keys())
            if(s.indexOf(_pextout) >= 0)
            {
                _pextout = s;
                break;
            }
    }

    if(!s_alsaWritablePorts.contains(_pextout))
    {
        // qWarning("MidiAlsaGdx::alsaConnectIn %s is not an output
        // port",qPrintable(_pextout));
        return;
    }

    snd_seq_addr_t dest;
    dest.client = m_clientID;
    dest.port   = _pin;

    snd_seq_addr_t sender;
    if(snd_seq_parse_address(m_seqHandle, &sender,
                             s_alsaWritablePorts.value(_pextout)
                                     .section(' ', 0, 0)
                                     .toLatin1()
                                     .constData()))
    {
        qWarning("MidiAlsaGdx: Error parsing sender-address %s",
                 qPrintable(_pextout));
        return;
    }

    alsaConnect(sender, dest);
}

void MidiAlsaGdx::alsaConnect(const snd_seq_addr_t& sender,
                              const snd_seq_addr_t& dest)
{
    m_seqMutex.lock();

    // qInfo("MidiAlsaGdx::alsaConnect from %u:%u to %u:%u",
    //      sender.client,sender.port,dest.client,dest.port);
    snd_seq_port_subscribe_t* subs;
    snd_seq_port_subscribe_malloc(&subs);
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);
    if(snd_seq_subscribe_port(m_seqHandle, subs))
        qWarning("MidiAlsaGdx::alsaConnect Failed to subscribe");
    snd_seq_port_subscribe_free(subs);

    m_seqMutex.unlock();
}

void MidiAlsaGdx::applyPortMode(MidiPort* _port)
{
    // qInfo("MidiAlsaGdx::applyPortMode %p mode=%d
    // name=%s",_port,_port->mode(),qPrintable(_port->displayName()));

    /*
      if(!m_Addr.contains(_port))
    {
            qFatal("MidiAlsaGdx::applyPortMode no addr found");
            return;
    }
    */

    m_seqMutex.lock();

    // snd_seq_addr_t a=m_Addr.value(_port);

    // determine port-capabilities

    /*
    unsigned int caps[2] = { 0, 0 };

    switch( _port->mode() )
    {
    case MidiPort::Duplex:
            caps[1] |= SND_SEQ_PORT_CAP_READ |
                    SND_SEQ_PORT_CAP_SUBS_READ;

    case MidiPort::Input:
            caps[0] |= SND_SEQ_PORT_CAP_WRITE |
                    SND_SEQ_PORT_CAP_SUBS_WRITE;
            break;

    case MidiPort::Output:
            caps[1] |= SND_SEQ_PORT_CAP_READ |
                    SND_SEQ_PORT_CAP_SUBS_READ;
            break;

    default:
            break;
    }

    for( int i = 0; i < 2; ++i )
    {
            int p=getFD(_port,i);
            qInfo("caps=%d [i=%d, p=%d:%d]",caps[i],i,m_clientID,p);

            if( caps[i] != 0 )
            {
                    // no port there yet?
                    if(p==-1)// m_portIDs[_port][i] == -1 )
                    {
                            //qInfo("create port
    '%s'",_port->displayName().toUtf8().constData());
                            // then create one;
                            //m_portIDs[_port][i] =
                            int p=snd_seq_create_simple_port
                                    (m_seqHandle,
                                     _port->displayName().toUtf8().constData(),
                                     //portName(m_seqHandle,&a).toUtf8().constData(),
                                     caps[i],
                                     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                     SND_SEQ_PORT_TYPE_APPLICATION );
                            if(p<0) qFatal("Fail to create alsa port
    %d:%d",m_clientID,p); else qInfo("Alsa port created %d:%d",m_clientID,p);

                            setFD(_port,i,p);
                            qInfo("port created p=%d i=%d
    fd=%d",p,i,getFD(_port,i)); continue;
                    }
                    qInfo("port exists already p=%d i=%d
    fd=%d",p,i,getFD(_port,i)); snd_seq_port_info_t * port_info;
                    snd_seq_port_info_malloc( &port_info );
                    snd_seq_get_port_info( m_seqHandle, p, port_info );
                    snd_seq_port_info_set_capability( port_info, caps[i] );
                    snd_seq_set_port_info( m_seqHandle, p, port_info );
                    snd_seq_port_info_free( port_info );
            }
            // still a port there although no caps? ( = dummy port)
            else if(p!=-1)
            {
                    BACKTRACE
                    SHOW_MIDI_MAP();
                    qInfo("delete dummy port p=%d i=%d",p,i);
                    setFD(_port,i,-1);
                    // then remove this port
                    //QList<Key> keys(const T &value) const
                    if((m_FD0.keys(p).size()==0)&&
                       (m_FD1.keys(p).size()==0))
                            {
                                    if(snd_seq_delete_simple_port(m_seqHandle,p))
                                            qInfo("fail to delete alsa port
    %d",p); else qInfo("Alsa port %d:%d deleted",m_clientID,p);
                            }
                    SHOW_MIDI_MAP();
            }
    }
    */

    /*TMP
    int caps=SND_SEQ_PORT_CAP_READ |
            SND_SEQ_PORT_CAP_SUBS_READ |
            SND_SEQ_PORT_CAP_WRITE |
            SND_SEQ_PORT_CAP_SUBS_WRITE;

    int p0=getFD(_port,0);
    if((p0>=0)&&(_port->mode()==MidiPort::Disabled))
    {
            qInfo("delete dummy port p0=%d",p0);
            setFD(_port,0,-1);
            setFD(_port,1,-1);
            if((m_FD0.keys(p0).size()==0)&&
               (m_FD1.keys(p0).size()==0))
            {
                    if(snd_seq_delete_simple_port(m_seqHandle,p0))
                            qInfo("Fail to delete alsa port %d",p0);
                    else
                            qInfo("Alsa port %d:%d deleted",m_clientID,p0);
            }
    }

    if((p0==-1)&&(_port->mode()!=MidiPort::Disabled))
    {
            p0=snd_seq_create_simple_port
                    (m_seqHandle,
                     _port->displayName().toUtf8().constData(),
                     //portName(m_seqHandle,&a).toUtf8().constData(),
                     caps,
                     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                     SND_SEQ_PORT_TYPE_APPLICATION );
            if(p0<0) qFatal("Fail to create alsa port %d",p0);
            else qInfo("Alsa port created %d:%d",m_clientID,p0);
            setFD(_port,0,p0);
            setFD(_port,1,p0);
    }
    */

    m_seqMutex.unlock();
}

void MidiAlsaGdx::applyPortName(MidiPort* _port)
{
    // qInfo("MidiAlsaGdx::applyPortName %p mode=%d
    // name=%s",_port,_port->mode(),qPrintable(_port->displayName()));

    /*
    if(!m_Addr.contains(_port))
    {
            qFatal("MidiAlsaGdx::applyPortName no addr found");
            return;
    }
    */

    m_seqMutex.lock();

    // snd_seq_addr_t a=m_Addr.value(_port);
    /*TMP
    for( int i = 0; i < 1; ++i ) //2
    {
            int p=getFD(_port,i);
            if(p==-1 ) continue;

            snd_seq_port_info_t * port_info;
            snd_seq_port_info_malloc( &port_info );
            snd_seq_get_port_info( m_seqHandle, p, port_info );
            snd_seq_port_info_set_name( port_info,
    _port->displayName().toUtf8().constData() ); snd_seq_set_port_info(
    m_seqHandle, p, port_info ); snd_seq_port_info_free( port_info );
    }
    */
    m_seqMutex.unlock();
}

void MidiAlsaGdx::removePort(MidiPort* _port)
{
    m_seqMutex.lock();
    /*
    int p0=getFD(_port,0);
    int p1=getFD(_port,1);
    qInfo("MidiAlsaGdx::removePort p0=%d p1=%d",p0,p1);
    if(p0>=0)
    {
            if(snd_seq_delete_simple_port( m_seqHandle, p0 )<0)
                    qWarning("MidiAlsaGdx::removePort fail p0=%d",p0);
            setFD(_port,0,-1);
    }
    if(p1>=0)
    {
            if(snd_seq_delete_simple_port( m_seqHandle, p1 )<0)
                    qWarning("MidiAlsaGdx::removePort fail p1=%d",p1);
            setFD(_port,1,-1);
    }
    */
    // m_portIDs.remove( _port );
    m_seqMutex.unlock();
    // SHOW_MIDI_MAP();

    MidiClient::removePort(_port);
}

QString MidiAlsaGdx::sourcePortName(const MidiEvent& _event) const
{
    if(_event.sourcePort())
    {
        const snd_seq_addr_t addr
                = *static_cast<const snd_seq_addr_t*>(_event.sourcePort());
        // qInfo("MidiAlsaGdx::sourcePortName %u:%u",addr.client,addr.port);
        QString prefix
                = QString("%1:%2 ")
                          .arg(addr.client == m_clientID ? 128 : addr.client)
                          .arg(addr.port);
        for(QString s: m_readablePorts)
            if(s.startsWith(prefix))
                return s;
        foreach(QString s, m_writablePorts)
            if(s.startsWith(prefix))
                return s;
        return alsaPortName(addr);
    }
    return MidiClient::sourcePortName(_event);
}

bool operator==(const snd_seq_addr_t& a, const snd_seq_addr_t& b)
{
    return (a.client == b.client) && (a.port == b.port);
}

uint qHash(const snd_seq_addr_t& a)
{
    return (((uint)a.client) << 8) | ((uint)a.port);
}

// alsa port,midi port
static QMultiHash<snd_seq_addr_t, MidiPort*> m_mapReadSubs;

void MidiAlsaGdx::subscribeReadablePort(MidiPort*      _port,
                                        const QString& _dest,
                                        bool           _subscribe)
{
    if(_port == nullptr)
    {
        qWarning("MidiAlsaGdx::subscribeReadablePort port is null");
        return;
    }

    /*
    if(!m_Addr.contains(_port))//m_portIDs.contains(_port))
    {
            qWarning("MidiAlsaGdx::subscribeReadablePort port with no ID");
            return;
    }

    if(getFD(_port,0)<0)//m_portIDs[_port][0]<0)
    {
            qWarning("MidiAlsaGdx::subscribeReadablePort port with invalid
    FD"); return;
    }
    */

    /*
    if(_subscribe&&(getFD(_port,0)<0)) //m_portIDs[_port][0]<0)
    {
            qWarning("MidiAlsaGdx::subscribeReadablePort port with invalid
    FD"); return;
    }
    */

    if(_dest.section(':', 0, 0) != "128")
        qWarning("MidiAlsaGdx: Trying to connect external port");

    m_seqMutex.lock();

    snd_seq_addr_t sender;
    if(snd_seq_parse_address(m_seqHandle, &sender,
                             _dest.section(' ', 0, 0).toLatin1().constData()))
    {
        qWarning("MidiAlsaGdx: Error parsing sender-address %s",
                 qPrintable(_dest));
        m_seqMutex.unlock();
        return;
    }

    if(_subscribe)
    {
        if(m_mapReadSubs.contains(sender))
        {
            if(m_mapReadSubs.contains(sender, _port))
            {
                qWarning("MidiAlsaGdx: sender already subscribed %u:%u",
                         sender.client, sender.port);
            }
            else
            {
                // qInfo("MidiAlsaGdx: sender subscribing %u:%u
                // %p",sender.client,sender.port,_port);
                m_mapReadSubs.insert(sender, _port);
            }
            m_seqMutex.unlock();
            return;
        }
        else
            m_mapReadSubs.insert(sender, _port);
    }

    if(!_subscribe)
    {
        if(m_mapReadSubs.contains(sender))
        {
            // qInfo("MidiAlsaGdx: sender unsubscribing %u:%u
            // %p",sender.client,sender.port,_port);
            m_mapReadSubs.remove(sender, _port);
            if(m_mapReadSubs.contains(sender))
            {
                m_seqMutex.unlock();
                return;
            }
        }
        else
        {
            m_seqMutex.unlock();
            return;
        }
    }

    // qInfo("MidiAlsaGdx::subscribeReadablePort physical %u:%u
    // %p",sender.client,sender.port,_port);

    /*
      int p0=getFD(_port,0);
    qInfo("MidiAlsaGdx::subscribeReadablePort p0=%d",p0);
    if(p0<0) { m_seqMutex.unlock(); return; }

    snd_seq_port_info_t * port_info;
    snd_seq_port_info_malloc( &port_info );
    snd_seq_get_port_info( m_seqHandle, p0, port_info ); //m_portIDs[_port][0]
    const snd_seq_addr_t * dest = snd_seq_port_info_get_addr( port_info );
    qInfo("MidiAlsaGdx::subscribeReadablePort dest
    %u:%u",dest->client,dest->port); snd_seq_port_subscribe_t * subs;
    snd_seq_port_subscribe_malloc( &subs );
    snd_seq_port_subscribe_set_sender( subs, &sender );
    snd_seq_port_subscribe_set_dest( subs, dest );
    if( _subscribe )
    {
            qInfo("MidiAlsaGdx::subscribeReadablePort subscribe physical
    %u:%u",sender.client,sender.port); if(snd_seq_subscribe_port( m_seqHandle,
    subs )) qWarning("failed to subscribe");
            //else m_mapReadSubs.insert(sender,_port);
    }
    else
    {
            qInfo("MidiAlsaGdx::subscribeReadablePort unsubcribe physical
    %u:%u",sender.client,sender.port); if(snd_seq_unsubscribe_port(
    m_seqHandle, subs )) qWarning("failed to unsubscribe");
            //else m_mapReadSubs.remove(sender,_port);
    }
    snd_seq_port_subscribe_free( subs );
    snd_seq_port_info_free( port_info );
    */

    m_seqMutex.unlock();

    // qInfo("mapReadSubs.size=%d",m_mapReadSubs.size());
    // SHOW_MIDI_MAP();
}

// midi port,alsa port
static QMultiHash<MidiPort*, snd_seq_addr_t> m_mapWriteSubs;

void MidiAlsaGdx::subscribeWritablePort(MidiPort*      _port,
                                        const QString& _dest,
                                        bool           _subscribe)
{
    if(_port == nullptr)
    {
        qWarning("MidiAlsaGdx::subscribeWritablePort port is null");
        return;
    }

    qInfo("MidiAlsaGdx::subscribeWritablePort port=%p dest=%s subscribe=%d",
          _port, qPrintable(_dest), _subscribe);

    /*
    if(!m_Addr.contains(_port))//m_portIDs.contains(_port))
    {
            qWarning("MidiAlsaGdx::subscribeWritablePort port with no ID");
            return;
    }
    */

    /*
    if(getFD(_port,1)<0)
    {
            qWarning("MidiAlsaGdx::subscribeWritablePort port with invalid
    FD"); return;
    }
    */

    m_seqMutex.lock();

    snd_seq_addr_t dest;
    if(snd_seq_parse_address(m_seqHandle, &dest,
                             _dest.section(' ', 0, 0).toLatin1().constData()))
    {
        qWarning("MidiAlsaGdx: Error parsing dest-address %s",
                 qPrintable(_dest));
        m_seqMutex.unlock();
        return;
    }

    /*
    const int pid = (m_portIDs[_port][1] < 0)
            ? m_portIDs[_port][0]
            : m_portIDs[_port][1];
    if( pid < 0 )
    {
            m_seqMutex.unlock();
            return;
    }
    */

    if(_subscribe)
    {
        if(m_mapWriteSubs.contains(_port))
        {
            if(m_mapWriteSubs.contains(_port, dest))
            {
                // qWarning("MidiAlsaGdx: dest already write subscribed");
            }
            else
            {
                // qInfo("MidiAlsaGdx: dest subscribing
                // %u:%u",dest.client,dest.port);
                m_mapWriteSubs.insert(_port, dest);
            }
            m_seqMutex.unlock();
            return;
        }
        else
            m_mapWriteSubs.insert(_port, dest);
    }

    if(!_subscribe)
    {
        if(m_mapWriteSubs.contains(_port))
        {
            // qInfo("MidiAlsaGdx: dest unsubscribing
            // %u:%u",dest.client,dest.port);
            m_mapWriteSubs.remove(_port, dest);
            if(m_mapWriteSubs.contains(_port))
            {
                m_seqMutex.unlock();
                return;
            }
        }
        else
        {
            m_seqMutex.unlock();
            return;
        }
    }

    /*
    qInfo("MidiAlsaGdx::subscribeWritablePort connecting physical
    %u:%u",dest.client,dest.port); int p1=getFD(_port,1);
    qInfo("MidiAlsaGdx::subscribeWritablePort p1=%d",p1);
    if(p1<0) { m_seqMutex.unlock(); return; }

    snd_seq_port_info_t * port_info;
    snd_seq_port_info_malloc( &port_info );
    snd_seq_get_port_info( m_seqHandle, p1, port_info );
    const snd_seq_addr_t * sender = snd_seq_port_info_get_addr( port_info );
    snd_seq_port_subscribe_t * subs;
    snd_seq_port_subscribe_malloc( &subs );
    snd_seq_port_subscribe_set_sender( subs, sender );
    snd_seq_port_subscribe_set_dest( subs, &dest );
    if( _subscribe )
    {
            snd_seq_subscribe_port( m_seqHandle, subs );
            m_mapWriteSubs.insert(dest,_port);
    }
    else
    {
            snd_seq_unsubscribe_port( m_seqHandle, subs );
            m_mapWriteSubs.remove(dest);
    }
    snd_seq_port_subscribe_free( subs );
    snd_seq_port_info_free( port_info );
    */

    m_seqMutex.unlock();

    // qInfo("mapWriteSubs.size=%d",m_mapWriteSubs.size());
    // SHOW_MIDI_MAP();
}

void MidiAlsaGdx::run()
{
    // DEBUG_THREAD_PRINT

    // watch the pipe and sequencer input events
    int pollfd_count = snd_seq_poll_descriptors_count(m_seqHandle, POLLIN);
    struct pollfd* pollfd_set = new struct pollfd[pollfd_count + 1];
    snd_seq_poll_descriptors(m_seqHandle, pollfd_set + 1, pollfd_count,
                             POLLIN);
    pollfd_set[0].fd     = m_pipe[0];
    pollfd_set[0].events = POLLIN;
    ++pollfd_count;

    while(m_quit == false)
    {
        int pollRet = poll(pollfd_set, pollfd_count, EventPollTimeOut);

        if(pollRet == 0)
        {
            // QThread::usleep( 10000 );
            if(m_evqueue.isEmpty())
                continue;
        }
        else if(pollRet == -1)
        {
            // gdb may interrupt the poll
            if(errno == EINTR)
            {
                continue;
            }
            qCritical("error while polling ALSA sequencer handle");
            break;
        }
        // shutdown?
        if(m_quit)
        {
            break;
        }

        if(pollRet > 0)
        {
            // DEBUG_THREAD_PRINT
            // PL_BEGIN("AlsaSeqInput")
            m_seqMutex.lock();

            int nbe;
            // while event queue is not empty
            while(((nbe = snd_seq_event_input_pending(m_seqHandle, false))
                   > 0)
                  || ((nbe = snd_seq_event_input_pending(m_seqHandle, true))
                      > 0))
            {
                // qInfo("MidiAlsaGdx: %d events pending",nbe);

                snd_seq_event_t* ev;
                int              res;
                if((res = snd_seq_event_input(m_seqHandle, &ev)) < 0)
                {
                    // m_seqMutex.unlock();

                    if(res == -ENOSPC)
                        qCritical(
                                "MidiAlsaGdx: The input FIFO overran, and "
                                "some events are lost.");
                    else
                        qCritical(
                                "MidiAlsaGdx: Error while fetching MIDI "
                                "event from sequencer");
                    break;
                }

                // qInfo( "MidiAlsaGdx: enqueue input event %d from %d:%d to
                // %d:%d",
                //	  ev->type,ev->source.client,ev->source.port,ev->dest.client,ev->dest.port
                //);
                m_evqueue.enqueue(*ev);
            }
            m_seqMutex.unlock();
            // PL_END("AlsaSeqInput")
        }

        // qInfo("MidiAlsaGdx: dequeue size=%d",m_evqueue.size());
        if(m_evqueue.isEmpty())
            continue;

        snd_seq_event_t  sev = m_evqueue.dequeue();
        snd_seq_event_t* ev  = &sev;

        // qInfo("MidiAlsaGdx::run receive from
        // %u:%u",sev.source.client,sev.source.port);

        MidiTime         ticks  = MidiTime(ev->time.tick);
        snd_seq_addr_t*  source = &ev->dest;  // local port source;
        QList<MidiPort*> ports
                = m_mapReadSubs.values(ev->dest);  // local port source);

        // qInfo("MidiAlsaGdx::run new source is
        // %u:%u",128,source->port);//source->client qInfo("MidiAlsaGdx::run
        // %d ports found",ports.size());

        for(int j = 0; j < ports.size(); j++)
        {
            MidiPort* dest = ports[j];
            // qInfo("MidiAlsaGdx::run process event %d to %p",ev->type,dest);
            if(dest == nullptr)
                qFatal("MidiAlsaGdx::run dest is null");

            if(dest->mode() == MidiPort::Disabled)
            {
                qInfo("MidiAlsaGdx::run skip disabled port");
                continue;  // not input, not duplex
            }

            if(dest->mode() == MidiPort::Output)
            {
                qInfo("MidiAlsaGdx::run skip write-only port");
                continue;  // not input, not duplex
            }

            // CRASH LMMS --> const QString dn=dest->displayName();
            // if(!dn.isNull()) qWarning("MidiAlsaGdx: dispatching event to
            // %s",qPrintable(dn));

            switch(ev->type)
            {
                case SND_SEQ_EVENT_NOTEON:
                    qInfo("MidiAlsaGdx: noteon ch=%d key=%d val=%d time=%d",
                          ev->data.note.channel, ev->data.note.note,
                          ev->data.note.velocity, ev->time.tick);
                    dest->processInEvent(
                            MidiEvent(MidiNoteOn, ev->data.note.channel,
                                      ev->data.note.note,  //-KeysPerOctave,
                                      ev->data.note.velocity, source),
                            ticks);
                    break;

                case SND_SEQ_EVENT_NOTEOFF:
                    dest->processInEvent(
                            MidiEvent(MidiNoteOff, ev->data.note.channel,
                                      ev->data.note.note,  //-KeysPerOctave,
                                      ev->data.note.velocity, source),
                            ticks);
                    break;

                case SND_SEQ_EVENT_KEYPRESS:
                    dest->processInEvent(
                            MidiEvent(MidiKeyPressure, ev->data.note.channel,
                                      ev->data.note.note,  //-KeysPerOctave,
                                      ev->data.note.velocity, source),
                            ticks);
                    break;

                case SND_SEQ_EVENT_CONTROLLER:
                    /*
                    qInfo("MidiAlsaGdx: controller ch=%d cc=%d val=%d
                    time=%d", ev->data.control.channel,
                          ev->data.control.param,
                          ev->data.control.value,
                          ev->time.tick);
                    */
                    dest->processInEvent(MidiEvent(MidiControlChange,
                                                   ev->data.control.channel,
                                                   ev->data.control.param,
                                                   ev->data.control.value,
                                                   source),
                                         ticks);
                    break;

                case SND_SEQ_EVENT_PGMCHANGE:
                    dest->processInEvent(MidiEvent(MidiProgramChange,
                                                   ev->data.control.channel,
                                                   ev->data.control.param,
                                                   ev->data.control.value,
                                                   source),
                                         ticks);
                    break;

                case SND_SEQ_EVENT_CHANPRESS:
                    dest->processInEvent(MidiEvent(MidiChannelPressure,
                                                   ev->data.control.channel,
                                                   ev->data.control.param,
                                                   ev->data.control.value,
                                                   source),
                                         ticks);
                    break;

                case SND_SEQ_EVENT_PITCHBEND:
                {
                    int p = ev->data.control.value + 8192;
                    qInfo("MidiAlsaGdx: pitchbend ch=%d p=%d p1=%d -> %d,%d "
                          "time=%d",
                          ev->data.control.channel, p, ev->data.control.value,
                          p & 0x7F, (p >> 7) & 0x7F, ev->time.tick);
                    dest->processInEvent(
                            MidiEvent(MidiPitchBend, ev->data.control.channel,
                                      (p)&0x7F, (p >> 7) & 0x7F, source),
                            ticks);
                }
                break;

                case SND_SEQ_EVENT_SENSING:
                case SND_SEQ_EVENT_CLOCK:
                    break;

                case SND_SEQ_EVENT_PORT_SUBSCRIBED:
                    qWarning("MidiAlsaSeq: seq event port subscribed %d",
                             ev->type);
                    break;

                case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
                    qWarning("MidiAlsaSeq: seq event port unsubscribed %d",
                             ev->type);
                    break;

                default:
                    qWarning("MidiAlsaSeq: unhandled input event %d",
                             ev->type);
                    break;
            }  // end switch
        }

        // m_seqMutex.lock();

        //}	// end while

        // m_seqMutex.unlock();
    }

    delete[] pollfd_set;
}

void MidiAlsaGdx::processInEvent(const MidiEvent& event,
                                 const MidiTime&  time,
                                 f_cnt_t          offset)
{
    // processInEvent(event,time,s_lmmsPorts[event.channel()]);
}

void MidiAlsaGdx::processOutEvent(const MidiEvent& event,
                                  const MidiTime&  time,
                                  f_cnt_t          offset)
{
    processOutEvent(event, time, s_lmmsPorts[event.channel()]);
}

void MidiAlsaGdx::processOutEvent(const MidiEvent& event,
                                  const MidiTime&  time,
                                  const MidiPort*  port)
{
    if(port == nullptr)
    {
        qFatal("MidiAlsaGdx::processOutEvent port is null");
    }

    if(port->mode() == MidiPort::Disabled)
    {
        // qInfo("MidiAlsaGdx::run skip disabled port");
        return;  // not output, not duplex
    }

    if(port->mode() == MidiPort::Input)
    {
        // qInfo("MidiAlsaGdx::run skip read-only port");
        return;  // not output, not duplex
    }

    // HACK!!! - need a better solution which isn't that easy since we
    // cannot store const-ptrs in our map because we need to call non-const
    // methods of MIDI-port - it's a mess...
    MidiPort* p = const_cast<MidiPort*>(port);
    // Q_UNUSED(p);

    qInfo("  %p mode=%d name=%s", port, port->mode(),
          qPrintable(port->displayName()));
    // SHOW_MIDI_MAP();
    // int p1=getFD(port,1);
    // qInfo("MidiAlsaGdx::processOutEvent #1 p1=%d",p1);
    // if((p1==-1)&&(port->mode()==MidiPort::Duplex)) p1=getFD(port,0);
    // qInfo("MidiAlsaGdx::processOutEvent #2 p1=%d",p1);

    QList<snd_seq_addr_t> addrs = m_mapWriteSubs.values(p);
    if(addrs.size() == 0)
    {
        qInfo("MidiAlsaGdx::processOutEvent no alsa output");
        return;
    }

    foreach(const snd_seq_addr_t a, addrs)
    {
        int p1 = a.port;  // getFD(port,1);
        // qInfo(" out ev a.client=%d a.port=%d p1=%d",a.client,a.port,p1);

        if(p1 < 0)
        {
            qCritical("MidiAlsaGdx::processOutEvent bad port %d:%d", a.client,
                      a.port);
            continue;
        }

        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_source(&ev, p1);
        //( m_portIDs[p][1] != -1 ) ?	m_portIDs[p][1] : m_portIDs[p][0] );
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_schedule_tick(&ev, m_queueID, 1, static_cast<int>(time));
        ev.queue = m_queueID;
        switch(event.type())
        {
            case MidiNoteOn:
                snd_seq_ev_set_noteon(&ev, event.channel(),
                                      event.key(),  // + KeysPerOctave,
                                      event.velocity());
                break;

            case MidiNoteOff:
                snd_seq_ev_set_noteoff(&ev, event.channel(),
                                       event.key(),  // + KeysPerOctave,
                                       event.velocity());
                break;

            case MidiKeyPressure:
                snd_seq_ev_set_keypress(&ev, event.channel(),
                                        event.key(),  // + KeysPerOctave,
                                        event.velocity());
                break;

            case MidiControlChange:
                snd_seq_ev_set_controller(&ev, event.channel(),
                                          event.controllerNumber(),
                                          event.controllerValue());
                break;

            case MidiProgramChange:
                snd_seq_ev_set_pgmchange(&ev, event.channel(),
                                         event.program());
                break;

            case MidiChannelPressure:
                snd_seq_ev_set_chanpress(&ev, event.channel(),
                                         event.channelPressure());
                break;

            case MidiPitchBend:
                snd_seq_ev_set_pitchbend(&ev, event.channel(),
                                         event.midiPitchBend() - 8192);
                break;

            default:
                qWarning("MidiAlsaGdx: unhandled output event %d\n",
                         (int)event.type());
                return;
        }

        m_seqMutex.lock();
        snd_seq_event_output(m_seqHandle, &ev);
        snd_seq_drain_output(m_seqHandle);
        m_seqMutex.unlock();
    }
}

void MidiAlsaGdx::sendBytes(const uint8_t*  bytes,
                                const int       size,
                                const MidiTime& time,
                                const MidiPort* port)
{
    if(port == nullptr)
    {
        qFatal("MidiAlsaGdx::sendFourBytes port is null");
    }

    if(port->mode() == MidiPort::Disabled)
    {
        // qInfo("MidiAlsaGdx::run skip disabled port");
        return;  // not output, not duplex
    }

    if(port->mode() == MidiPort::Input)
    {
        // qInfo("MidiAlsaGdx::run skip read-only port");
        return;  // not output, not duplex
    }

    // HACK!!! - need a better solution which isn't that easy since we
    // cannot store const-ptrs in our map because we need to call non-const
    // methods of MIDI-port - it's a mess...
    MidiPort* p = const_cast<MidiPort*>(port);
    // Q_UNUSED(p);

    // qInfo("  %p mode=%d name=%s", port, port->mode(),
    //      qPrintable(port->displayName()));
    // SHOW_MIDI_MAP();
    // int p1=getFD(port,1);
    // qInfo("MidiAlsaGdx::processOutEvent #1 p1=%d",p1);
    // if((p1==-1)&&(port->mode()==MidiPort::Duplex)) p1=getFD(port,0);
    // qInfo("MidiAlsaGdx::processOutEvent #2 p1=%d",p1);

    QList<snd_seq_addr_t> addrs = m_mapWriteSubs.values(p);
    if(addrs.size() == 0)
    {
        if(port->displayName().indexOf("MTC") >= 0)
        {
            SHOW_MIDI_MAP();
            qInfo("MidiAlsaGdx::sendFourBytes no alsa output (p=%s)",
                  qPrintable(p->displayName()));
        }
        return;
    }

    foreach(const snd_seq_addr_t a, addrs)
    {
        int p1 = a.port;  // getFD(port,1);
        qInfo(" out ev a.client=%d a.port=%d p1=%d", a.client, a.port, p1);

        if(p1 < 0)
        {
            qCritical("MidiAlsaGdx::sendFourBytes bad port %d:%d", a.client,
                      a.port);
            continue;
        }

        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_source(&ev, p1);
        //( m_portIDs[p][1] != -1 ) ?	m_portIDs[p][1] : m_portIDs[p][0] );
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_schedule_tick(&ev, m_queueID, 1, static_cast<int>(time));
        ev.queue = m_queueID;

        snd_midi_event_t* dev;
        qInfo("snd_midi_event_new -> %d", snd_midi_event_new(64, &dev));
        snd_midi_event_init(dev);
        qInfo("snd_midi_event_new -> %ld",
              snd_midi_event_encode(dev, bytes, size, &ev));
        snd_midi_event_free(dev);

        m_seqMutex.lock();
        snd_seq_event_output(m_seqHandle, &ev);
        snd_seq_drain_output(m_seqHandle);
        m_seqMutex.unlock();
    }
}

void MidiAlsaGdx::changeQueueTempo(bpm_t _bpm)
{
    m_seqMutex.lock();
    snd_seq_change_queue_tempo(m_seqHandle, m_queueID, 60000000 / (int)_bpm,
                               nullptr);
    snd_seq_drain_output(m_seqHandle);
    m_seqMutex.unlock();
}

void MidiAlsaGdx::updateAlsaPortList()
{
    s_alsaWritablePorts.clear();
    s_alsaReadablePorts.clear();

    // get input- and output-ports
    snd_seq_client_info_t* cinfo;
    snd_seq_port_info_t*   pinfo;

    snd_seq_client_info_malloc(&cinfo);
    snd_seq_port_info_malloc(&pinfo);

    snd_seq_client_info_set_client(cinfo, -1);

    m_seqMutex.lock();

    while(snd_seq_query_next_client(m_seqHandle, cinfo) >= 0)
    {
        int client = snd_seq_client_info_get_client(cinfo);
        if(client == m_clientID)
            continue;

        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);
        while(snd_seq_query_next_port(m_seqHandle, pinfo) >= 0)
        {
            // we need both READ and SUBS_READ
            if((snd_seq_port_info_get_capability(pinfo)
                & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ))
               == (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ))
            {
                QString s = alsaPortName(cinfo, pinfo);
                QString p = s.section(' ', 0, 0).section(':', 1, 1);
                QString f = s.section(' ', 1);
                QString d = f.section(':', 0, 0);
                QString c = f.section(':', 1, 1);
                if(d.startsWith("LMMS"))
                    d = "LMMS";
                QString e = QString("%1:ch%2").arg(d).arg(p.toInt() % 16);
                // qInfo("Writable: s=%s f=%s d=%s c=%s e=%s",
                //	qPrintable(s),qPrintable(f),qPrintable(d),qPrintable(c),qPrintable(e));
                s_alsaWritablePorts.insert(s, s);
                s_alsaWritablePorts.insert(f, s);
                s_alsaWritablePorts.insert(d, s);
                s_alsaWritablePorts.insert(c, s);
                s_alsaWritablePorts.insert(e, s);
            }
            if((snd_seq_port_info_get_capability(pinfo)
                & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE))
               == (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE))
            {
                QString s = alsaPortName(cinfo, pinfo);
                QString p = s.section(' ', 0, 0).section(':', 1, 1);
                QString f = s.section(' ', 1);
                QString d = f.section(':', 0, 0);
                QString c = f.section(':', 1, 1);
                if(d.startsWith("LMMS"))
                    d = "LMMS";
                QString e = QString("%1:ch%2").arg(d).arg(p.toInt() % 16);
                // qInfo("Readable: s=%s f=%s d=%s c=%s e=%s",
                //	qPrintable(s),qPrintable(f),qPrintable(d),qPrintable(c),qPrintable(e));
                s_alsaReadablePorts.insert(s, s);
                s_alsaReadablePorts.insert(f, s);
                s_alsaReadablePorts.insert(d, s);
                s_alsaReadablePorts.insert(c, s);
                s_alsaReadablePorts.insert(e, s);
            }
        }
    }

    m_seqMutex.unlock();

    snd_seq_client_info_free(cinfo);
    snd_seq_port_info_free(pinfo);
}

void MidiAlsaGdx::updatePortList()
{
    QStringList readablePorts;
    QStringList writablePorts;

    for(int _num = 0; _num < 16; _num++)
    {
        MidiPort* mp = s_lmmsPorts[_num];
        readablePorts.push_back(QString("%1:%2 %3:in from %4")
                                        .arg(128)  // m_clientID)
                                        .arg(getFD(mp, 0))
                                        .arg("LMMS")  // m_clientName)
                                        .arg(TYPICAL_CLIENT_NAME[_num]
                                                     .replace(':', ' ')
                                                     .trimmed()));
        writablePorts.push_back(QString("%1:%2 %3:out to %4")
                                        .arg(128)  // m_clientID)
                                        .arg(getFD(mp, 1))
                                        .arg("LMMS")  // m_clientName)
                                        .arg(TYPICAL_CLIENT_NAME[_num]
                                                     .replace(':', ' ')
                                                     .trimmed()));
    }

    if(m_readablePorts != readablePorts)
    {
        m_readablePorts = readablePorts;
        emit readablePortsChanged();
    }

    if(m_writablePorts != writablePorts)
    {
        m_writablePorts = writablePorts;
        emit writablePortsChanged();
    }
}

void MidiAlsaGdx::SHOW_MIDI_MAP()
{
    qInfo("LMMS");
    foreach(const MidiPort* mp, m_FD0.keys())
    {
        int p0 = getFD(mp, 0);
        int p1 = getFD(mp, 1);
        qInfo("mp0 %p p0=%d p1=%d mode=%d name=%s", mp, p0, p1, mp->mode(),
              qPrintable(mp->displayName()));
    }
    foreach(const MidiPort* mp, m_FD1.keys())
    {
        if(m_FD0.contains(mp))
            continue;
        int p0 = getFD(mp, 0);
        int p1 = getFD(mp, 1);
        qInfo("mp1 %p p0=%d p1=%d mode=%d name=%s", mp, p0, p1, mp->mode(),
              qPrintable(mp->displayName()));
    }
    /*
    qInfo("ALSA");
    foreach(const QString& p, m_writablePorts)
        qInfo("writable %s", qPrintable(p));
    foreach(const QString& p, m_readablePorts)
        qInfo("readable %s", qPrintable(p));
    */
    qInfo("SUBSCRIPTIONS");
    foreach(const snd_seq_addr_t& a, m_mapReadSubs.uniqueKeys())
    {
        int n = m_mapReadSubs.values(a).size();
        qInfo("read %d:%d (%d readers)", a.client, a.port, n);
        foreach(const MidiPort* mp, m_mapReadSubs.values(a))
            qInfo("           %p %d %s", mp, mp->mode(),
                  qPrintable(mp->displayName()));
    }
    foreach(MidiPort* mp, m_mapWriteSubs.uniqueKeys())
    {
        int n = m_mapWriteSubs.values(mp).size();
        qInfo("write %p %d %s", mp, mp->mode(),
              qPrintable(mp->displayName()));
        foreach(const snd_seq_addr_t& a, m_mapWriteSubs.values(mp))
            qInfo("           %d:%d (%d writers)", a.client, a.port, n);
    }
}

#endif
