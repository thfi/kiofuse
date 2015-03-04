/****************************************************************************
 *   Copyright (c) 2003-2004 by Alexander Neundorf & Kevin 'ervin' Ottens   *
 *   Copyright (c) 2007-2008 Vlad Codrea                                    *
 *   Copyright (c) 2015 Thomas Fischer                                      *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License.     *
 *                                                                          *
 ****************************************************************************/

#include "jobhelpers.h"
#include <kdebug.h>

/*********** ListJobHelper ***********/
ListJobHelper::ListJobHelper(const KUrl& url, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "listJobMainThread",
                                       Q_ARG(KUrl, url),
                                       Q_ARG(ListJobHelper*, this)));
    /*connect(this, SIGNAL(reqListJob(const KUrl&, ListJobHelper*)), kioFuseApp,
            SLOT(listJobMainThread(const KUrl&, ListJobHelper*)), Qt::QueuedConnection);
    emit reqListJob(url, this);*/
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
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "statJobMainThread",
                                       Q_ARG(KUrl, url),
                                       Q_ARG(StatJobHelper*, this)));
    /*connect(this, SIGNAL(reqStatJob(const KUrl&, StatJobHelper*)), kioFuseApp,
            SLOT(statJobMainThread(const KUrl&, StatJobHelper*)), Qt::QueuedConnection);
    emit reqStatJob(url, this);*/
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
      m_fileHandleId(0)
{
    // Needed by Qt::QueuedConnection
    //qRegisterMetaType<QIODevice::OpenMode>("QIODevice::OpenMode");
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "openJobMainThread",
                                       Q_ARG(KUrl, url),
                                       Q_ARG(QIODevice::OpenMode, qtMode),
                                       Q_ARG(OpenJobHelper*, this)));
    /*connect(this, SIGNAL(reqFileJob(const KUrl&, const QIODevice::OpenMode&, OpenJobHelper*)),
            kioFuseApp, SLOT(openJobMainThread(const KUrl&, const QIODevice::OpenMode&, OpenJobHelper*)),
            Qt::QueuedConnection);
    emit reqFileJob(url, qtMode, this);*/
}

void OpenJobHelper::setFileHandleId(const uint64_t& aFileHandleId)
{
    m_fileHandleId = aFileHandleId;
}

OpenJobHelper::~OpenJobHelper()
{
    kDebug()<<"OpenJobHelper dtor"<<endl;
}

/*********** LockHelper ***********/
LockHelper::LockHelper(const uint64_t& fileHandleId, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, KUrl()),  // The generalized job helper
                    m_fileHandleId(fileHandleId)
{
    // Needed by Qt::QueuedConnection
    //qRegisterMetaType<uint64_t>("uint64_t");
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "findMutexMainThread",
                                       Q_ARG(uint64_t, m_fileHandleId),
                                       Q_ARG(LockHelper*, this)));
    /*connect(this, SIGNAL(reqFindMutex(const uint64_t&, LockHelper*)),
            kioFuseApp, SLOT(findMutexMainThread(const uint64_t&, LockHelper*)),
                             Qt::QueuedConnection);
    emit reqFindMutex(m_fileHandleId, this);*/
}

void LockHelper::setJobMutex(QMutex* mutex, const int& error)
{
    m_jobMutex = mutex;
    VERIFY(QMetaObject::invokeMethod(this, "jobDone",
                                       Q_ARG(int, error)));
    /*connect(this, SIGNAL(sendJobDone(const int&)),
            this, SLOT(jobDone(const int&)));
    emit sendJobDone(error);*/
}

LockHelper::~LockHelper()
{
    kDebug()<<"LockHelper dtor"<<endl;
}

/*********** ReadJobHelper ***********/
ReadJobHelper::ReadJobHelper(const uint64_t& fileHandleId, const KUrl& url, const size_t& size,
                             const off_t& offset, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url),  // The generalized job helper
      m_fileHandleId(fileHandleId),
      m_fileJob(NULL),
      m_data(),
      m_size(size),
      m_offset(offset)
{
    // Needed by Qt::QueuedConnection
    //qRegisterMetaType<off_t>("off_t");
    //qRegisterMetaType<uint64_t>("uint64_t");
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "seekReadMainThread",
                                       Q_ARG(uint64_t, m_fileHandleId),
                                       Q_ARG(off_t, offset),
                                       Q_ARG(ReadJobHelper*, this)));
    /*connect(this, SIGNAL(reqSeek(const uint64_t&,
            const off_t&, ReadJobHelper*)),
            kioFuseApp, SLOT(seekReadMainThread(const uint64_t&,
            const off_t&, ReadJobHelper*)),
            Qt::QueuedConnection);
    emit reqSeek(m_fileHandleId, offset, this);*/
}

