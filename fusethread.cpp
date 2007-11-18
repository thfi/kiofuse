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
 
#include "fusethread.h"

#include <QString>

#include <kdebug.h>

FuseThread::FuseThread(QObject *parent, struct fuse *fuseHandle,
                       struct fuse_chan *fuseChannel, KUrl mountPoint)
    : QThread(parent),
      m_fuseHandle(fuseHandle),
      m_fuseChannel(fuseChannel),
      m_mountPoint(mountPoint)
{
    kDebug()<<"QThread::currentThread()"<<QThread::currentThread()<<endl;
}

void FuseThread::run()
{
    kDebug()<<"QThread::currentThread()"<<QThread::currentThread()<<endl;
    
    // Give FUSE the control. It will call functions in ops as they are requested by users of the FS.
    // Since fuse_loop_mt() is used instead of fuse_loop(), every call to the ops will be made in a new thread
    fuse_loop_mt(m_fuseHandle);

    // FUSE has quit its event loop, so we tell it to unmount
    fuse_unmount(m_mountPoint.path().toLatin1(), m_fuseChannel);
}

FuseThread::~FuseThread()
{
    m_fuseHandle = NULL;
    m_fuseChannel = NULL;
}

