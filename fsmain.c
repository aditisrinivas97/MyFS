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
#include "fsoperations.h"

static struct fuse_operations operations = {
        .getattr	= do_getattr,
        .readdir	= do_readdir,
        .mkdir      = do_mkdir,
        .rmdir      = do_rmdir,
        .mknod		= do_mknod,
        .open       = do_open,
        .read		= do_read,
        .unlink     = do_unlink,
        .chmod		= do_chmod,
        .write		= do_write,
        .utime	 = do_utimens,
};

int main( int argc, char *argv[] ){
    char * rpath = "/";
    insert_node(rpath);
    return fuse_main(argc, argv, &operations);
}