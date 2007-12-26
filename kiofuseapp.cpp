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

#include "kiofuseapp.h"

#include <QThread>

#include <kdebug.h>

KioFuseApp *kioFuseApp = NULL;

KioFuseApp::KioFuseApp(const KUrl &url, const KUrl &mountPoint)
    : KApplication(false),  //No GUI
      m_baseUrl(url),
      m_mountPoint(mountPoint),
      m_baseUrlMutex(QMutex::Recursive),  // Allow the mutex to be locked several times within the same thread
      m_mountPointMutex(QMutex::Recursive),  // Allow the mutex to be locked several times within the same thread
      m_cacheRoot(NULL),
      m_numCached(1),  // One stub (the root) is already cached in the constructor, so start counter at 1
      m_numLeafStubsCached(0),  // Leaf stubs are for opened files that have no stat data
      m_cacheMutex(QMutex::Recursive)  // Allow the mutex to be locked several times within the same thread
{
    QMutexLocker locker(&m_cacheMutex);
    kDebug()<<"KioFuseApp ctor baseUrl: "<<m_baseUrl.prettyUrl()<<endl;

    QString root = QString("/");  // Create the cache root, which represents the root directory (/)
    m_cacheRoot = new Cache(root, Cache::innerStubType);  // All files and folders will be children of this node
}

KioFuseApp::~KioFuseApp()
{
    QMutexLocker locker(&m_cacheMutex);

    kDebug()<<"KioFuseApp dtor"<<endl;
    delete m_cacheRoot;  // Recursively delete the whole cache
    m_cacheRoot = NULL;
}

const KUrl& KioFuseApp::baseUrl()  // Getter method for the remote base URL
{
    QMutexLocker locker(&m_baseUrlMutex);
    return m_baseUrl;
}

const KUrl& KioFuseApp::mountPoint()  // Getter method for the remote base URL
{
    QMutexLocker locker(&m_mountPointMutex);
    return m_mountPoint;
}

KUrl KioFuseApp::buildRemoteUrl(const QString& path)  // Create a full URL containing both the remote base and the relative path
{
    QMutexLocker locker(&m_baseUrlMutex);
    KUrl url = baseUrl();
    /*if (path == "/"){
        // Don't need to append only a "/"
        // Allows files to be baseUrls
        return url;
    }*/
    url.addPath(path);
    return url;
}

KUrl KioFuseApp::buildLocalUrl(const QString& path)  // Create a full URL containing both the remote base and the relative path
{
    QMutexLocker locker(&m_mountPointMutex);
    KUrl url = mountPoint();
    url.addPath(path);
    return url;
}

bool KioFuseApp::UDSCached(const KUrl& url)
{
    QMutexLocker locker(&m_cacheMutex);
    return false;
}

bool KioFuseApp::childrenNamesCached(const KUrl& url)
{
    QMutexLocker locker(&m_cacheMutex);
    //TODO Names might be cached, but other info may not be
    return UDSCached(url);
}

bool KioFuseApp::UDSCacheExpired(const KUrl& url)
{
    QMutexLocker locker(&m_cacheMutex);
    return true;
}

void KioFuseApp::addToCache(KFileItem* item)  // Add this item (and any stub directories that may be needed) to the cache
{
    QMutexLocker locker(&m_cacheMutex);
    m_cacheRoot->insert(item);
    m_numCached++;
}

void KioFuseApp::storeOpenHandle(const KUrl& url, KIO::FileJob* fileJob,
                                 const uint64_t& fileHandleId)  // Add this item (and any stub directories that may be needed) to the cache
{
    QMutexLocker locker(&m_cacheMutex);
    bool addedLeafStub = m_cacheRoot->setExtraData(url, fileHandleId, fileJob);
    if (addedLeafStub){
        m_numLeafStubsCached++;
    }
}

