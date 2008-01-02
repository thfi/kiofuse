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
    connect(this, SIGNAL(reqListJob(const KUrl&, ListJobHelper*)), kioFuseApp,
            SLOT(listJobMainThread(const KUrl&, ListJobHelper*)), Qt::QueuedConnection);
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
    connect(this, SIGNAL(reqStatJob(const KUrl&, StatJobHelper*)), kioFuseApp,
            SLOT(statJobMainThread(const KUrl&, StatJobHelper*)), Qt::QueuedConnection);
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

/*********** OpenJobHelper ***********/
OpenJobHelper::OpenJobHelper(const KUrl& url, const QIODevice::OpenMode& qtMode,
                             QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url),  // The generalized job helper
      m_qtMode(qtMode),  // How to open the file (read, write, both, append, etc)
      m_fileJob(NULL)
{
    // Needed by Qt::QueuedConnection
    qRegisterMetaType<QIODevice::OpenMode>("QIODevice::OpenMode");
    connect(this, SIGNAL(reqFileJob(const KUrl&, const QIODevice::OpenMode&, OpenJobHelper*)),
            kioFuseApp, SLOT(openJobMainThread(const KUrl&, const QIODevice::OpenMode&, OpenJobHelper*)),
            Qt::QueuedConnection);
    emit reqFileJob(url, qtMode, this);
}

OpenJobHelper::~OpenJobHelper()
{
    kDebug()<<"OpenJobHelper dtor"<<endl;
}

void OpenJobHelper::receiveFileJob(KIO::FileJob* fileJob)  // Store entry so that the FUSE op can get it
{
    m_fileJob = fileJob;
}

/*********** ReadJobHelper ***********/
ReadJobHelper::ReadJobHelper(KIO::FileJob* fileJob, const KUrl& url, const size_t& size,
                             const off_t& offset, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url),  // The generalized job helper
      m_fileJob(fileJob),
      m_data(),
      m_size(size),
      m_offset(offset)
{
    // Needed by Qt::QueuedConnection
    qRegisterMetaType<off_t>("off_t");
    connect(this, SIGNAL(reqSeek(KIO::FileJob*,
            const off_t&, ReadJobHelper*)),
            kioFuseApp, SLOT(seekReadMainThread(KIO::FileJob*,
            const off_t&, ReadJobHelper*)),
            Qt::QueuedConnection);
    emit reqSeek(m_fileJob, offset, this);
}

ReadJobHelper::~ReadJobHelper()
{
    kDebug()<<"ReadJobHelper dtor"<<endl;
}

void ReadJobHelper::receivePosition(const off_t& pos, const int& error)
{
    kDebug()<<"m_offset"<<m_offset<<"pos"<<pos;
    kDebug()<<"m_size"<<m_size<<"error"<<error;
    kDebug()<<"m_fileJob"<<m_fileJob<<endl;
    kDebug()<<"this->thread()"<<this->thread()<<endl;
    if (pos == m_offset){
        // Needed by Qt::QueuedConnection
        qRegisterMetaType<size_t>("size_t");
        connect(this, SIGNAL(reqRead(KIO::FileJob*,
                const size_t&, ReadJobHelper*)),
                kioFuseApp, SLOT(readMainThread(KIO::FileJob*,
                const size_t&, ReadJobHelper*)),
                Qt::QueuedConnection);
        emit reqRead(m_fileJob, m_size, this);
    } else {  // FIXME
        m_size = 0;
        connect(this, SIGNAL(sendJobDone(const int&)),
                this, SLOT(jobDone(const int&)));
        emit sendJobDone(error);
    }
}

void ReadJobHelper::receiveData(const QByteArray& data, const int& error)
{
    kDebug()<<"data"<<data<<endl;
    m_data = data;
    connect(this, SIGNAL(sendJobDone(const int&)),
            this, SLOT(jobDone(const int&)));
    emit sendJobDone(error);
}

/*********** WriteJobHelper ***********/
WriteJobHelper::WriteJobHelper(KIO::FileJob* fileJob, const KUrl& url, const QByteArray& data,
                             const off_t& offset, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url),  // The generalized job helper
      m_fileJob(fileJob),
      m_data(data),
      m_written(),
      m_offset(offset)
{
    // Needed by Qt::QueuedConnection
    qRegisterMetaType<off_t>("off_t");
    connect(this, SIGNAL(reqSeek(KIO::FileJob*,
            const off_t&, WriteJobHelper*)),
            kioFuseApp, SLOT(seekWriteMainThread(KIO::FileJob*,
            const off_t&, WriteJobHelper*)),
            Qt::QueuedConnection);
    emit reqSeek(m_fileJob, offset, this);
}

WriteJobHelper::~WriteJobHelper()
{
    kDebug()<<"WriteJobHelper dtor"<<endl;
}

void WriteJobHelper::receivePosition(const off_t& pos, const int& error)
{
    kDebug()<<"m_offset"<<m_offset<<"pos"<<pos;
    kDebug()<<"m_fileJob"<<m_fileJob<<endl;
    kDebug()<<"this->thread()"<<this->thread()<<endl;
    if (pos == m_offset){
        // Needed by Qt::QueuedConnection
        qRegisterMetaType<size_t>("size_t");
        connect(this, SIGNAL(reqWrite(KIO::FileJob*,
                const QByteArray&, WriteJobHelper*)),
                kioFuseApp, SLOT(writeMainThread(KIO::FileJob*,
                const QByteArray&, WriteJobHelper*)),
                Qt::QueuedConnection);
        emit reqWrite(m_fileJob, m_data, this);
    } else {  // FIXME
        m_written = 0;
        connect(this, SIGNAL(sendJobDone(const int&)),
                this, SLOT(jobDone(const int&)));
        emit sendJobDone(error);
    }
}

void WriteJobHelper::receiveWritten(const size_t& written, const int& error)
{
    kDebug()<<"written"<<written<<endl;
    m_written = written;
    connect(this, SIGNAL(sendJobDone(const int&)),
            this, SLOT(jobDone(const int&)));
    emit sendJobDone(error);
}

/*********** MkNod ***********/
MkNodHelper::MkNodHelper(const KUrl& url, const mode_t& mode, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    // Needed by Qt::QueuedConnection
    qRegisterMetaType<mode_t>("mode_t");
    connect(this, SIGNAL(reqMkNod(const KUrl&, const mode_t&, MkNodHelper*)), kioFuseApp,
            SLOT(MkNodMainThread(const KUrl&, const mode_t&, MkNodHelper*)), Qt::QueuedConnection);
    emit reqMkNod(url, mode, this);
}

MkNodHelper::~MkNodHelper()
{
    kDebug()<<"MkNodHelper dtor"<<endl;
}
