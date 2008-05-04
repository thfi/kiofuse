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

BaseJobHelper::BaseJobHelper(QEventLoop* eventLoop, const KUrl& url)
    : QObject(),
      m_error(0),  // Error code returned by the job
      m_url(url),  // The remote url
      m_eventLoop(eventLoop)  // The event loop that will return execution to the FUSE op once the job finished
{
    kDebug()<<"BaseJobHelper ctor"<<endl;
}

void BaseJobHelper::jobDone(const int& error)
{
    kDebug()<<"jobDone"<<endl;
    m_error = error;

    m_eventLoop->exit();  // Return execution to the FUSE op that called us
}

BaseJobHelper::~BaseJobHelper()
{
    kDebug()<<"BaseJobHelper dtor"<<endl;

    if (!m_eventLoop->isRunning())
    {
        m_eventLoop->exit();
        m_eventLoop = NULL;
    }
}
