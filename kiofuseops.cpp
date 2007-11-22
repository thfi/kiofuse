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

#include "kiofuseops.h"
#include "jobhelpers.h"

#include <errno.h>

#include <QThread>

#include <kdebug.h>

int kioFuseGetAttr(const char *relPath, struct stat *stbuf)
{
    kDebug()<<"relPath"<<relPath<<endl;
    
    int res = 0;
    
    StatJobHelper* helper;  // Helps retrieve the directory descriptors or file descriptors
    QEventLoop* eventLoop = new QEventLoop();  // Returns control to this function after helper gets the data
    KUrl url = kioFuseApp->buildUrl(QString(relPath)); // The remote URL of the directory that is being read

    if (false /*kioFuseApp->UDSCacheExpired(url)*/){
        //TODO get from cache
    } else {
        helper = new StatJobHelper(url, eventLoop);  // Get the directory or file descriptor (entry)
        eventLoop->exec(QEventLoop::ExcludeUserInputEvents);  // eventLoop->quit() is called in BaseJobHelper::jobDone() of helper
        
        //eventLoop has finished, so entry is now available
        if (helper->error()){
            res = -ENOENT;
        } else {
            KIO::UDSEntry entry = helper->entry();
            memset(stbuf, 0, sizeof(struct stat));
            stbuf->st_mode = S_IFDIR | entry.numberValue(KIO::UDSEntry::UDS_ACCESS);
            stbuf->st_nlink = 2;
        }
        delete helper;
        helper = NULL;
    }
    delete eventLoop;
    eventLoop = NULL;

    return res;
}

int kioFuseOpen(const char *path, struct fuse_file_info *fi)
{
    return -ENOENT;
}

int kioFuseRead(const char *path, char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi)
{
    size = 0;

    return 0;
}

// Get the names of files and directories under a specified directory
int kioFuseReadDir(const char *relPath, void *buf, fuse_fill_dir_t filler,
                    off_t offset, struct fuse_file_info *fi)
{
    int res = 0;
    ListJobHelper* helper;  // Helps retrieve the directory descriptors or file descriptors
    QEventLoop* eventLoop = new QEventLoop();  // Returns control to this function after helper gets the data
    KUrl url = kioFuseApp->buildUrl(QString(relPath)); // The remote URL of the directory that is being read

    kDebug()<<"kioFuseReadDir relPath: "<<relPath<<"eventLoop->thread()"<<eventLoop->thread()<<endl;

    if (kioFuseApp->childrenNamesCached(url) && !kioFuseApp->UDSCacheExpired(url)){
        //TODO get from cache
    } else {
        helper = new ListJobHelper(url, eventLoop);  // Get the directory or file descriptors (entries)
        eventLoop->exec(QEventLoop::ExcludeUserInputEvents);  // eventLoop->quit() is called in BaseJobHelper::jobDone() of helper

        //eventLoop has finished, so entries are now available
        if (helper->error()){
            res = -ENOENT;
        } else {
            KIO::UDSEntryList entries = helper->entries();
            for(KIO::UDSEntryList::ConstIterator it = entries.begin();
                 it!=entries.end(); ++it){
                KIO::UDSEntry entry = *it;
                struct stat st;
                /*if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == ".."){
                    kDebug()<<"continuing"<<endl;
                    continue;
                }*/
                kDebug()<<" entry.numberValue(KIO::UDSEntry::UDS_ACCESS)"<<entry.numberValue(KIO::UDSEntry::UDS_ACCESS)<<endl;
                st.st_mode = S_IFDIR | entry.numberValue(KIO::UDSEntry::UDS_ACCESS);

                KFileItem* item = new KFileItem(entry, url, true, true);  //FIXME item needs to be deleted in the cache as it expires
                filler(buf, item->name().toLatin1(), &st, 0);  // Tell the name of this item to FUSE
    
                kDebug()<<"KFileItem URL: "<<item->url().path()<<endl;
                kioFuseApp->addToCache(item);  // Add this item (and any stub directories that may be needed) to the cache
            }
        }
        delete helper;
        helper = NULL;
    }
    delete eventLoop;
    eventLoop = NULL;

    return res;
}
