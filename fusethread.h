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

#ifndef FUSE_THREAD_H
#define FUSE_THREAD_H

#define FUSE_USE_VERSION 26
extern "C" {
#include <fuse.h>
}

#include <QThread>

#include <kurl.h>

class FuseThread : public QThread
{
    Q_OBJECT

    public:
        FuseThread(QObject *parent, struct fuse *fuseHandle,
                   struct fuse_chan *fuseChannel, KUrl mountPoint);
        void unmount();
        ~FuseThread();

    protected:
        void run();

    private:
        struct fuse *m_fuseHandle;
        struct fuse_chan *m_fuseChannel;
        KUrl m_mountPoint;
        //bool m_alreadyUnmounted;
};

#endif /* FUSE_THREAD_H */
