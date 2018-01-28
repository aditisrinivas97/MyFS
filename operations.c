#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "fstree.c"

static int do_getattr(const char *path, struct stat *st){
	printf( "[getattr] Called" );
	printf( "Attributes of %s requested\n", path);

    char * copy_path = (char *)path;
    int i = 0;
    FStree * dir_node = NULL;

    if(strlen(copy_path) > 1){
        dir_node = search_node(copy_path);
    }
    else if(strlen(copy_path) == 1){
        dir_node = root;
    }

    if(dir_node == NULL){
        return -ENOENT;
    }
    else{
        if (strcmp(dir_node->type, "directory") == 0){
		    st->st_mode = S_IFDIR | 0755;
		    st->st_nlink = 2;
        }
        else{
            st->st_mode = S_IFREG | 0644;
            st->st_nlink = 1;
            st->st_size = 1024;
        }
    }
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	return 0;
}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
	printf("[readdir] called!");
	
	filler(buffer, ".", NULL, 0 ); // Current Directory
	filler(buffer, "..", NULL, 0 ); // Parent Directory
	
    char * copy_path = (char *)path;
    int i = 0;
    FStree * dir_node = NULL;
    if(strlen(copy_path) > 1){
        dir_node = search_node(copy_path);
    }
    else if(strlen(copy_path) == 1){
        dir_node = root;
        printf("\n\n\n\n\n here\n\n");
    }
    if(dir_node == NULL){
        return -ENOENT;
    }
    else{
        for(i = 0; i < dir_node->num_children; i++){
            filler( buffer, dir_node->children[i]->name, NULL, 0 );
        }
    }
	return 0;
}

int do_mkdir(const char * path, mode_t x){
    printf("[mkdir] called!\n");
    insert_node(path);
    return 0;
}

int do_rmdir(const char * path){
    printf("[rmdir] called!\n");
    delete_node(path);
    return 0;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .mkdir      = do_mkdir,
    .rmdir      = do_rmdir,
};

int main( int argc, char *argv[] ){
    char * rpath = "/";
    insert_node(rpath);
	return fuse_main(argc, argv, &operations, NULL);
}

//gcc operations.c -o operations `pkg-config fuse --cflags --libs`

//./operations -f [mount point]