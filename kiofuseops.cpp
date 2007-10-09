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

#include <errno.h>

#include "kiofuseops.h"
#include "kiofuseapp.h"
#include "jobhelpers.h"

int kioFuseGetAttr(const char *path, struct stat *stbuf)
{
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    /*if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
    } else
        res = -ENOENT;*/
    stbuf->st_mode = S_IFDIR | 0777;
    stbuf->st_nlink = 2;

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

int kioFuseReadDir(const char *relPath, void *buf, fuse_fill_dir_t filler,
                    off_t offset, struct fuse_file_info *fi)
{
    //KIO::UDSEntry entry;
    //KIO::UDSEntryList entries;
    //QString fileName;
    ListJobHelper *helper;
    QEventLoop *eventLoop = new QEventLoop();
    KUrl url = kioFuseApp->buildUrl(QString(relPath));

    kDebug()<<"kioFuseReadDir relPath: "<<relPath<<endl;

    /*kDebug()<<"Before while() "<<endl;
    while (!helper->done())
    {
       kDebug()<<"Inside while() "<<endl;
       kioFuseApp->processEvents();
    }
    kDebug()<<"After while() "<<endl;*/


    if (kioFuseApp->childrenNamesCached(url) && !kioFuseApp->UDSCacheExpired(url)){
        //TODO get from cache
    }
    else{
        helper = new ListJobHelper(url, eventLoop);
        eventLoop->exec(QEventLoop::ExcludeUserInputEvents);

        //eventLoop has finished, so entries are now available
        KIO::UDSEntryList entries = helper->entries();
        for(KIO::UDSEntryList::ConstIterator it = entries.begin();
             it!=entries.end(); ++it){
            KIO::UDSEntry entry = *it;

            //FIXME item needs to be deleted in the cache as it expires
            KFileItem* item = new KFileItem(entry, url, true, true);
            filler(buf, item->name().toLatin1(), NULL, 0);
    
            kDebug()<<"KFileItem URL: "<<item->url().path()<<endl;
            //kioFuseApp->updateCache(url.addPath(entryName), entry);
            kioFuseApp->addToCache(item);
        }
        
        delete helper;
        helper = NULL;
    }

    delete eventLoop;
    eventLoop = NULL;

    /*if (strcmp(relPath, "/") != 0)
        return -ENOENT;*/

    return 0;
}
