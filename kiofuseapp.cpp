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

#include "kiofuseapp.h"

KioFuseApp *kioFuseApp = NULL;

KioFuseApp::KioFuseApp(const KUrl &url)
    : QObject(),
      m_baseUrl(url),
      m_baseUrlMutex(QMutex::Recursive),  // Allow the mutex to be locked several times within the same thread
      m_cacheRoot(NULL),
      m_numCached(1),  // One stub (the root) is already cached in the constructor, so start counter at 1
      m_cacheMutex(QMutex::Recursive)  // Allow the mutex to be locked several times within the same thread
{
    QMutexLocker locker(&m_cacheMutex);
    kDebug()<<"KioFuseApp ctor baseUrl: "<<m_baseUrl.prettyUrl()<<endl;

    QString root = QString("/");  // Create the cache root, which represents the root directory (/)
    m_cacheRoot = new Cache(root);  // All files and folders will be children of this node
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

KUrl KioFuseApp::buildUrl(const QString& path)  // Create a full URL containing both the remote base and the relative path
{
    QMutexLocker locker(&m_baseUrlMutex);
    KUrl url = baseUrl();
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
