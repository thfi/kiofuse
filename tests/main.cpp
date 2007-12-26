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
#include <kdebug.h>

MyThread::MyThread (TestFileJob *testFileJob) 
    : QThread(),
      m_testFileJob(testFileJob)
{
}

MyThread::~MyThread()
{
}

void MyThread::run()
{
    while (!m_testFileJob->opened1()){
        kDebug()<<"Waiting for FileJob 1 to be opened"<<endl;
        sleep(1);
    }
    
    /*while (!m_testFileJob->opened2()){
        kDebug()<<"Waiting for FileJob 2 to be opened"<<endl;
        sleep(1);
    }*/
    
    connect(this, SIGNAL(sendReadRequest()),
            m_testFileJob, SLOT(read()),
            Qt::QueuedConnection);
    
    connect(this, SIGNAL(sendSeekRequest()),
            m_testFileJob, SLOT(seek()),
            Qt::QueuedConnection);
    
    sleep(5);
    for (int i=0; i<4; i++){
        //sleep(5);
        kDebug()<<endl<<endl<<"----------------------------------------"<<endl;
        kDebug()<<"Reading"<<endl;
        emit(sendReadRequest());
    
        sleep(2);
        kDebug()<<"Seeking"<<endl;
        emit(sendSeekRequest());
    }
}

TestFileJob::TestFileJob()
    : m_job1(NULL),
      m_job2(NULL),
      m_opened1(false),
      m_opened2(false)
{
}

TestFileJob::~TestFileJob()
{
}

void TestFileJob::slotData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty()){
	kDebug()<<"data="<<data<<"<---EMPTY!!!!!"<<endl;
    } else {
	kDebug()<<"data="<<data<<endl;
    }
}

void TestFileJob::slotResult(KJob* job)
{
    kDebug()<<"job->error()"<<job->error()<<endl;
}

void TestFileJob::slotOpen(KIO::Job* job)
{
    kDebug()<<job<<endl;
    KIO::FileJob *fileJob = qobject_cast<KIO::FileJob *>(job);
    kDebug()<<"Size = "<<fileJob->size()<<endl;
    
    if (fileJob == m_job1){
        m_opened1 = true;
    } else {
        m_opened2 = true;
    }
}

void TestFileJob::slotClose(KIO::Job*)
{
    kDebug()<<"Closed"<<endl;
}

void TestFileJob::slotPosition(KIO::Job*, KIO::filesize_t offset)
{
    kDebug()<<"Position is now "<<offset<<endl;
}

void TestFileJob::slotRedirection(KIO::Job*, const KUrl& url)
{
    kDebug()<<"New Url="<<url.path()<<endl;
}

void TestFileJob::slotMimetype(KIO::Job*, const QString &mimetype)
{
    kDebug()<<"Mimetype is "<<mimetype<<endl;
}

void TestFileJob::open()
{
    KUrl url = KUrl(QDir::currentPath() + "/data.txt");

    m_job1 = KIO::open(url, QIODevice::ReadOnly);
    connect(m_job1, SIGNAL(open(KIO::Job*)),
                SLOT(slotOpen(KIO::Job*)));
    connect(m_job1, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
    connect(m_job1, SIGNAL(result(KJob*)),
            SLOT(slotResult(KJob*)));
    connect(m_job1, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
            SLOT(slotPosition(KIO::Job*, KIO::filesize_t)));
    connect(m_job1, SIGNAL(redirection(KIO::Job*, const KUrl&)),
            SLOT(slotRedirection(KIO::Job*, const KUrl&)));
    connect(m_job1, SIGNAL(close(KIO::Job*)),
                 SLOT(slotClose(KIO::Job*)));
    connect(m_job1, SIGNAL(mimetype(KIO::Job*, const QString&)),
                 SLOT(slotMimetype(KIO::Job*, const QString&)));
    
    /*m_job2 = KIO::open(url, QIODevice::ReadOnly);
    connect(m_job2, SIGNAL(open(KIO::Job*)),
            SLOT(slotOpen(KIO::Job*)));
    connect(m_job2, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
    connect(m_job2, SIGNAL(result(KJob*)),
            SLOT(slotResult(KJob*)));
    connect(m_job2, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
            SLOT(slotPosition(KIO::Job*, KIO::filesize_t)));
    connect(m_job2, SIGNAL(redirection(KIO::Job*, const KUrl&)),
            SLOT(slotRedirection(KIO::Job*, const KUrl&)));
    connect(m_job2, SIGNAL(close(KIO::Job*)),
            SLOT(slotClose(KIO::Job*)));
    connect(m_job2, SIGNAL(mimetype(KIO::Job*, const QString&)),
            SLOT(slotMimetype(KIO::Job*, const QString&)));*/
}

void TestFileJob::read()
{
    m_job1->read(100024);
    //m_job2->read(100024);
}

void TestFileJob::seek()
{
    m_job1->seek(5);
    //m_job2->seek(5);
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

    TestFileJob* aTestFileJob = new TestFileJob();
    aTestFileJob->open();
    MyThread* aThread = new MyThread(aTestFileJob);
    aThread->start();

    return app.exec();
}
