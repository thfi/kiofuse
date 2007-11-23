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
    KUrl url = kioFuseApp->buildRemoteUrl(QString(relPath)); // The remote URL of the directory that is being read

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
            KFileItem* item = new KFileItem(entry, url,
                                            true /*delayedMimeTypes*/,
                                            true /*urlIsDirectory*/);  //FIXME item needs to be deleted in the cache as it expires
            fillStatBufFromFileItem(stbuf, item);
        }
        delete helper;
        helper = NULL;
    }
    delete eventLoop;
    eventLoop = NULL;

    return res;
}

int kioFuseReadLink(const char *relPath, char *buf, size_t size)
{
    kDebug()<<"relPath"<<relPath<<endl;
    int res = 0;
    StatJobHelper* helper;  // Helps retrieve the directory descriptors or file descriptors
    QEventLoop* eventLoop = new QEventLoop();  // Returns control to this function after helper gets the data
    KUrl url = kioFuseApp->buildRemoteUrl(QString(relPath)); // The remote URL of the directory that is being read

    if (false){
        //TODO get from cache
    } else {
        helper = new StatJobHelper(url, eventLoop);  // Get the directory or file descriptor (entry)
        eventLoop->exec(QEventLoop::ExcludeUserInputEvents);  // eventLoop->quit() is called in BaseJobHelper::jobDone() of helper
        
        //eventLoop has finished, so entry is now available
        if (helper->error()){
            res = -ENOENT;
        } else {
            KIO::UDSEntry entry = helper->entry();
            KFileItem* item = new KFileItem(entry, url,
                                            true /*delayedMimeTypes*/,
                                            true /*urlIsDirectory*/);  //FIXME item needs to be deleted in the cache as it expires
            // Make sure the item is a link and that it is under the baseURL
            if(!item->isLink() ||
               !item->linkDest().startsWith(kioFuseApp->baseUrl().path())){
                res = -errno;
            } else {
                QString destRelPath = item->linkDest().section(kioFuseApp->baseUrl().path(), 1,-1);
                QString fullLocalPath = kioFuseApp->buildLocalUrl(destRelPath).path();
                fillLinkBufFromFileItem(buf, size, fullLocalPath);
            }
        }
        delete helper;
        helper = NULL;
    }
    delete eventLoop;
    eventLoop = NULL;
    
    return res;
}

int kioFuseOpen(const char *relPath, struct fuse_file_info *fi)
{
    return -ENOENT;
}

int kioFuseRead(const char *relPath, char *buf, size_t size, off_t offset,
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
    KUrl url = kioFuseApp->buildRemoteUrl(QString(relPath)); // The remote URL of the directory that is being read

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
                
                /* The parent (..) directory doesn't belong to us. We need to
                   get its local permissions. */
                if (entry.stringValue(KIO::UDSEntry::UDS_NAME) == ".."){
                    kDebug()<<"Parent dir"<<endl;
                    memset(&st, 0, sizeof(st));
                    lstat("..", &st);
                    filler(buf, "..", &st, 0);
                } else {
                    KFileItem* item = new KFileItem(entry, url,
                            true /*delayedMimeTypes*/,
                            true /*urlIsDirectory*/);  //FIXME item needs to be deleted in the cache as it expires
                    fillStatBufFromFileItem(&st, item);
                    filler(buf, item->name().toLatin1(), &st, 0);  // Tell the name of this item to FUSE
    
                    kDebug()<<"KFileItem URL: "<<item->url().path()<<endl;
                    kioFuseApp->addToCache(item);  // Add this item (and any stub directories that may be needed) to the cache
                }
            }
        }
        delete helper;
        helper = NULL;
    }
    delete eventLoop;
    eventLoop = NULL;

    return res;
}

static void fillStatBufFromFileItem(struct stat *stbuf, KFileItem *item)
{
    //kDebug()<<" entry.numberValue(KIO::UDSEntry::UDS_ACCESS)"<<entry.numberValue(KIO::UDSEntry::UDS_ACCESS)<<endl;
    
    /* We should be finding out the effective permissions by taking into
    consideration 1) the group affiliation of the username we are connecting
    as and 2) the remote permission of file/directory we are accessing.
    This requires making KFileItem::isWritable() et. al. reliable for
    network slaves. For example, KFileItem::isWritable() should check the
       /etc/group file on the remote server to ensure that the user we are
    connecting with is a member of a group that has write permission. This
    will allow KioFuse to display the effective permission on the client
    side (currently KioFuse just copies the remote stat, which is not
    applicable to the local system).
    */
    
    memset(stbuf, 0, sizeof(struct stat));
    stbuf->st_dev = 0;
    stbuf->st_ino = 0;
    stbuf->st_mode = item->permissions()|(item->isLink()?S_IFLNK:item->mode());
    stbuf->st_nlink = 1;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_rdev = 0;
    stbuf->st_size = item->size();
    stbuf->st_mtime = item->time(KIO::UDSEntry::UDS_MODIFICATION_TIME);
    stbuf->st_atime = item->time(KIO::UDSEntry::UDS_ACCESS_TIME);
    stbuf->st_ctime = item->time(KIO::UDSEntry::UDS_CREATION_TIME);
    stbuf->st_blksize = 0;
    stbuf->st_blocks = 0;
}

static void fillLinkBufFromFileItem(char *buf, size_t size, const QString& dest)
{
    const char *data = dest.toLatin1();

    unsigned int len = size-1;

    if (dest.length()<len)
    {
        len = dest.length();
    }

    for(unsigned int i=0; i<len; i++)
    {
        buf[i] = data[i];
    }

    buf[len] = '\0';
}

