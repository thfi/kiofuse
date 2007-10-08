#ifndef CACHE_H
#define CACHE_H

#include <QString>
#include <QList>

#include <kurl.h>
#include <kfileitem.h>

class Cache : public QObject
{
    Q_OBJECT

    public:
        Cache(KFileItem* item);
        Cache(const QString& rootOfRelPath);
        ~Cache();
        void insert(KFileItem* newItem);
        Cache find(const KUrl &url);
        void removeExpired();
        KFileItem* item() const {return m_item;}

    private:
        bool removeChild(const QString& fileName);
        int findIdxOfChildFromFileName(const QString& fileName);

        KFileItem* m_item;
        QList<Cache*> children;
        bool m_stub;
};

#endif /* CACHE_H */
