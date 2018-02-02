#ifndef OPERATIONS
#define OPERATIONS

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "fstree.h"

extern FStree * root;

int do_getattr(const char *path, struct stat *st);
int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi );
int do_mkdir(const char * path, mode_t x);
int do_rmdir(const char * path);
int do_mknod(const char * path, mode_t x, dev_t y);
int do_open(const char *path, struct fuse_file_info *fi);
int do_unlink(const char * path);
int do_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
int do_chmod(const char *path, mode_t new);
int do_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int do_utimens(const char *path, struct utimbuf *tv);
int do_truncate(const char *path, off_t size, struct fuse_file_info *fi);
int do_rename(const char* from, const char* to);
int do_access(const char* path,int mask);
#endif
