/*
 * MixerWorkerThread.cpp - implementation of MixerWorkerThread
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

#include "MixerWorkerThread.h"

#include "Mixer.h"
#include "ThreadableJob.h"
#include "denormals.h"
#include "Backtrace.h"

#include <QMutex>
#include <QWaitCondition>

MixerWorkerThread::JobQueue MixerWorkerThread::s_globalJobQueue;
QWaitCondition*             MixerWorkerThread::s_queueReadyWaitCond = nullptr;
SafeList<MixerWorkerThread*> MixerWorkerThread::s_workerThreads;

// implementation of internal JobQueue
void MixerWorkerThread::JobQueue::reset(OperationMode _opMode)
{
    /*
    if(m_queueSize > 0)
    {
        BACKTRACE
        qWarning("MixerWorkerThread::JobQueue::reset queue=%d done=%d",
                 (int)m_queueSize, (int)m_itemsDone);
    }
    */
    m_queueSize = 0;
    m_itemsDone = 0;
    m_opMode    = _opMode;
}

void MixerWorkerThread::JobQueue::addJob(ThreadableJob* _job)
{
    if(_job->requiresProcessing())
    {
        // update job state
        _job->queue();
        // actually queue the job via atomic operations
        auto index = m_queueSize.fetchAndAddOrdered(1);
        if(index < JOB_QUEUE_SIZE)
        {
            m_items[index] = _job;
        }
        else
        {
            qWarning("MixerWorkerThread::JobQueue full");
            m_itemsDone.fetchAndAddOrdered(1);
        }
    }
}

void MixerWorkerThread::JobQueue::run()
{
    bool processedJob = true;
    while(processedJob && (int)m_itemsDone < (int)m_queueSize)
    {
        processedJob = false;
        for(int i = 0; i < m_queueSize && i < JOB_QUEUE_SIZE; ++i)
        {
            ThreadableJob* job = m_items[i].fetchAndStoreOrdered(nullptr);
            if(job != nullptr)
            {
                // qInfo("Processing job in %s",
                //      qPrintable(QThread::currentThread()->objectName()));
                job->process();
                processedJob = true;
                m_itemsDone.fetchAndAddOrdered(1);
            }
        }
        // always exit loop if we're not in dynamic mode
        processedJob = processedJob && (m_opMode == Dynamic);
    }
}

void MixerWorkerThread::JobQueue::wait()
{
    while((int)m_itemsDone < (int)m_queueSize)
    {
#if defined(LMMS_HOST_X86) || defined(LMMS_HOST_X86_64)
        asm("pause");
#endif
    }
}

// implementation of worker threads

MixerWorkerThread::MixerWorkerThread(Mixer* mixer) :
      QThread(mixer), m_quit(false)
{
    setObjectName("mixer worker");

    // initialize global static data
    if(s_queueReadyWaitCond == nullptr)
        s_queueReadyWaitCond = new QWaitCondition();

    // keep track of all instantiated worker threads - this is used for
    // processing the last worker thread "inline", see comments in
    // MixerWorkerThread::startAndWaitForJobs() for details
    // workerThreads << this;
    s_workerThreads.append(this);

    resetJobQueue();
}

MixerWorkerThread::~MixerWorkerThread()
{
    // workerThreads.removeAll(this);
    s_workerThreads.removeOne(this);
}

void MixerWorkerThread::quit()
{
    m_quit = true;
    resetJobQueue();
}

void MixerWorkerThread::startAndWaitForJobs()
{
    s_queueReadyWaitCond->wakeAll();
    // The last worker-thread is never started. Instead it's processed
    // "inline" i.e. within the global Mixer thread. This way we can reduce
    // latencies that otherwise would be caused by synchronizing with another
    // thread.
    // s_globalJobQueue.run();
    s_globalJobQueue.wait();
}

void MixerWorkerThread::run()
{
    disable_denormals();

    Mutex m("MixerWorkerThread::run", false);
    while(m_quit == false)
    {
        m.lock();
        s_queueReadyWaitCond->wait(&m);
        s_globalJobQueue.run();
        m.unlock();
    }
}
