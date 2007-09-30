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

#include "basejobhelper.h"

#include <kdebug.h>

BaseJobHelper::BaseJobHelper()
    : QObject(), m_done(false),
      m_error(0), m_job(0)
{
    kDebug()<<"BaseJobHelper ctor"<<endl;
}

void BaseJobHelper::jobDone(KJob *job)
{
    kDebug()<<"jobDone"<<endl;
    m_error = job->error();
    m_done = true;
    m_eventLoop->quit();
    m_eventLoop = 0;
}

void BaseJobHelper::setEventLoop(QEventLoop &eventLoop)
{
    m_eventLoop = &eventLoop;
}

BaseJobHelper::~BaseJobHelper()
{
    kDebug()<<"BaseJobHelper dtor"<<endl;

    if (m_job!=0)
    {
        m_job->kill();
        m_job = 0;
    }

    if (m_eventLoop!=0)
    {
        m_eventLoop->quit();
        m_eventLoop = 0;
    }
}
