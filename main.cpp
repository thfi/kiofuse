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

#define FUSE_USE_VERSION 26
extern "C" {
#include <fuse.h>
}

#include "kiofuseops.h"
#include "kiofuseapp.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <kdebug.h>
#include <kurl.h>
#include <QDir>

static const KAboutData aboutData("kiofuse",
                        0,
                        ki18n("KioFuse"),
                        "0.1",
                        ki18n("Expose KIO filesystems to all POSIX-compliant applications"),
                        KAboutData::License_GPL,
                        ki18n("(c) 2007"),
                        KLocalizedString(),
                        "http://fuse.sourceforge.net",
                        "submit@bugs.kde.org");

bool prepareArguments(KCmdLineArgs *args, KUrl &mountPoint, KUrl &baseUrl)
{
    if (args->isSet("mountpoint")){
        if (QDir(mountPoint.path()).exists()){
            mountPoint = KUrl(args->getOption("mountpoint"));
        }
        else{
            kDebug() <<"The specified mountpoint is not valid"<<endl;
            return 0;
        }
    }
    else{
        kDebug() <<"Please specify the mountpoint"<<endl;
        return 0;
    }

    if (args->isSet("URL")){
        baseUrl = KUrl(args->getOption("URL"));
    }
    else{
        kDebug() <<"Please specify the URL of the remote resource"<<endl;
        return 0;
    }

    return 1;
}

int main (int argc, char *argv[])
{
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("mountpoint <argument>", ki18n("Where to place the remote files within the root hierarchy"));
    options.add("URL <argument>", ki18n("The URL of the remote resource"));
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KUrl mountPoint;
    KUrl baseUrl;

    if (!prepareArguments(args, mountPoint, baseUrl)){
        exit(-1);
    }

    struct fuse_operations ops;
    struct fuse_args fuseArguments = FUSE_ARGS_INIT(0, NULL);
    struct fuse_chan *fuseChannel = 0;
    struct fuse *fuseHandle = 0;

    fuseChannel = fuse_mount(mountPoint.path().toLatin1(), &fuseArguments);
    if (fuseChannel == NULL){
        kDebug()<<"fuse_mount() failed"<<endl;
        exit(-1);
    }

    memset(&ops, 0, sizeof(ops));
    ops.getattr = kioFuseGetAttr;
    ops.open = kioFuseOpen;
    ops.read = kioFuseRead;
    ops.readdir = kioFuseReadDir;

    fuseHandle = fuse_new(fuseChannel, &fuseArguments, &ops, sizeof(ops), NULL);
    if (fuseHandle == 0){
        kDebug()<<"fuse_new() failed"<<endl;
        exit(-1);
    }

    kioFuseApp = new KioFuseApp(baseUrl);

    fuse_loop(fuseHandle);
    fuse_unmount(mountPoint.path().toLatin1(), fuseChannel);
    
    delete kioFuseApp;
    kioFuseApp = 0;
    return 0;
}
