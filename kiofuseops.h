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

#ifndef KIOFUSE_OPS_H
#define KIOFUSE_OPS_H

#define FUSE_USE_VERSION 26
extern "C" {
#include <fuse.h>
}

#include <QString>

#include <KFileItem>

int kioFuseGetAttr(const char *relPath, struct stat *stbuf);
int kioFuseReadLink(const char *relPath, char *buf, size_t size);
int kioFuseMkNod(const char *relPath, mode_t mode, dev_t /*rdev*/);
int kioFuseMkDir(const char *relPath, mode_t mode);
int kioFuseUnLink(const char *relPath);
int kioFuseRmDir(const char *relPath);
int kioFuseSymLink(const char *from, const char *to);
int kioFuseReName(const char *from, const char *to);
int kioFuseLink(const char *from, const char *to);
int kioFuseChMod(const char *relPath, mode_t mode);
int kioFuseChOwn(const char *path, uid_t uid, gid_t gid);
int kioFuseTruncate(const char *relPath, off_t size);
int kioFuseOpen(const char *relPath, struct fuse_file_info *fi);
int kioFuseRead(const char *relPath, char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi);
int kioFuseWrite(const char *relPath, const char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi);
int kioFuseStatFs(const char *path, struct statvfs *stbuf);
//FIXME add kioFuseFlush
int kioFuseRelease(const char* relPath, struct fuse_file_info *fi);
int kioFuseFSync(const char *path, int isdatasync, struct fuse_file_info *fi);
int kioFuseSetXAttr(const char *path, const char *name, const char *value, size_t size, int flags);
int kioFuseGetXAttr(const char *path, const char *name, char *value, size_t size);
int kioFuseListXAttr(const char *path, char *list, size_t size);
int kioFuseRemoveXAttr(const char *path, const char *name);
//FIXME add kioFuseOpenDir
int kioFuseReadDir(const char *relPath, void *buf, fuse_fill_dir_t filler,
                   off_t /*offset*/, struct fuse_file_info* /*fi*/);
//FIXME add kioFuseReleaseDir
//FIXME add kioFuseFSyncDir
//FIXME add kioFuseInit
//FIXME add kioFuseDestroy
//int kioFuseAccess(const char *relPath, int mask); // TODO when KIO can check permissions
//FIXME add kioFuseCreate
//int kioFuseFTruncate(const char *relPath, off_t size, struct fuse_file_info *fi);  // TODO only possible if KIO gets a truncate method
//FIXME add kioFuseFGetAttr
//FIXME add kioFuseLock
int kioFuseUTimeNS(const char *relPath, const struct timespec ts[2]);
// TODO #ifdef HAVE_POSIX_FALLOCATE
int kioFuseFAllocate(const char *path, int mode, off_t offset, off_t length, struct fuse_file_info *fi);
// TODO #endif // HAVE_POSIX_FALLOCATE

void fillStatBufFromFileItem(struct stat *stbuf, KFileItem *item);
void fillLinkBufFromFileItem(char *buf, size_t size, const QString& dest);
QIODevice::OpenMode modeFromPosix(int flags);

#endif /* KIOFUSE_OPS_H */
