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

#ifndef KIO_FUSE_APP_H
#define KIO_FUSE_APP_H

#include <QString>
#include <kapplication.h>
#include <kurl.h>
#include <kdebug.h>

class KioFuseApp : public KApplication
{
    Q_OBJECT

    public:
        KioFuseApp(const KUrl &url);
        ~KioFuseApp();
        const KUrl &baseUrl() const {return m_baseUrl;}
        KUrl buildUrl(const QString& path);
        bool UDSEntryCached(const QString &path);
        bool UDSEntryCacheExpired(const QString &path);
    private:
        KUrl m_baseUrl;
};

extern KioFuseApp *kioFuseApp;

#endif /* FUSE_KIO_FUSE_APP_H */
