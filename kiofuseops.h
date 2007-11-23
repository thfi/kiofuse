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
//int kioFuseMkNod(const char *relPath, mode_t mode, dev_t rdev);
//int kioFuseMkDir(const char *relPath, mode_t mode);
//int kioFuseUnLink(const char *relPath);
//int kioFuseRmDir(const char *relPath);
//int kioFuseSymLink(const char *from, const char *to);
//int kioFuseReName(const char *from, const char *to);
//FIXME add kioFuseLink
//int kioFuseChmod(const char *relPath, mode_t mode);
//FIXME add kioFuseChOwn
//int kioFuseTruncate(const char *relPath, off_t size);
int kioFuseOpen(const char *relPath, struct fuse_file_info *fi);
int kioFuseRead(const char *relPath, char *buf, size_t size, off_t offs,
                  struct fuse_file_info *fi);
//int kioFuseWrite(const char *relPath, const char *buf, size_t size, off_t offs,
//                   struct fuse_file_info *fi);
//FIXME add kioFuseStatFS
//FIXME add kioFuseFlush
//int kioFuseRelease(const char* relPath, struct fuse_file_info *fi);
//FIXME add kioFuseFSync
//FIXME add kioFuseSetXAttr
//FIXME add kioFuseGetXAttr
//FIXME add kioFuseListXAttr
//FIXME add kioFuseRemoveXAttr
//FIXME add kioFuseOpenDir
int kioFuseReadDir(const char *relPath, void *buf, fuse_fill_dir_t filler,
                   off_t offset, struct fuse_file_info *fi);
//FIXME add kioFuseReadDir
//FIXME add kioFuseReleaseDir
//FIXME add kioFuseFSyncDir
//FIXME add kioFuseInit
//FIXME add kioFuseDestroy
//FIXME add kioFuseAccess
//FIXME add kioFuseCreate
//FIXME add kioFuseFTruncate
//FIXME add kioFuseFGetAttr
//FIXME add kioFuseLock
//FIXME add kioFuseUTimeNS

static void fillStatBufFromFileItem(struct stat *stbuf, KFileItem *item);
static void fillLinkBufFromFileItem(char *buf, size_t size, const QString& dest);

#endif /* KIOFUSE_OPS_H */
