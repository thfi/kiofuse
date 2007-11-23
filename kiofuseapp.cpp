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
    Q_ASSERT(fileJob->thread() == this->thread());
    
    // Send the FileJob
    connect(this, SIGNAL(sendFileJob(KIO::FileJob*)),
            openJobHelper, SLOT(receiveFileJob(KIO::FileJob*)),
                                Qt::QueuedConnection);
    emit sendFileJob(fileJob);
    
    connect(this, SIGNAL(sendJobDone(const int&)),
            openJobHelper, SLOT(jobDone(const int&)), Qt::QueuedConnection);
    emit sendJobDone(fileJob->error());
}