ReadJobHelper::~ReadJobHelper()
{
    kDebug()<<"ReadJobHelper dtor"<<endl;
}

void ReadJobHelper::receivePosition(const off_t& pos, const int& error, KIO::FileJob* fileJob)
{
    //disconnect(kioFuseApp, SIGNAL(sendPosition(const off_t&, const int&, KIO::FileJob*)),
    //           this, 0);
    //FIXME Don't need because QMetaObject::invokeMethod
    //disconnect(kioFuseApp, 0, this, 0);
    m_fileJob = fileJob;

    kDebug()<<"m_offset"<<m_offset<<"pos"<<pos;
    kDebug()<<"m_size"<<m_size<<"error"<<error;
    kDebug()<<"m_fileJob"<<m_fileJob<<endl;
    kDebug()<<"this"<<this<<"this->thread()"<<this->thread()<<endl;

    if (error){
        kWarning()<<"WARNING: Job reported error while seeking.";
        m_size = 0;
        VERIFY(QMetaObject::invokeMethod(this, "jobDone",
                                         Q_ARG(int, error)));
        return;
    }

    /*// Needed by Qt::QueuedConnection
    qRegisterMetaType<size_t>("size_t");
    connect(this, SIGNAL(reqRead(KIO::FileJob*,
            const size_t&, ReadJobHelper*)),
            kioFuseApp, SLOT(readMainThread(KIO::FileJob*,
                             const size_t&, ReadJobHelper*)),
            Qt::QueuedConnection);
    emit reqRead(m_fileJob, m_size, this);*/
    if (pos == m_offset){
        // Needed by Qt::QueuedConnection
        //qRegisterMetaType<size_t>("size_t");
        VERIFY(QMetaObject::invokeMethod(kioFuseApp, "readMainThread",
                                           Q_ARG(KIO::FileJob*, m_fileJob),
                                           Q_ARG(size_t, m_size),
                                           Q_ARG(ReadJobHelper*, this)));
        /*connect(this, SIGNAL(reqRead(KIO::FileJob*,
                const size_t&, ReadJobHelper*)),
                kioFuseApp, SLOT(readMainThread(KIO::FileJob*,
                const size_t&, ReadJobHelper*)),
                Qt::QueuedConnection);
        emit reqRead(m_fileJob, m_size, this);*/
    } else {
        kWarning()<<"WARNING: m_offset != pos.";
        m_size = 0;
        /*VERIFY(QMetaObject::invokeMethod(kioFuseApp, "slotResult",
                                           Q_ARG(KJob*, qobject_cast<KJob*>
                                                                 (m_fileJob))));*/
        VERIFY(QMetaObject::invokeMethod(this, "jobDone",
                                           Q_ARG(int,
                                                 KIO::ERR_COULD_NOT_SEEK)));
        /*connect(this, SIGNAL(sendJobDone(const int&)),
                this, SLOT(jobDone(const int&)));
        emit sendJobDone(error);*/
    }
}

void ReadJobHelper::receiveData(const QByteArray& data, const int& error)
{
    //disconnect(kioFuseApp, SIGNAL(sendData(const QByteArray&, const int&)),
    //           this, 0);
    //FIXME Not needed because of QMetaObject::invokeMethod
    //disconnect(kioFuseApp, 0, this, 0);

    kDebug()<<"data"<<data<<endl;
    m_data = data;
    VERIFY(QMetaObject::invokeMethod(this, "jobDone",
                                       Q_ARG(int, error)));
    /*connect(this, SIGNAL(sendJobDone(const int&)),
            this, SLOT(jobDone(const int&)));
    emit sendJobDone(error);*/
}

/*********** WriteJobHelper ***********/
WriteJobHelper::WriteJobHelper(const uint64_t& fileHandleId, const KUrl& url, const QByteArray& data,
                               const off_t& offset, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url),  // The generalized job helper
      m_fileHandleId(fileHandleId),
      m_fileJob(NULL),
      m_data(data),
      m_written(),
      m_offset(offset)
{
    // Needed by Qt::QueuedConnection
    //qRegisterMetaType<off_t>("off_t");
    //qRegisterMetaType<uint64_t>("uint64_t");
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "seekWriteMainThread",
                                       Q_ARG(uint64_t, m_fileHandleId),
                                       Q_ARG(off_t, offset),
                                       Q_ARG(WriteJobHelper*, this)));
    /*connect(this, SIGNAL(reqSeek(const uint64_t&,
            const off_t&, WriteJobHelper*)),
            kioFuseApp, SLOT(seekWriteMainThread(const uint64_t&,
            const off_t&, WriteJobHelper*)),
            Qt::QueuedConnection);
    emit reqSeek(m_fileHandleId, offset, this);*/
}

WriteJobHelper::~WriteJobHelper()
{
    kDebug()<<"WriteJobHelper dtor"<<endl;
}

