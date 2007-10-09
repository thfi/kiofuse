/****************************************************************************
 *    Copyright (c) 2007 Vlad Codrea                                        *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Library General Public            *
 *   License as published by the Free Software Foundation; either           *
 *   version 2 of the License, or (at your option) any later version.       *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Library General Public License for more details.                       *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.   *
 ****************************************************************************/

#include "cache.h"

#include <kdebug.h>

Cache::Cache(KFileItem* item)
    : QObject(), m_item(item), m_stub(false)
{
    kDebug()<<"Cache ctor(item)"<<endl;
}

Cache::Cache(QString rootOfRelPath)
    : QObject(), m_item(NULL), m_stub(true)
{
    kDebug()<<"Cache ctor (rootOfRelPath)"<<endl;
    //FIXME item needs to be deleted in the cache as it expires
    KIO::UDSEntry blankEntry = KIO::UDSEntry();
    m_item = new KFileItem(blankEntry, KUrl(rootOfRelPath), true, true);
}

Cache::~Cache()
{
    kDebug()<<"Cache dtor: m_item->url().path(KUrl::RemoveTrailingSlash): "<<m_item->url().path(KUrl::RemoveTrailingSlash)<<endl;
    delete m_item;
    m_item = 0;
    for (int i = 0; i < children.size(); ++i){
        delete children.at(i);
    }
}

void Cache::insert(KFileItem* newItem)
{
    //sleep(3);
    QString completePath = newItem->url().path(KUrl::RemoveTrailingSlash);
    kDebug()<<"completePath: "<<completePath<<endl;
    QString myPath = m_item->url().path(KUrl::RemoveTrailingSlash);
    kDebug()<<"myPath: "<<myPath<<endl;
    int relPathLength = completePath.length() - myPath.length();

    if (relPathLength == 0){
        //Replace previous item (which may be a stub) with a real item
        kDebug()<<"Replacing: myPath: "<<myPath<<endl;
        setItem(newItem);
        return;
    }

    QString relPath = completePath.right(relPathLength);
    relPath = stripBegSlashes(relPath);
    kDebug()<<"relPath"<<relPath<<endl;
    int endOfRootOfRelPath = relPath.indexOf("/");

    if (endOfRootOfRelPath < 0){
        // item is a child and it has no children of its own
        int i = findIdxOfChildFromFileName(newItem->url().fileName(KUrl::IgnoreTrailingSlash));
        if (i != -1){
            // A child by this name already exists; replace its old item with the new item
            kDebug()<<"Replacing item of child"<<newItem->url().fileName(KUrl::IgnoreTrailingSlash)<<endl;
            children.at(i)->insert(newItem);         //Same as children.at(i)->setItem(newItem);
        } else {
            // Create a new child that contains newItem
            kDebug()<<"Creating new child"<<newItem->url().fileName(KUrl::IgnoreTrailingSlash)<<endl;
            Cache* newChild = new Cache(newItem);
            children.append(newChild);
        }
        /*// Delete any child that might have the same filename and replace it with the new child
        removeChild(newItem->url().fileName(KUrl::IgnoreTrailingSlash));
        Cache* newChild = new Cache(newItem);
        children.append(newChild);*/
        return;
    } 
    
    QString rootOfRelPath = relPath.left(endOfRootOfRelPath);

    kDebug()<<"rootOfRelPath"<<rootOfRelPath<<endl;

    int i = findIdxOfChildFromFileName(rootOfRelPath);
    if (i != -1){
        kDebug()<<"Inserting"<<endl;
        children.at(i)->insert(newItem);
    } else {
        kDebug()<<"Creating stub"<<endl;

        Cache* stub;
        if (myPath == "/"){       // Treat root dir differently since it is not affected by RemoveTrailingSlash
            stub = new Cache(myPath.append(rootOfRelPath));
        } else {
            stub = new Cache(myPath.append("/").append(rootOfRelPath));
        }

        children.append(stub);

        // last() can be used because the stub that was just appended is found in the last position
        children.last()->insert(newItem);
    }
}

// Returns true if it actually found a child to remove
/*bool Cache::removeChild(const QString& fileName)
{
    kDebug()<<"removeChild: fileName: "<<fileName<<endl;
    int i = findIdxOfChildFromFileName(fileName);
    if (i != -1){
        kDebug()<<"Found child to remove"<<endl;
        delete children.at(i);
        children.removeAt(i);
        return true;
    }
    return false;
}*/

int Cache::setItem(KFileItem* newItem)
{
    if (m_item != NULL){
        delete m_item;
    }
    m_item = newItem;
    m_stub = false;
}

Cache Cache::find(const KUrl &url)
{

}

void Cache::removeExpired()
{

}

int Cache::findIdxOfChildFromFileName(const QString& fileName)
{
    for (int i = 0; i < children.size(); ++i){
        if (children.at(i)->item()->url().fileName(KUrl::IgnoreTrailingSlash) == fileName){
            return i;
        }
    }
    return -1;
}

QString Cache::stripBegSlashes(const QString& path)
{
    bool foundNonSlash = false;
    QString newPath = QString();

    for (int i = 0; i < path.length(); ++i){
        if (path[i] != '/'){
            foundNonSlash = true;
        }

        if (foundNonSlash){
            newPath.append(path[i]);
        }
    }

    return newPath;
}