// Find the job using its ID, and prevent other threads from
// using it
KIO::FileJob* KioFuseApp::checkOutJob(const KUrl& url, const uint64_t& fileHandleId)
{
    QMutexLocker locker(&m_cacheMutex);
    Cache* currCache = m_cacheRoot->find(url);
    // FIXME when currCache == NULL
    
    if (currCache->jobsMap().contains(fileHandleId)){
        FileJobData* fileJobData = currCache->jobsMap().value(fileHandleId);
        while (fileJobData->inUse){
            locker.unlock();
            // Another FuseOp thread is using this FileJob. Sleep for a while
            // and try again. This should be needed very rarely, if at all.
            kDebug()<<"Waiting 1s for filejob"<<url.path()<<endl;
            sleep(1);
            locker.relock();
        }
        fileJobData->inUse = true;
        return fileJobData->fileJob;
    } else {
        // Didn't find the job
        return NULL;
    }
}

// Allow other threads to use the FileJob specified by this fileHandleId
void KioFuseApp::checkInJob(const KUrl& url, const uint64_t& fileHandleId)
{
    QMutexLocker locker(&m_cacheMutex);
    Cache* currCache = m_cacheRoot->find(url);
    // FIXME when currCache == NULL
    
    // The fileJob *must* be there if we're checking it back in
    Q_ASSERT(currCache->jobsMap().contains(fileHandleId));
     
    FileJobData* fileJobData = currCache->jobsMap().value(fileHandleId);
    fileJobData->inUse = false;
}

/*********** ListJob ***********/
void KioFuseApp::listJobMainThread(const KUrl& url, ListJobHelper* listJobHelper)
{
    kDebug()<<"this->thread()"<<this->thread()<<endl;
    kDebug()<<"QThread::currentThread()"<<QThread::currentThread()<<endl;
    
    KIO::ListJob* listJob = KIO::listDir(url, KIO::HideProgressInfo, true);
    Q_ASSERT(listJob->thread() == this->thread());
    
    kDebug()<<"listJob->thread()"<<listJob->thread()<<endl;
    
    // Job will be deleted when finished
    connect(listJob, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));
    
    // Needed to be able to use Qt::QueuedConnection
    qRegisterMetaType<KIO::UDSEntryList>("KIO::UDSEntryList");
    
    // Send the entries to listJobHelper when they become available
    connect(listJob, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList &)),
            listJobHelper, SLOT(receiveEntries(KIO::Job*, const KIO::UDSEntryList &)),
            Qt::QueuedConnection);
    
    // Correlate listJob with the ListJobHelper that needs it
    m_jobToJobHelper.insert(qobject_cast<KJob*>(listJob),
                            qobject_cast<BaseJobHelper*>(listJobHelper));
    

    kDebug()<<"at the end"<<endl;
}

void KioFuseApp::slotResult(KJob* job)
{
    kDebug()<<"this->thread()"<<this->thread()<<endl;
    
    BaseJobHelper* jobHelper = m_jobToJobHelper.value(job);
    connect(this, SIGNAL(sendJobDone(const int&)),
            jobHelper, SLOT(jobDone(const int&)), Qt::QueuedConnection);
    emit sendJobDone(job->error());
    
    // Remove job and jobHelper from map
    int numJobsRemoved = m_jobToJobHelper.remove(job);
    Q_ASSERT(numJobsRemoved == 1);
    
    Q_ASSERT(job);
    job->kill();
    job = NULL;
}

/*********** StatJob ***********/
void KioFuseApp::statJobMainThread(const KUrl& url, StatJobHelper* statJobHelper)
{
    /*KIO::StatJob* statJob = KIO::stat(url, KIO::StatJob::SourceSide,
                                      2, KIO::HideProgressInfo);*/
    KIO::StatJob* statJob = KIO::stat(url, KIO::HideProgressInfo);
    Q_ASSERT(statJob->thread() == this->thread());
    
    // Job will be deleted when finished
    connect(statJob, SIGNAL(result(KJob*)),
            this, SLOT(slotStatJobResult(KJob*)));
    
    // Correlate listJob with the ListJobHelper that needs it
    m_jobToJobHelper.insert(qobject_cast<KJob*>(statJob),
                            qobject_cast<BaseJobHelper*>(statJobHelper));
}