void WriteJobHelper::receivePosition(const off_t& pos, const int& error, KIO::FileJob* fileJob)
{
    m_fileJob = fileJob;

    kDebug()<<"m_offset"<<m_offset<<"pos"<<pos;
    kDebug()<<"m_fileJob"<<m_fileJob<<endl;
    kDebug()<<"this->thread()"<<this->thread()<<endl;

    if (error){
        kWarning()<<"WARNING: Job reported error while seeking.";
        m_written = 0;
        VERIFY(QMetaObject::invokeMethod(this, "jobDone",
               Q_ARG(int, error)));
        return;
    }

    /*connect(this, SIGNAL(reqWrite(KIO::FileJob*,
            const QByteArray&, WriteJobHelper*)),
            kioFuseApp, SLOT(writeMainThread(KIO::FileJob*,
                             const QByteArray&, WriteJobHelper*)),
            Qt::QueuedConnection);
    emit reqWrite(m_fileJob, m_data, this);*/
    if (pos == m_offset){
        VERIFY(QMetaObject::invokeMethod(kioFuseApp, "writeMainThread",
                                           Q_ARG(KIO::FileJob*, m_fileJob),
                                           Q_ARG(QByteArray, m_data),
                                           Q_ARG(WriteJobHelper*, this)));
        /*connect(this, SIGNAL(reqWrite(KIO::FileJob*,
                const QByteArray&, WriteJobHelper*)),
                kioFuseApp, SLOT(writeMainThread(KIO::FileJob*,
                const QByteArray&, WriteJobHelper*)),
                Qt::QueuedConnection);
        emit reqWrite(m_fileJob, m_data, this);*/
    } else {
        kWarning()<<"WARNING: m_offset != pos.";
        m_written = 0;
        /*VERIFY(QMetaObject::invokeMethod(kioFuseApp, "slotResult",
                                           Q_ARG(KJob*, qobject_cast<KJob*>
                                                                 (m_fileJob))));*/
        VERIFY(QMetaObject::invokeMethod(this, "jobDone",
                                           Q_ARG(int,
                                                 KIO::ERR_COULD_NOT_SEEK)));
        /*connect(this, SIGNAL(sendJobDone(const int&)),
                this, SLOT(jobDone(const int&)));
        emit sendJobDone(error);*/
    }
}

void WriteJobHelper::receiveWritten(const size_t& written, const int& error)
{
    kDebug()<<"written"<<written<<endl;
    m_written = written;
    VERIFY(QMetaObject::invokeMethod(this, "jobDone",
                                       Q_ARG(int, error)));
    /*connect(this, SIGNAL(sendJobDone(const int&)),
            this, SLOT(jobDone(const int&)));
    emit sendJobDone(error);*/
}

/*********** MkDir ***********/
MkDirHelper::MkDirHelper(const KUrl& url, const mode_t& mode, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    // Needed by Qt::QueuedConnection
    //qRegisterMetaType<mode_t>("mode_t");
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "mkDirMainThread",
                                       Q_ARG(KUrl, url),
                                       Q_ARG(mode_t, mode),
                                       Q_ARG(MkDirHelper*, this)));
    /*connect(this, SIGNAL(reqMkDir(const KUrl&, const mode_t&, MkDirHelper*)), kioFuseApp,
            SLOT(mkDirMainThread(const KUrl&, const mode_t&, MkDirHelper*)), Qt::QueuedConnection);
    emit reqMkDir(url, mode, this);*/
}

MkDirHelper::~MkDirHelper()
{
    kDebug()<<"MkDirHelper dtor"<<endl;
}

/*********** UnLink ***********/
UnLinkHelper::UnLinkHelper(const KUrl& url, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "unLinkMainThread",
                                       Q_ARG(KUrl, url),
                                       Q_ARG(UnLinkHelper*, this)));
    /*connect(this, SIGNAL(reqUnLink(const KUrl&, UnLinkHelper*)), kioFuseApp,
            SLOT(unLinkMainThread(const KUrl&, UnLinkHelper*)), Qt::QueuedConnection);
    emit reqUnLink(url, this);*/
}

UnLinkHelper::~UnLinkHelper()
{
    kDebug()<<"UnLinkHelper dtor"<<endl;
}

/*********** MkNod ***********/
MkNodHelper::MkNodHelper(const KUrl& url, const mode_t& mode, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    // Needed by Qt::QueuedConnection
    //qRegisterMetaType<mode_t>("mode_t");
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "mkNodMainThread",
                                       Q_ARG(KUrl, url),
                                       Q_ARG(mode_t, mode),
                                       Q_ARG(MkNodHelper*, this)));
    /*connect(this, SIGNAL(reqMkNod(const KUrl&, const mode_t&, MkNodHelper*)), kioFuseApp,
            SLOT(mkNodMainThread(const KUrl&, const mode_t&, MkNodHelper*)), Qt::QueuedConnection);
    emit reqMkNod(url, mode, this);*/
}

