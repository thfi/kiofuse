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
    : KApplication(), m_baseUrl(url), m_cacheRoot(NULL), m_numCached(1)
{
    kDebug()<<"KioFuseApp ctor baseUrl: "<<m_baseUrl.prettyUrl()<<endl;
    QString root = QString("/");
    m_cacheRoot = new Cache(root);
}

KioFuseApp::~KioFuseApp()
{
    kDebug()<<"KioFuseApp dtor"<<endl;
    delete m_cacheRoot;
    m_cacheRoot = 0;
}

KUrl KioFuseApp::buildUrl(const QString& path)
{
   KUrl url = baseUrl();
   url.addPath(path);
   return url;
}

bool KioFuseApp::UDSCached(const KUrl& url)
{
    return false;
}

bool KioFuseApp::childrenNamesCached(const KUrl& url)
{
    //TODO Names might be cached, but other info may not be
    return UDSCached(url);
}

bool KioFuseApp::UDSCacheExpired(const KUrl& url)
{
    return true;
}

void KioFuseApp::addToCache(KFileItem* item)
{
    /*kDebug()<<"addToCache"<<endl;
    if (m_cacheRoot == NULL){
        kDebug()<<"Creating cacheRoot"<<endl;
        m_cacheRoot = new Cache(item);
    }else{
        kDebug()<<"Using extant cacheRoot"<<endl;
        m_cacheRoot->insert(item);
    }*/
    m_cacheRoot->insert(item);

    m_numCached++;
}