void KioFuseApp::slotStatJobResult(KJob* job)
{
    BaseJobHelper* jobHelper = m_jobToJobHelper.value(job);
    StatJobHelper* statJobHelper = qobject_cast<StatJobHelper*>(jobHelper);
    
    if (job->error() == 0){
        KIO::StatJob* statJob = qobject_cast<KIO::StatJob*>(job);
        KIO::UDSEntry entry = statJob->statResult();
        
        // Needed to be able to use Qt::QueuedConnection
        qRegisterMetaType<KIO::UDSEntry>("KIO::UDSEntry");
    
        // Send the entry to statJobHelper
        connect(this, SIGNAL(sendEntry(const KIO::UDSEntry &)),
                statJobHelper, SLOT(receiveEntry(const KIO::UDSEntry &)),
                Qt::QueuedConnection);
        emit sendEntry(entry);
    }
    
    connect(this, SIGNAL(sendJobDone(const int&)),
            jobHelper, SLOT(jobDone(const int&)), Qt::QueuedConnection);
    emit sendJobDone(job->error());
    
    // Remove job and jobHelper from map
    int numJobsRemoved = m_jobToJobHelper.remove(job);
    Q_ASSERT(numJobsRemoved == 1);
    
    Q_ASSERT(job);
    job->kill();
    job = NULL;
}

/*********** OpenJob ***********/
void KioFuseApp::openJobMainThread(const KUrl& url, const QIODevice::OpenMode& qtMode, OpenJobHelper* openJobHelper)
{
    KIO::FileJob* fileJob = KIO::open(url, qtMode);  // Will be cached. Kill()-ed upon close()
    m_jobToJobHelper.insert(qobject_cast<KJob*>(fileJob),
                            qobject_cast<BaseJobHelper*>(openJobHelper));
    
    Q_ASSERT(fileJob->thread() == this->thread());
    connect(fileJob, SIGNAL(open(KIO::Job*)),
            this, SLOT(fileJobOpened(KIO::Job*)));
}

void KioFuseApp::fileJobOpened(KIO::Job* job)
{
    KIO::FileJob* fileJob = qobject_cast<KIO::FileJob*>(job);
    BaseJobHelper* jobHelper = m_jobToJobHelper.value(qobject_cast<KJob*>(job));
    OpenJobHelper* openJobHelper = qobject_cast<OpenJobHelper*>(jobHelper);
    
    // Send the FileJob
    connect(this, SIGNAL(sendFileJob(KIO::FileJob*)),
            openJobHelper, SLOT(receiveFileJob(KIO::FileJob*)),
            Qt::QueuedConnection);
    emit sendFileJob(fileJob);
    
    connect(this, SIGNAL(sendJobDone(const int&)),
            openJobHelper, SLOT(jobDone(const int&)), Qt::QueuedConnection);
    emit sendJobDone(fileJob->error());
    
    // Remove job and jobHelper from map
    int numJobsRemoved = m_jobToJobHelper.remove(qobject_cast<KJob*>(job));
    Q_ASSERT(numJobsRemoved == 1);
}

/*********** Seek ***********/
void KioFuseApp::seekMainThread(KIO::FileJob* fileJob, const off_t& offset, ReadJobHelper* readJobHelper)
{
    Q_ASSERT(fileJob->thread() == this->thread());
    kDebug()<<"fileJob"<<fileJob<<"fileJob->thread()"<<fileJob->thread()<<endl;
    connect(fileJob, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
            this, SLOT(slotPosition(KIO::Job*, KIO::filesize_t)));
    m_jobToJobHelper.insert(qobject_cast<KJob*>(fileJob),
                            qobject_cast<BaseJobHelper*>(readJobHelper));
    fileJob->seek(static_cast<KIO::filesize_t>(offset));
}

