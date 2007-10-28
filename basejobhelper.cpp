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

#include "basejobhelper.h"

#include <kdebug.h>

BaseJobHelper::BaseJobHelper(QEventLoop* eventLoop)
    : QObject(),
      m_done(false),  // The job has not yet been started, so it is not done
      m_eventLoop(eventLoop)  // The event loop that will return execution to the FUSE op once the job finished
{
    kDebug()<<"BaseJobHelper ctor"<<endl;
}

void BaseJobHelper::jobDone()
{
    kDebug()<<"jobDone"<<endl;
    m_done = true;
    m_eventLoop->quit();  // Return execution to the FUSE op that called us
    m_eventLoop = NULL;
}

KIO::UDSEntryList BaseJobHelper::entries()
{
    return m_entries;
}

BaseJobHelper::~BaseJobHelper()
{
    kDebug()<<"BaseJobHelper dtor"<<endl;

    if (m_eventLoop != NULL)  // Should already be NULL from  jobDone(), but it might not be if this instance of
    {                         // BaseJobHelper is deleted before the job is done
        m_eventLoop->quit();
        m_eventLoop = NULL;
    }
}
