/****************************************************************************
 *    Copyright (c) 2007 Vlad Codrea                                        *
 *    Copyright (c) 2003-2004 by Alexander Neundorf & Kevin 'ervin' Ottens  *
 *                                                                          *
 *   This program is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation; either version 2 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program; if not, write to the Free Software            *
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston,                 *
 *   MA 02111-1307, USA.                                                    *
 ****************************************************************************/

#include "jobhelpers.h"
#include <kdebug.h>

ListJobHelper::ListJobHelper(const KUrl& url, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop),  // The generalized job helper
      m_url(url),  // The remote url that we must list
      m_listJob(NULL)
{
    kDebug()<<"ListJobHelper() ctor for "<<m_url.prettyUrl()<<endl;
    kDebug()<<"eventLoop->thread()"<<eventLoop->thread()<<endl;
    kDebug()<<"this->thread()"<<this->thread()<<endl;

    m_listJob = KIO::listDir(url, KIO::HideProgressInfo, true);
    kDebug()<<"m_listJob->thread()"<<m_listJob->thread()<<endl;
    m_job = m_listJob;  // m_job belongs to the parent class
    
    connect(kioFuseApp, SIGNAL(testSignal1()), this, SLOT(testSlot1()), Qt::QueuedConnection);
    connect(this, SIGNAL(testSignal2()), kioFuseApp, SLOT(testSlot2()), Qt::QueuedConnection);
    emit testSignal2();

    // Load the entries into m_entries when they become available
    connect(m_listJob, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList &)),
            this, SLOT(receiveEntries(KIO::Job*, const KIO::UDSEntryList &)));

    kDebug()<<"After connecting entries"<<endl;

    // Job will be deleted when finished, and execution returned to the FUSE op
    connect(m_listJob, SIGNAL(result(KJob*)),
            this, SLOT( jobDone(KJob*)));
    kDebug()<<"After connecting result"<<endl;
}

ListJobHelper::~ListJobHelper()
{
    kDebug()<<"ListJobHelper dtor"<<endl;
}

void ListJobHelper::receiveEntries(KIO::Job*, const KIO::UDSEntryList &entries)  // Store entries so that the FUSE op can get them
{
    m_entries = entries;
}

void ListJobHelper::testSlot1()
{
    kDebug()<<"this->thread()"<<this->thread()<<endl;
}

/*void ListJobHelper::testSignal2()
{
    kDebug()<<"this->thread()"<<this->thread()<<endl;
}*/
