/*
 * VideoLineExport.h - support for exporting a simple video line
 *                     .mpg (M-JPEG, BPM images per 10 second)
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

#include "VideoLineExport.h"

#include "AutomationTrack.h"
#include "BBTrack.h"
#include "BBTrackContainer.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "InstrumentTrack.h"
#include "SampleTrack.h"
#include "Song.h"
#include "SongEditor.h"
#include "TimeLineWidget.h"
#include "TrackContainer.h"
#include "lmms_math.h"

#include <QApplication>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QProgressDialog>
#include <QRegExp>

extern "C"
{
    Plugin::Descriptor PLUGIN_EXPORT videolineexport_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "Video Line Export",
               QT_TRANSLATE_NOOP("pluginBrowser",
                                 "Filter for exporting M-JPEG Video lines. "
                                 "72x40@15fps. 10 minutes max."),
               "gi0e5b06",
               0x0100,
               Plugin::ExportFilter,
               nullptr,
               nullptr,
               nullptr};
}

VideoLineExport::VideoLineExport() :
      ExportFilter(&videolineexport_plugin_descriptor)
{
}

VideoLineExport::~VideoLineExport()
{
}

int VideoLineExport::posToImg(int tempo, int pos)  // 1s=1:3:8=2*48+8
{
    double ms = 60000. / tempo / 48. * pos;
    int    i  = qRound(15. / 1000. * ms);
    // qInfo("PosToImg p=%d -> i=%d",pos,i);
    return i;
}

int VideoLineExport::imgToPos(int tempo, int img)
{
    double ms = 1000. / 15. * img;
    int    p  = qRound(ms * tempo * 48. / 60000.);
    // qInfo("ImgToPos i=%d -> p=%d",img,p);
    return p;
}

bool VideoLineExport::proceed(const QString& _fileName)
{
    Tracks tracks = Engine::getSong()->tracks();
    int    tempo  = Engine::getSong()->getTempo();

    const TimeLineWidget* tl = gui->songEditor()->m_editor->timeLineWidget();

    int nTracks = 0;
    for(const Track* track: tracks)
        if(track->type() == Track::InstrumentTrack)
            nTracks++;

    // mpg is a vector of jpeg images
    const int       WI = 72;
    const int       HI = 40;
    QVector<QImage> mpg(600 * 15, QImage(WI, HI, QImage::Format_ARGB32));

    int c = 0x00;
    int d = 0x03;
    for(int i = 0; i < mpg.size(); i++)
    {
        int q = imgToPos(tempo, i);
        if((q / 768) % 2 == 1)
            c = qMax(c, 0x22);
        if(q % 768 < 32)
        {
            c = 0xFF;
            d = 0x05;
        }
        else if(q % 192 < 32)
        {
            c = 0xBF;
            d = 0x05;
        }
        // else if(q% 48<32) { c=0xBF; d=0x07; }
        mpg[i].fill(QColor(c, c, qMin(0x7F, c), 0xFF));
        c = qMax(0x00, c - d);
    }

    for(int l = 0; l < tl->NB_LOOPS; l++)
    {
        int first  = posToImg(tempo, tl->loopBegin(l));
        int second = posToImg(tempo, tl->loopEnd(l));
        for(int i = first; i <= second; i++)
        {
            QImage& img = mpg[i];

            QPainter p(&img);
            p.fillRect(WI - 13, HI - 16, 11, 14, QColor(255, 0, 0, 160));
            p.setPen(QColor(0, 0, 0, 255));
            p.setFont(QFont("monospace", 13));
            p.drawText(WI - 13, HI - 3, QChar('A' + l));
            p.drawText(WI - 12, HI - 3, QChar('A' + l));
            p.end();
        }
    }

    int rmax = 0;
    // mpg tracks
    int tn = 0;
    for(Track* track: tracks)
    {
        tn++;
        DataFile    dataFile(DataFile::SongProject);
        QColor      pc(0x00, 0x00, 0xAA, 0x99);
        QDomElement element;

        if(track->type() == Track::AutomationTrack)
        {
            AutomationTrack* autoTrack;
            autoTrack = dynamic_cast<AutomationTrack*>(track);
            element   = autoTrack->saveState(dataFile, dataFile.content());
            pc        = QColor(0xAA, 0x00, 0xAA, 0x99);
        }
        else if(track->type() == Track::InstrumentTrack)
        {
            InstrumentTrack* instTrack;
            instTrack = dynamic_cast<InstrumentTrack*>(track);
            element   = instTrack->saveState(dataFile, dataFile.content());
            pc        = QColor(0x00, 0xAA, 0x00, 0x99);
        }
        else if(track->type() == Track::SampleTrack)
        {
            SampleTrack* sampleTrack;
            sampleTrack = dynamic_cast<SampleTrack*>(track);
            element = sampleTrack->saveState(dataFile, dataFile.content());
            pc      = QColor(0xFF, 0xAA, 0x00, 0x99);
        }
        else if(track->type() == Track::BBTrack)
        {
            BBTrack* bbTrack;
            bbTrack = dynamic_cast<BBTrack*>(track);
            element = bbTrack->saveState(dataFile, dataFile.content());
            pc      = QColor(0x00, 0xAA, 0xFF, 0x99);
        }
        else
            continue;

        for(QDomNode n = element.firstChild(); !n.isNull();
            n          = n.nextSibling())
        {
            if(n.nodeName() == "pattern" || n.nodeName() == "sampletco"
               || n.nodeName() == "bbtco"
               || n.nodeName() == "automationpattern")
            {
                int base_time = n.toElement().attribute("pos", "0").toInt();
                int base_len  = n.toElement().attribute("len", "0").toInt();
                if(base_time < 0)
                    continue;
                if(base_len < 0)
                    base_len = 0;

                int r = posToImg(tempo, base_time);
                int u = posToImg(tempo, base_time + base_len) - r;
                if(r < 0)
                    r = 0;
                if(r >= mpg.size())
                    r = mpg.size() - 1;
                if(r + u > rmax)
                    rmax = r + u;
                for(int k = 0; k < 5; k++)  // tempo/10
                {
                    if(r + k >= mpg.size())
                        continue;
                    QImage&  img = mpg[r + k];
                    QPainter p(&img);
                    p.fillRect(0, 0, WI, HI, pc);
                    p.setPen(Qt::black);
                    p.setFont(QFont("monospace", 18));
                    p.drawText(5, HI - 2, QString("%1").arg(tn));
                    p.end();
                }
                for(int k = -u - WI; k < u + WI; k++)
                {
                    if(r + k < 0)
                        continue;
                    if(r + k >= mpg.size())
                        continue;
                    QImage&  img = mpg[r + k];
                    QPainter p(&img);
                    p.fillRect(10 - k, tn * 3, u, 2, pc);
                    p.fillRect(10 - k + u - 1, tn * 3, 1, 2, pc);
                    // p.setPen(Qt::black);
                    // p.setFont(QFont("monospace",18));
                    // p.drawText(5,HI-2,QString("%1").arg(tn));
                    p.end();
                }
            }
        }
    }  // for each track

    rmax += posToImg(tempo, 192);
    if(rmax >= mpg.size())
        rmax = mpg.size() - 1;
    mpg = mpg.mid(0, rmax + 1);

    int fps = 15;
    for(int i = 0; i <= rmax; i++)
    {
        QImage& img = mpg[i];

        QPainter p(&img);
        p.setPen(QColor(0xFF, 0xFF, 0x00, 0x99));
        for(int k = -10; k <= WI + 21; k++)
        {
            if(k + i < -10)
                continue;
            int q = imgToPos(tempo, i + k);
            if(i == 0)
                qInfo("i=0  k=%d q=%d", k, q);
            if(q % 192 < 7)
                p.drawLine(10 + k, 0, 10 + k, HI - 1);
            if(q % 768 < 7)
                p.drawLine(8 + k, 0, 8 + k, HI - 1);
        }

        int frm = i % fps;
        int msc = qRound(1000. * (frm / 15.));
        int sec = ((i - frm) / fps) % 60;
        int min = ((i - frm) / fps - sec) / 60;

        int q   = imgToPos(tempo, i);
        int bar = q / 192;
        int bet = (q - 192 * bar) / 48;
        int rem = q - 192 * bar - 48 * bet;

        QString ti("#%1");
        QString tt("%1:%2:%3");
        QString tb("%1-%2-%3");
        ti = ti.arg(i, 7, 10, QChar('0'));
        tt = tt.arg(min, 2, 10, QChar('0'))
                     .arg(sec, 2, 10, QChar('0'))
                     .arg(msc, 3, 10, QChar('0'));
        tb = tb.arg(bar + 1, 2, 10, QChar('-'))
                     .arg(bet + 1, 2, 10, QChar('-'))
                     .arg(rem, 3, 10, QChar('-'));
        p.setPen(Qt::white);
        p.setFont(QFont("monospace", 6));
        p.drawText(12, 8, ti);
        p.drawText(14, 15, tt);
        p.drawText(14, 22, tb);
        p.drawLine(10, 0, 10, HI);
        p.end();
    }

    qWarning("Images: %d", rmax);
    qWarning("Time: %d:%d:%d", (rmax / fps / 60), (rmax / fps) % 60,
             rmax % fps);
    qWarning("Exporting: ffmpeg");

    QStringList args;
    args << "-y"
         << "-loglevel"
         << "quiet";

    // QFile
    // audio(QString(filename).replace(QRegExp("[.][a-zA-Z0-9]+$"),".wav"));
    // if(audio.exists()) args << "-acodec" << "wav" << "-i" <<
    // audio.fileName();

    args << "-framerate"
         << "15"  // QString("%1").arg(tempo/10)
         << "-f"
         << "image2pipe"
         << "-vcodec"
         << "png"
         << "-i"
         << "-";

    args << "-acodec"
         << "libmp3lame"
         << "-vcodec"
         << "libx264" << _fileName;

    // cat *.jpg | ffmpeg -f image2pipe -r 1 -vcodec mjpeg -i - -vcodec
    // libx264 out.mp4

    QProcess& ffmpeg = *new QProcess();
    // ffmpeg.setProcessChannelMode(QProcess::ForwardedChannels);

    qInfo("QProcess ffmpeg: start");
    ffmpeg.start("/usr/bin/ffmpeg", args);
    if(!ffmpeg.waitForStarted())
        return false;

    qInfo("QProcess ffmpeg: image loop");
    for(int i = 0; i <= rmax; i++)
    {
        mpg[i].save(&ffmpeg, "PNG");
    }

    qInfo("QProcess ffmpeg: close");
    ffmpeg.closeWriteChannel();
    // qWarning("%s",qPrintable(QString(ffmpeg.readAll())));
    // ffmpeg.closeReadChannel(QProcess::StandardOutput);
    // ffmpeg.closeReadChannel(QProcess::StandardError);

    // qInfo("QProcess ffmpeg: wait");
    // if(!ffmpeg.waitForFinished(-1)) return false;

    qInfo("QProcess ffmpeg: end");
    // QByteArray result=
    // ffmpeg.readAll();

    // cat tuf_*.png | ffmpeg -framerate 24 -f image2pipe -i - ./output.mp4

    return true;
}

void VideoLineExport::error()
{
    // qDebug() << "VideoLineExport error: " << m_error ;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model*, void* _data)
    {
        return new VideoLineExport();
    }
}
