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

#include "main.h"

#include <QDir>

#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
#include <kio/filejob.h>
#include <kdebug.h>

using namespace std;

void TestFileJob::slotData(KIO::Job* job, const QByteArray& data)
{
    KIO::FileJob *fileJob = qobject_cast<KIO::FileJob *>(job);
    if (data.isEmpty()){
	kDebug()<<"data="<<data<<"<---EMPTY!!!!!"<<endl;
    } else {
	kDebug()<<"data="<<data<<endl;
    }
    fileJob->seek(5);
}

void TestFileJob::slotResult(KJob* job)
{
    kDebug()<<"job->error()"<<job->error()<<endl;
}

void TestFileJob::slotOpen(KIO::Job* job)
{
    KIO::FileJob *fileJob = qobject_cast<KIO::FileJob *>(job);
    kDebug()<<"Size = "<<fileJob->size()<<endl;
    fileJob->read(1024);
}

void TestFileJob::slotClose(KIO::Job*)
{
    kDebug()<<"Closed"<<endl;
}

void TestFileJob::slotPosition(KIO::Job* /*job*/, KIO::filesize_t offset)
{
    kDebug()<<"Position is now "<<offset<<endl;
    //KIO::FileJob *fileJob = qobject_cast<KIO::FileJob *>(job);
}

void TestFileJob::slotRedirection(KIO::Job*, const KUrl& url)
{
    kDebug()<<"New Url="<<url.path()<<endl;
}

void TestFileJob::slotMimetype(KIO::Job* /*job*/, const QString &mimetype)
{
    //KIO::FileJob *fileJob = qobject_cast<KIO::FileJob *>(job);
    kDebug()<<"Mimetype is "<<mimetype<<endl;
}

void TestFileJob::run()
{
    KUrl url = KUrl(QDir::currentPath() + "/data.txt");

    KIO::FileJob *job = KIO::open(url, QIODevice::ReadOnly);
    connect(job, SIGNAL(open(KIO::Job*)),
                 SLOT(slotOpen(KIO::Job*)));
    connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
    connect(job, SIGNAL(result(KJob*)),
            SLOT(slotResult(KJob*)));
    connect(job, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
            SLOT(slotPosition(KIO::Job*, KIO::filesize_t)));
    connect(job, SIGNAL(redirection(KIO::Job*, const KUrl&)),
            SLOT(slotRedirection(KIO::Job*, const KUrl&)));
    connect(job, SIGNAL(close(KIO::Job*)),
                 SLOT(slotClose(KIO::Job*)));
    connect(job, SIGNAL(mimetype(KIO::Job*, const QString&)),
                 SLOT(slotMimetype(KIO::Job*, const QString&)));
}

static const KAboutData aboutData("TestFileJob",
                        0,
                        ki18n("TestFileJob"),
                        "0.1",
                        ki18n("Use asynchronous KIO facilities"),
                        KAboutData::License_LGPL,
                        ki18n("(c) 2007"),
                        KLocalizedString(),
                        "",
                        "");

int main (int argc, char *argv[])
{
    KCmdLineArgs::init( argc, argv, &aboutData );    
    KApplication app;

    TestFileJob aTestFileJob;
    aTestFileJob.run();

    return app.exec();
}
