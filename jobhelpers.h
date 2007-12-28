/****************************************************************************
 *    Copyright (c) 2007 Vlad Codrea                                        *
 *    Copyright (c) 2003-2004 by Alexander Neundorf & Kevin 'ervin' Ottens  *
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

#ifndef JOB_HELPERS_H
#define JOB_HELPERS_H

#include "basejobhelper.h"
#include "kiofuseapp.h"

#include <QByteArray>

#include <kio/job.h>
#include <kio/filejob.h>
#include <kio/udsentry.h>

class ListJobHelper : public BaseJobHelper  // Helps list a specified directory
{
    Q_OBJECT

    public:
        ListJobHelper(const KUrl& url, QEventLoop* eventLoop);
        ~ListJobHelper();
        KIO::UDSEntryList entries();  // Sends file and directory info to the FUSE op that started the job
    
    signals:
        void reqListJob(const KUrl&, ListJobHelper*);

    public slots:
        void receiveEntries(KIO::Job*, const KIO::UDSEntryList &entries);  // Store entries so that the FUSE op can get them
    
    protected:
        KIO::UDSEntryList m_entries;  // file and directory info gathered by m_job and given to the FUSE ops that started the job
};

class StatJobHelper : public BaseJobHelper  // Helps stat a specified file or directory
{
    Q_OBJECT

    public:
        StatJobHelper(const KUrl& url, QEventLoop* eventLoop);
        ~StatJobHelper();
        KIO::UDSEntry entry();  // Sends file and directory info to the FUSE op that started the job
    
    signals:
        void reqStatJob(const KUrl&, StatJobHelper*);

    public slots:
        void receiveEntry(const KIO::UDSEntry &entry);  // Store entry so that the FUSE op can get it
        
    protected:
        KIO::UDSEntry m_entry;
};

class OpenJobHelper : public BaseJobHelper  // Helps open a specified file or directory
{
    Q_OBJECT

    public:
        OpenJobHelper(const KUrl& url, const QIODevice::OpenMode& qtMode,
                      QEventLoop* eventLoop);
        ~OpenJobHelper();
        KIO::FileJob* fileJob() {return m_fileJob;}  // Sends fileJob handle to the FUSE op that started the job
    
    signals:
        void reqFileJob(const KUrl&, const QIODevice::OpenMode&, OpenJobHelper*);

    public slots:
        void receiveFileJob(KIO::FileJob* fileJob);  // Store the FileJob so that the FUSE op can get it
        
    protected:
        QIODevice::OpenMode m_qtMode;  // How to open the file (read, write, both, append, etc)
        KIO::FileJob* m_fileJob;  // FIXME Needs to be deleted by close() or the cache cleaner
};

class ReadJobHelper : public BaseJobHelper  // Helps read a specified file
{
    Q_OBJECT

    public:
        ReadJobHelper(KIO::FileJob* fileJob, const KUrl& url, const size_t& size,
                      const off_t& offset, QEventLoop* eventLoop);
        ~ReadJobHelper();
        QByteArray data() {return m_data;}  // Sends data to the FUSE op
    
    signals:
        void reqSeek(KIO::FileJob*, const off_t&, ReadJobHelper*);
        void reqRead(KIO::FileJob*, const size_t&, ReadJobHelper*);
        void sendJobDone(const int&);

    public slots:
        void receivePosition(const off_t& pos, const int& error);
        void receiveData(const QByteArray& data, const int& error);
        
    protected:
        KIO::FileJob* m_fileJob;  // FIXME Needs to be deleted by close() or the cache cleaner
        QByteArray m_data;
        size_t m_size;
        off_t m_offset;
};

class WriteJobHelper : public BaseJobHelper  // Helps write to a specified file
{
    Q_OBJECT

    public:
        WriteJobHelper(KIO::FileJob* fileJob, const KUrl& url, const QByteArray& data,
                                       const off_t& offset, QEventLoop* eventLoop);
        ~WriteJobHelper();
        size_t written() {return m_written;}  // Sends number of
                                              // bytes written to the FUSE op
    
    signals:
        void reqSeek(KIO::FileJob*, const off_t&, WriteJobHelper*);
        void reqWrite(KIO::FileJob*, const QByteArray&, WriteJobHelper*);
        void sendJobDone(const int&);

    public slots:
        void receivePosition(const off_t& pos, const int& error);
        void receiveWritten(const size_t& written, const int& error);
        
    protected:
        KIO::FileJob* m_fileJob;  // FIXME Needs to be deleted by close() or the cache cleaner
        QByteArray m_data;
        size_t m_written;
        off_t m_offset;
};

#endif /* JOB_HELPERS_H */
