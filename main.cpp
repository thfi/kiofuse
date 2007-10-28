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

#define FUSE_USE_VERSION 26
extern "C" {
#include <fuse.h>
}

#include "kiofuseops.h"
#include "kiofuseapp.h"

#include <QDir>  // Needed for QDir.exists() when checking that mountPoint is a directory

#include <KAboutData>
#include <KCmdLineArgs>
#include <kapplication.h>
#include <kdebug.h>
#include <kurl.h>

static const KAboutData aboutData("kiofuse",
                        NULL,
                        ki18n("KioFuse"),
                        "0.1",
                        ki18n("Expose KIO filesystems to all POSIX-compliant applications"),
                        KAboutData::License_GPL,
                        ki18n("(c) 2007"),
                        KLocalizedString(),
                        "http://fuse.sourceforge.net",
                        "submit@bugs.kde.org");

// Initializes mountPoint and baseUrl as specified on the command line
// Returns false if needed argument is not found
bool prepareArguments(KCmdLineArgs *args, KUrl &mountPoint, KUrl &baseUrl)
{
    if (args->isSet("mountpoint")){
        if (QDir(mountPoint.path()).exists()){
            mountPoint = KUrl(args->getOption("mountpoint"));
        }
        else{
            kDebug() <<"The specified mountpoint is not valid"<<endl;
            return false;
        }
    }
    else{
        kDebug() <<"Please specify the mountpoint"<<endl;
        return false;
    }

    if (args->isSet("URL")){
        baseUrl = KUrl(args->getOption("URL"));
    }
    else{
        kDebug() <<"Please specify the URL of the remote resource"<<endl;
        return false;
    }

    return true;
}

int main (int argc, char *argv[])
{
    // KDE initialization
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("mountpoint <argument>", ki18n("Where to place the remote files within the root hierarchy"));
    options.add("URL <argument>", ki18n("The URL of the remote resource"));
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KUrl mountPoint;  // Local path where files will appear
    KUrl baseUrl;  // Remote location of the resource

    if (!prepareArguments(args, mountPoint, baseUrl)){  // Initialize mountPoint and baseUrl as specified on the commandline
        exit(-1);  // Quit program if a needed argument is not provided by the user
    }

    // FUSE variables
    struct fuse_operations ops;
    struct fuse_args fuseArguments = FUSE_ARGS_INIT(0, NULL);
    struct fuse_chan *fuseChannel = NULL;
    struct fuse *fuseHandle = NULL;

    // Tell FUSE where the local mountpoint is
    fuseChannel = fuse_mount(mountPoint.path().toLatin1(), &fuseArguments);
    if (fuseChannel == NULL){
        kDebug()<<"fuse_mount() failed"<<endl;
        exit(-1);
    }

    // Connect the FS operations used by FUSE to their respective KioFuse implementations
    memset(&ops, 0, sizeof(ops));
    ops.getattr = kioFuseGetAttr;
    ops.open = kioFuseOpen;
    ops.read = kioFuseRead;
    ops.readdir = kioFuseReadDir;

    // Tell FUSE about the KioFuse implementations of FS operations
    fuseHandle = fuse_new(fuseChannel, &fuseArguments, &ops, sizeof(ops), NULL);
    if (fuseHandle == NULL){
        kDebug()<<"fuse_new() failed"<<endl;
        exit(-1);
    }

    KApplication app(false);  // Disable GUI
    kioFuseApp = new KioFuseApp(baseUrl); // Holds persistent info (ie. the FS cache)
    kDebug()<<"kioFuseApp->thread()"<<kioFuseApp->thread()<<endl;

    // Give FUSE the control. It will call functions in ops as they are requested by users of the FS.
    // Since fuse_loop_mt() is used instead of fuse_loop(), every call to the ops will be made in a new thread
    fuse_loop_mt(fuseHandle);

    // FUSE has quit its event loop, so we'll quit too
    fuse_unmount(mountPoint.path().toLatin1(), fuseChannel);
    delete kioFuseApp;
    kioFuseApp = NULL;
    return 0;
}
