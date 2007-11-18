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

#ifndef JOB_HELPERS_H
#define JOB_HELPERS_H

#include "basejobhelper.h"
#include "kiofuseapp.h"

#include <kurl.h>
#include <kio/udsentry.h>

class ListJobHelper : public BaseJobHelper  // Helps list a specified directory
{
    Q_OBJECT

public:
    ListJobHelper(const KUrl& url, QEventLoop* eventLoop);
    ~ListJobHelper();
    
signals:
    void reqListJob(KUrl, ListJobHelper*);

public slots:
    void receiveEntries(KIO::Job* job, const KIO::UDSEntryList& items);  // Store entries so that the FUSE op can get them

protected:
    KUrl m_url;  // The remote url that we must list
};

#endif /* JOB_HELPERS_H */