void KioFuseApp::slotPosition(KIO::Job* job, KIO::filesize_t pos)
{
    /*if (!m_jobToJobHelper.contains(qobject_cast<KJob*>(job))){
        // This is the second time FileJob::position() was called.
        // We must already have removed the job. Ignore the call.
        kDebug()<<"ignoring FileJob::position"<<endl;
        return;
    }*/
    
    KIO::FileJob* fileJob = qobject_cast<KIO::FileJob*>(job);
    
    // Prevent FileJob::position() signal from being triggered multiple times
    // upon subsequent seeks
    fileJob->disconnect();
    
    kDebug()<<"job"<<job<<"job->thread()"<<job->thread()<<endl;
    
    Q_ASSERT(m_jobToJobHelper.contains(qobject_cast<KJob*>(job)));
    BaseJobHelper* jobHelper = m_jobToJobHelper.value(qobject_cast<KJob*>(job));
    ReadJobHelper* readJobHelper = qobject_cast<ReadJobHelper*>(jobHelper);
    // Needed by Qt::QueuedConnection
    qRegisterMetaType<off_t>("off_t");
    // Send the position to readJobHelper
    connect(this, SIGNAL(sendPosition(const off_t&, const int&)),
            readJobHelper, SLOT(receivePosition(const off_t&, const int&)),
            Qt::QueuedConnection);
    emit sendPosition(static_cast<off_t>(pos), job->error());

    // Remove job and jobHelper from map
    int numJobsRemoved = m_jobToJobHelper.remove(qobject_cast<KJob*>(job));
    Q_ASSERT(numJobsRemoved == 1);
}

void KioFuseApp::readMainThread(KIO::FileJob* fileJob, const size_t& size, ReadJobHelper* readJobHelper)
{
    kDebug()<<"size"<<size<<endl;
    kDebug()<<"readJobHelper"<<readJobHelper<<endl;
    
    m_jobToJobHelper.insert(qobject_cast<KJob*>(fileJob),
                            qobject_cast<BaseJobHelper*>(readJobHelper));
    
    Q_ASSERT(fileJob->thread() == this->thread());
    connect(fileJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
    
    fileJob->read(static_cast<KIO::filesize_t>(size));
}

void KioFuseApp::slotData(KIO::Job* job, const QByteArray& data)
{
    kDebug()<<"data"<<data<<endl;
    
    KIO::FileJob* fileJob = qobject_cast<KIO::FileJob*>(job);
    
    // Prevent FileJob::data() signal from being triggered multiple times
    // upon subsequent reads
    fileJob->disconnect();
    
    BaseJobHelper* jobHelper = m_jobToJobHelper.value(qobject_cast<KJob*>(job));
    ReadJobHelper* readJobHelper = qobject_cast<ReadJobHelper*>(jobHelper);
    
    Q_ASSERT(readJobHelper);
    
    /*if (readJobHelper == NULL){
        if(data.size() != 0){
            //FIXME
            kDebug()<<"IGNORING REAL DATA. BAD!!!!!!!"<<endl;
        } else {
            kDebug()<<"IGNORING EMPTY"<<endl;
        }
        // Safe to ignore because FileJob sends extra empty data() signals
        return;
    }*/
    
    connect(this, SIGNAL(sendData(const QByteArray&, const int&)),
            readJobHelper, SLOT(receiveData(const QByteArray&, const int&)),
            Qt::QueuedConnection);
    emit sendData(data, job->error());
    
    // Remove job and jobHelper from map
    int numJobsRemoved = m_jobToJobHelper.remove(qobject_cast<KJob*>(job));
    Q_ASSERT(numJobsRemoved == 1);
}
