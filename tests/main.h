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

#include <QObject>
#include <KIO/Job>
#include <QIODevice>

class TestFileJob : public QObject
{
    Q_OBJECT

    public:
        TestFileJob() {}
        void run();
    
    public slots:
        void slotData(KIO::Job*, const QByteArray& data);
        void slotOpen(KIO::Job* job);
        void slotResult(KJob* job);
        void slotPosition(KIO::Job* job, KIO::filesize_t offset);
        void slotRedirection(KIO::Job*, const KUrl& url);
        void slotClose(KIO::Job*);
        void slotMimetype(KIO::Job* job, const QString &mimetype);
};
