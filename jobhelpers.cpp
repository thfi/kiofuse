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

/*********** ListJobHelper ***********/
ListJobHelper::ListJobHelper(const KUrl& url, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    connect(this, SIGNAL(reqListJob(KUrl, ListJobHelper*)), kioFuseApp,
            SLOT(listJobMainThread(KUrl, ListJobHelper*)), Qt::QueuedConnection);
    emit reqListJob(url, this);
}

ListJobHelper::~ListJobHelper()
{
    kDebug()<<"ListJobHelper dtor"<<endl;
}

KIO::UDSEntryList ListJobHelper::entries()
{
    return m_entries;
}

void ListJobHelper::receiveEntries(KIO::Job*, const KIO::UDSEntryList &entries)  // Store entries so that the FUSE op can get them
{
    m_entries = entries;
}

/*********** StatJobHelper ***********/
StatJobHelper::StatJobHelper(const KUrl& url, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    connect(this, SIGNAL(reqStatJob(KUrl, StatJobHelper*)), kioFuseApp,
            SLOT(statJobMainThread(KUrl, StatJobHelper*)), Qt::QueuedConnection);
    emit reqStatJob(url, this);
}

StatJobHelper::~StatJobHelper()
{
    kDebug()<<"StatJobHelper dtor"<<endl;
}

KIO::UDSEntry StatJobHelper::entry()
{
    return m_entry;
}

void StatJobHelper::receiveEntry(const KIO::UDSEntry &entry)  // Store entry so that the FUSE op can get it
{
    m_entry = entry;
}
