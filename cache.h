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
