/*
 * MixerWorkerThread.h - declaration of class MixerWorkerThread
 *
 * Copyright (c) 2009-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef MIXER_WORKER_THREAD_H
#define MIXER_WORKER_THREAD_H

#include "AudioPort.h"
#include "SafeList.h"

#include <QAtomicPointer>
#include <QThread>

#include <AtomicInt.h>

class QWaitCondition;
class Mixer;
class ThreadableJob;

class MixerWorkerThread : public QThread
{
  public:
    // internal representation of the job queue - all functions are
    // thread-safe
    class JobQueue
    {
      public:
        enum OperationMode
        {
            Static,  // no jobs added while processing queue
            Dynamic  // jobs can be added while processing queue
        };

        JobQueue() :
              m_items(), m_queueSize(0), m_itemsDone(0), m_opMode(Static)
        {
        }

        void reset(OperationMode _opMode);

        void addJob(ThreadableJob* _job);

        void run();
        void wait();

      private:
#define JOB_QUEUE_SIZE 256
        // 2048
        QAtomicPointer<ThreadableJob> m_items[JOB_QUEUE_SIZE];
        AtomicInt                     m_queueSize;
        AtomicInt                     m_itemsDone;
        OperationMode                 m_opMode;
    };

    MixerWorkerThread(Mixer* mixer);
    virtual ~MixerWorkerThread();

    virtual void quit();

    static void resetJobQueue(JobQueue::OperationMode _opMode
                              = JobQueue::Static)
    {
        s_globalJobQueue.reset(_opMode);
    }

    static void addJob(ThreadableJob* _job)
    {
        s_globalJobQueue.addJob(_job);
    }

    // a convenient helper function allowing to pass a container with pointers
    // to ThreadableJob objects
    template <typename T>
    static void fillJobQueue(SafeList<T>&            _vec,
                             JobQueue::OperationMode _opMode
                             = JobQueue::Static)
    {
        resetJobQueue(_opMode);
        _vec.map([](T _e) {
            /*
              if(_e.isNull())
                qWarning("MixerWorkingThread::fillJobQueue null job");
              else
            */
            {
                /*
                AudioPort* ap=dynamic_cast<AudioPort*>(_e.data());
                if(ap!=nullptr)
                  qWarning("fillJobQueue ap=%s", qPrintable(ap->name()));
                */
                MixerWorkerThread::addJob(_e.data());
            }
        });
    }

    static void startAndWaitForJobs();

  private:
    virtual void run();

    static JobQueue                     s_globalJobQueue;
    static QWaitCondition*              s_queueReadyWaitCond;
    static SafeList<MixerWorkerThread*> s_workerThreads;

    volatile bool m_quit;
};

#endif
