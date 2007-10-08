/****************************************************************************
 *    Copyright (c) 2007 Vlad Codrea                                        *
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
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.   *
 ****************************************************************************/

#include "cache.h"

#include <kdebug.h>

Cache::Cache(KFileItem* item)
    : QObject(), m_item(item), m_stub(false)
{
    kDebug()<<"Cache ctor(item)"<<endl;
}

Cache::Cache(const QString& rootOfRelPath)
    : QObject(), m_stub(true)
{
    kDebug()<<"Cache ctor (rootOfRelPath)"<<endl;
    //FIXME item needs to be deleted in the cache as it expires
    m_item = new KFileItem();
    m_item->setUrl(KUrl(rootOfRelPath));
}

Cache::~Cache()
{
    kDebug()<<"Cache dtor: m_item->url().path(): "<<m_item->url().path()<<endl;
    delete m_item;
    m_item = 0;
    for (int i = 0; i < children.size(); ++i){
        delete children.at(i);
    }
}

void Cache::insert(KFileItem* newItem)
{
    //sleep(20);
    bool inserted = false;
    QString completePath = newItem->url().path();
    kDebug()<<"completePath: "<<completePath<<endl;
    QString myPath = m_item->url().path();
    kDebug()<<"myPath: "<<myPath<<endl;
    int relPathLength = completePath.length() - myPath.length();

    if (relPathLength == 0){
        //Replace previous item (which may be a stub) with a real item
        kDebug()<<"Replacing: myPath: "<<myPath<<endl;
        delete m_item;
        m_item = newItem;
        m_stub = false;
        return;
    }

    QString relPath = completePath.right(relPathLength);
    kDebug()<<"relPath"<<relPath<<endl;
    int endOfRootOfRelPath = relPath.indexOf("/");

    if (endOfRootOfRelPath < 0){
        // item is a child and it has no children of its own

        // Delete any child that might have the same filename and replace it with the new child
        removeChild(newItem->url().fileName());
        Cache* newChild = new Cache(newItem);
        children.append(newChild);
        return;
    } 
    
    QString rootOfRelPath = relPath.left(endOfRootOfRelPath);

    kDebug()<<"rootOfRelPath"<<rootOfRelPath<<endl;

    int i = findIdxOfChildFromFileName(rootOfRelPath);
    if (i != -1){
        kDebug()<<"inserting"<<endl;
        children.at(i)->insert(newItem);
        inserted = true;
    }
    
    if (!inserted){
        Cache* stub = new Cache(myPath.append(rootOfRelPath));
        children.append(stub);
        children.last()->insert(newItem);
    }
}

// Returns true if it actually found a child to remove
bool Cache::removeChild(const QString& fileName)
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
}

int Cache::findIdxOfChildFromFileName(const QString& fileName)
{
    for (int i = 0; i < children.size(); ++i){
        if (children.at(i)->item()->url().fileName() == fileName){
            return i;
        }
    }
    return -1;
}

Cache Cache::find(const KUrl &url)
{

}

void Cache::removeExpired()
{

}
