/****************************************************************************
 *   Copyright (c) 2003-2004 by Alexander Neundorf & Kevin 'ervin' Ottens   *
 *   Copyright (c) 2007-2008 Vlad Codrea                                    *
 *   Copyright (c) 2015 Thomas Fischer                                      *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License.     *
 *                                                                          *
 ****************************************************************************/

#include "kiofuseops.h"
#include "kiofuseapp.h"
#include "fusethread.h"

#include <signal.h>

// Needed for QDir.exists() when checking that mountPoint is a directory
#include <QDir>

#include <KAboutData>
#include <KCmdLineArgs>
#include <kdebug.h>


// This pointer to the fuse thread is used by main() and exitHandler()
FuseThread* fuseThread = NULL;


static const KAboutData aboutData("kiofuse",
                        NULL,
                        ki18n("KioFuse"),
                        "0.1",
                        ki18n("Expose KIO filesystems to all POSIX-compliant applications"),
                        KAboutData::License_GPL_V3,
                        ki18n("(c) 2007-2015 The KioFuse Authors"),
                        KLocalizedString(),
                        "https://techbase.kde.org/Projects/KioFuse",
                        "submit@bugs.kde.org");


static void exitHandler(int)
{
    if (fuseThread != NULL)
    {
        //kdDebug()<<"exit";
        fuseThread->unmount();
        //kdDebug()<<"Quitting kioFuseApp";
        exit(0);
        //QMetaObject::invokeMethod(kioFuseApp, "aboutToQuit");
        //QMetaObject::invokeMethod(kioFuseApp, "quit");
    }
}


static void set_signal_handlers()
{
    struct sigaction sa;

    sa.sa_handler = exitHandler;
    sigemptyset(&(sa.sa_mask));
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) == -1
        || sigaction(SIGINT, &sa, NULL) == -1
        || sigaction(SIGQUIT, &sa, NULL) == -1
        || sigaction(SIGTERM, &sa, NULL) == -1)
    {
        kdDebug()<<"Cannot set exit signal handlers.";
        exit(1);
    }

    sa.sa_handler = SIG_IGN;

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
    {
        kdDebug()<<"Cannot set ignored signals.";
        exit(1);
    }
}


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

    // Initialize mountPoint and baseUrl as specified on the commandline
    if (!prepareArguments(args, mountPoint, baseUrl)){
        // Quit program if a needed argument is not provided by the user
        exit(1);
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
        exit(1);
    }

    // Connect the FS operations used by FUSE to their respective KioFuse implementations
    memset(&ops, 0, sizeof(ops));
    ops.getattr = kioFuseGetAttr;
    ops.readlink = kioFuseReadLink;
    ops.mkdir = kioFuseMkDir;
    ops.unlink = kioFuseUnLink;
    ops.rmdir = kioFuseRmDir;
    ops.mknod = kioFuseMkNod;
    ops.symlink = kioFuseSymLink;
    ops.rename = kioFuseReName;
    ops.chmod = kioFuseChMod;
    ops.open = kioFuseOpen;
    ops.read = kioFuseRead;
    ops.write = kioFuseWrite;
    ops.readdir = kioFuseReadDir;
    ops.release = kioFuseRelease;
    ops.truncate = kioFuseTruncate;
    //ops.access = kioFuseAccess;
    ops.utimens = kioFuseUTimeNS;

    // Tell FUSE about the KioFuse implementations of FS operations
    fuseHandle = fuse_new(fuseChannel, &fuseArguments, &ops, sizeof(ops), NULL);
    if (fuseHandle == NULL){
        kDebug()<<"fuse_new() failed"<<endl;
        exit(1);
    }

    // Holds persistent info (ie. the FS cache)
    kioFuseApp = new KioFuseApp(baseUrl, mountPoint);
    kDebug()<<"kioFuseApp->thread()"<<kioFuseApp->thread()<<endl;

    // Enable QMetaObject::invokeMethod to work with unusual types like off_t
    kioFuseApp->setUpTypes();

    // Translate between KIO::Error and errno.h
    //kioFuseApp->setUpErrorTranslator();

    // Start FUSE's event loop in a separate thread
    fuseThread = new FuseThread(NULL, fuseHandle, fuseChannel, mountPoint);
    fuseThread->start();

    set_signal_handlers();

    // An event loop needs to run in the main thread so that
    // we can connect to slots in the main thread using Qt::QueuedConnection
    kioFuseApp->exec();

    // kioFuseApp has quit its event loop.
    // Execution will never reach here because exitHandler calls exit(0),
    // but we try to wrap up things nicely nonetheless.

    // Use terminate() rather than quit() because the fuse_loop_mt() that is
    // running in the thread may not yet have returned, so we have to force it.
    // Also, calling quit() and then deleting the thread causes crashes.
    fuseThread->terminate();

    delete fuseThread;
    fuseThread = NULL;

    delete kioFuseApp;
    kioFuseApp = NULL;

    return 0;
}