MkNodHelper::~MkNodHelper()
{
    kDebug()<<"MkNodHelper dtor"<<endl;
}

/*********** SymLink ***********/
SymLinkHelper::SymLinkHelper(const KUrl& source, const KUrl& dest, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, source)  // The generalized job helper
{
    kDebug()<<"source"<<source<<"dest"<<dest<<endl;
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "symLinkMainThread",
                                       Q_ARG(KUrl, source),
                                       Q_ARG(KUrl, dest),
                                       Q_ARG(SymLinkHelper*, this)));
    /*connect(this, SIGNAL(reqSymLink(const KUrl&, const KUrl&, SymLinkHelper*)), kioFuseApp,
            SLOT(symLinkMainThread(const KUrl&, const KUrl&, SymLinkHelper*)), Qt::QueuedConnection);
    emit reqSymLink(source, dest, this);*/
}

SymLinkHelper::~SymLinkHelper()
{
    kDebug()<<"SymLinkHelper dtor"<<endl;
}

/*********** ReName ***********/
ReNameHelper::ReNameHelper(const KUrl& source, const KUrl& dest, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, source)  // The generalized job helper
{
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "reNameMainThread",
                                       Q_ARG(KUrl, source),
                                       Q_ARG(KUrl, dest),
                                       Q_ARG(ReNameHelper*, this)));
    /*connect(this, SIGNAL(reqReName(const KUrl&, const KUrl&, ReNameHelper*)), kioFuseApp,
            SLOT(reNameMainThread(const KUrl&, const KUrl&, ReNameHelper*)), Qt::QueuedConnection);
    emit reqReName(source, dest, this);*/
}

ReNameHelper::~ReNameHelper()
{
    kDebug()<<"ReNameHelper dtor"<<endl;
}

/*********** ChMod ***********/
ChModHelper::ChModHelper(const KUrl& url, const mode_t& mode, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    // Needed by Qt::QueuedConnection
    //qRegisterMetaType<mode_t>("mode_t");
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "chModMainThread",
                                       Q_ARG(KUrl, url),
                                       Q_ARG(mode_t, mode),
                                       Q_ARG(ChModHelper*, this)));
    /*connect(this, SIGNAL(reqChMod(const KUrl&, const mode_t&, ChModHelper*)), kioFuseApp,
            SLOT(chModMainThread(const KUrl&, const mode_t&, ChModHelper*)), Qt::QueuedConnection);
    emit reqChMod(url, mode, this);*/
}

ChModHelper::~ChModHelper()
{
    kDebug()<<"ChModHelper dtor"<<endl;
}

/*********** ReleaseJob ***********/
ReleaseJobHelper::ReleaseJobHelper(const KUrl& url,
                                   const uint64_t& fileHandleId,
                                   const bool& jobIsAnnulled,
                                   QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    // Needed by Qt::QueuedConnection
    //qRegisterMetaType<uint64_t>("uint64_t");
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "releaseJobMainThread",
                                       /*Q_ARG(KUrl, url),*/
                                       Q_ARG(uint64_t, fileHandleId),
                                       Q_ARG(bool, jobIsAnnulled),
                                       Q_ARG(ReleaseJobHelper*, this)));
    /*connect(this, SIGNAL(reqReleaseJob(const KUrl&, const uint64_t&, ReleaseJobHelper*)), kioFuseApp,
            SLOT(releaseJobMainThread(const KUrl&, const uint64_t&, ReleaseJobHelper*)), Qt::QueuedConnection);
    emit reqReleaseJob(url, fileHandleId, this);*/
}

ReleaseJobHelper::~ReleaseJobHelper()
{
    kDebug()<<"ReleaseJobHelper dtor"<<endl;
}

/*********** ChTime ***********/
ChTimeHelper::ChTimeHelper(const KUrl& url, const QDateTime& dt, QEventLoop* eventLoop)
    : BaseJobHelper(eventLoop, url)  // The generalized job helper
{
    VERIFY(QMetaObject::invokeMethod(kioFuseApp, "chTimeMainThread",
                                       Q_ARG(KUrl, url),
                                       Q_ARG(QDateTime, dt),
                                       Q_ARG(ChTimeHelper*, this)));
    /*connect(this, SIGNAL(reqChTime(const KUrl&, const QDateTime&, ChTimeHelper*)), kioFuseApp,
            SLOT(chTimeMainThread(const KUrl&, const QDateTime&, ChTimeHelper*)), Qt::QueuedConnection);
    emit reqChTime(url, dt, this);*/
}

ChTimeHelper::~ChTimeHelper()
{
    kDebug()<<"ChTimeHelper dtor"<<endl;
}
