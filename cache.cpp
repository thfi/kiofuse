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
