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

#ifndef KIO_FUSE_APP_H
#define KIO_FUSE_APP_H

#include "cache.h"
#include "jobhelpers.h"

#include <KApplication>
#include <ktemporaryfile.h>

class ListJobHelper;
class StatJobHelper;
class OpenJobHelper;
class ReadJobHelper;
class WriteJobHelper;
class MkNodHelper;
class ChModHelper;
class ReleaseJobHelper;
class MkDirHelper;

class KioFuseApp : public KApplication
{
    Q_OBJECT

    public:
        KioFuseApp(const KUrl &url, const KUrl &mountPoint);
        ~KioFuseApp();
        const KUrl& baseUrl();  // Getter method for the remote base URL
        const KUrl& mountPoint();  // Getter method for the mountpoint URL
        KUrl buildRemoteUrl(const QString& path);  // Create a full URL containing both the remote base and the relative path
        KUrl buildLocalUrl(const QString& path);  // Create a full URL containing both the local mountpoint and the relative path
        bool UDSCached(const KUrl& url);
        bool childrenNamesCached(const KUrl& url);
        bool UDSCacheExpired(const KUrl& url);
        void addToCache(KFileItem* item);  // Add this item (and any stub directories that may be needed) to the cache
        void storeOpenHandle(const KUrl& url, KIO::FileJob* fileJob,
                             const uint64_t& fileHandleId);
        KIO::FileJob* checkOutJob(const KUrl& url, const uint64_t& fileHandleId);  // Find the job using its ID, 
                                                                                   // and prevent other threads from using it.
        void checkInJob(const KUrl& url, const uint64_t& fileHandleId);  // Allow other threads to use the
                                                                          // FileJob specified by this fileHandleId.
    
    public slots:
        void listJobMainThread(const KUrl& url, ListJobHelper* listJobHelper);
        void slotResult(KJob* job);
        void statJobMainThread(const KUrl& url, StatJobHelper* statJobHelper);
        void slotStatJobResult(KJob* job);
        void openJobMainThread(const KUrl& url, const QIODevice::OpenMode& qtMode, OpenJobHelper* openJobHelper);
        void seekReadMainThread(KIO::FileJob* fileJob, const off_t& offset, ReadJobHelper* readJobHelper);
        void slotReadPosition(KIO::Job* job, KIO::filesize_t pos);
        void readMainThread(KIO::FileJob* fileJob, const size_t& size, ReadJobHelper* readJobHelper);
        void slotData(KIO::Job* job, const QByteArray& data);
        void fileJobOpened(KIO::Job* job);
        void seekWriteMainThread(KIO::FileJob* fileJob, const off_t& offset, WriteJobHelper* writeJobHelper);
        void slotWritePosition(KIO::Job* job, KIO::filesize_t pos);
        void writeMainThread(KIO::FileJob* fileJob, const QByteArray& data, WriteJobHelper* writeJobHelper);
        void slotWritten(KIO::Job* job, const KIO::filesize_t& written);
        void mkDirMainThread(const KUrl& url, const mode_t& mode, MkDirHelper* mkDirHelper);
        void slotMkDirResult(KJob* job);
        void mkNodMainThread(const KUrl& url, const mode_t& mode, MkNodHelper* mkNodHelper);
        void slotMkNodResult(KJob* job);
        void chModMainThread(const KUrl& url, const mode_t& mode,
                             ChModHelper* chModHelper);
        void slotChModResult(KJob* job);
        void releaseJobMainThread(const KUrl& url, const uint64_t& fileHandleId, ReleaseJobHelper* releaseJobHelper);

        
    signals:
        void sendJobDone(const int&);
        void sendEntry(const KIO::UDSEntry &);
        void sendFileJob(KIO::FileJob*);
        void sendPosition(const off_t&, const int&);
        void sendData(const QByteArray&, const int&);
        void sendWritten(const size_t&, const int&);
        
    private:
        KUrl m_baseUrl;  // Remote base URL
        KUrl m_mountPoint; // Local mountpoint
        QMutex m_baseUrlMutex;  // Allows only one thread to access the remote base URL at a time
        QMutex m_mountPointMutex;  // Allows only one thread to access the local mountpoint URL at a time

        Cache* m_cacheRoot;  // Root node of cache
        int m_numCached;  // Number of files cached
        int m_numLeafStubsCached;  // Leaf stubs are for opened files that have no stat data
        QMutex m_cacheMutex;  // Allows only one thread to access the cache at a time
        
        QMap<KJob *, BaseJobHelper *> m_jobToJobHelper;  // Correlate listJob with the ListJobHelper that needs it
        QMap<KJob *, KTemporaryFile *> m_jobToTempFile;  // Correlate FileCopyJob with the MkNodHelper that needs it
};

extern KioFuseApp *kioFuseApp;  // Make the kioFuseApp variable known to everyone who includes this file

#endif /* FUSE_KIO_FUSE_APP_H */
