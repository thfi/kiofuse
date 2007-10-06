/****************************************************************************
 *    Copyright (c) 2007 Vlad Codrea                                        *
 *    Copyright (c) 2003-2004 by Alexander Neundorf & Kévin 'ervin' Ottens  *
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

ListJobHelper::ListJobHelper(const KUrl &url, QEventLoop *eventLoop)
    : BaseJobHelper(eventLoop), m_url(url), m_listJob(0)
{
    kDebug()<<"ListJobHelper() ctor for "<<m_url.prettyUrl()<<endl;

    m_listJob = KIO::listDir(url, false, true);
    m_job = m_listJob;

    connect(m_listJob, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)),
            this, SLOT(receiveEntries(KIO::Job *, const KIO::UDSEntryList &)));

    connect(m_listJob, SIGNAL(result(KJob *)),
            this, SLOT( jobDone(KJob *)));
}

ListJobHelper::~ListJobHelper()
{
    kDebug()<<"ListJobHelper dtor"<<endl;
}

void ListJobHelper::receiveEntries(KIO::Job *, const KIO::UDSEntryList &items)
{
    sleep(20);
    /*for(KIO::UDSEntryList::ConstIterator it = items.begin();
        it!=items.end(); ++it)
    {
       KIO::UDSEntry entry = *it;
       fuseApp->listDirCache()->fileItems.append(
                                                 new KFileItem(entry, m_url, true, true)
                                                );
    }*/
}
