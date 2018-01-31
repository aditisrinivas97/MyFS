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
#include<sys/stat.h>

static int do_getattr(const char *path, struct stat *st){
	printf( "[getattr] Called" );
	printf( "Attributes of %s requested\n", path);

    char * copy_path = (char *)path;
    int i = 0;
    FStree * dir_node = NULL;
    FSfile * file_node = NULL;
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
		    st->st_nlink = 2;
        }
        else{
            st->st_nlink = 1;
	    file_node=find_file(path);
            st->st_size = file_node->size;
        }
    }
	
	st->st_mode = dir_node->permissions;
	st->st_uid = dir_node->user_id; // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = dir_node->group_id; // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = dir_node->a_time; // The last "a"ccess of the file/directory is right now
	st->st_mtime = dir_node->m_time; // The last "m"odification of the file/directory is right now
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
        printf("\n\n\n\n\n here:%s\n\n",path);
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

static int do_create(const char * path, mode_t x,struct fuse_file_info *fi){
	printf("[create file] called! and path is:%s\n",path);
	insert_file(path);
	return 0;
}

static int do_open(const char *path, struct fuse_file_info *fi) {
	printf("\n[open called]\n");
  	return 0;
}

int do_unlink(const char * path){
	printf("\n[unlink called]\n");
	delete_file(path);
	return 0;
}

static int do_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi) {
	printf("\n[read called]\n");
	size_t len;
	FStree * my_file_tree_node;
	FSfile * my_file;
	char * filecontent;
	my_file = find_file(path);
	uid_t u = getuid();
	gid_t g = getgid();
	mode_t p;
	int per_flag=0;
	if(my_file != NULL){
		my_file_tree_node = search_node((char *)path);
		p = my_file_tree_node->permissions;
		if(u == my_file_tree_node->user_id){
			p = p & S_IRUSR;
			if(p == 0400)
				per_flag = 1;
		}
		else if(g == my_file_tree_node->group_id){
			p = p & S_IRGRP;
			if(p == 040)
				per_flag = 1;
		}
		else{
			p = p & S_IROTH;
			if(p == 04)
				per_flag = 1;
		}
		if(per_flag){	
			my_file_tree_node->a_time = time(NULL);
			len = strlen(my_file->data);
			if(len == 0)
				return 0;
			memcpy(buf, my_file->data, size);
			printf("\n read char are :%s and len is :%d\n",buf,(int)strlen(buf));
			return size;
		}
		else
			return -EACCES;
	}
	return -ENOENT;
}

static int do_chmod(const char *path, mode_t new){	
	printf("\n[chmod called]\n");
	FStree * current;
	current = search_node((char *)path);
	if(current != NULL){
		current->permissions = new;
		return 0;
	}
	return -ENOENT;
}

int do_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("\n[Write called]\n");
	int len;
	FStree * my_file_tree_node;
	FSfile * my_file;
	my_file = find_file(path);
	uid_t u = getuid();
	gid_t g = getgid();
	mode_t p;
	int per_flag=0;
	if(my_file != NULL){
		my_file_tree_node = search_node((char *)path);
		p = my_file_tree_node->permissions;
		if(u == my_file_tree_node->user_id){
			p = p & S_IWUSR;
			if(p == 0200)
				per_flag=1;
		}
		else if(g == my_file_tree_node->group_id){
			p = p & S_IWGRP;
			if(p == 020)
				per_flag=1;
		}
		else{
			p = p & S_IWOTH;
			if(p == 02)
				per_flag=1;
		}
		if(per_flag){	
			my_file_tree_node->m_time = time(NULL);
			my_file_tree_node->a_time = time(NULL);
			my_file->data = (char *)realloc(my_file->data, sizeof(char) * (size + offset + 1));
			my_file->size = size + offset;
			memset((my_file->data) + offset, 0, size);
			memcpy((my_file->data) + offset, buf, size);
			my_file_tree_node->size = size + offset;
			printf("content of buf is : %s and len is:%d\n", buf,(int)strlen(buf));
			printf("content is : %s and len is:%d\n", my_file->data,(int)strlen(my_file->data));
			return size;
		}
		else{
			return -EACCES;
		}
	}
	return -ENOENT;
}
static int do_utime(){
	return 0;
}
static struct fuse_operations operations = {
    .getattr	= do_getattr,
	.readdir	= do_readdir,
    .mkdir      = do_mkdir,
    .rmdir      = do_rmdir,
	.create		= do_create,
    .open       = do_open,
    .read		= do_read,
	.unlink     = do_unlink,
    .chmod		= do_chmod,
    .write		= do_write,
	.utimens	= do_utime,
};

int main( int argc, char *argv[] ){
    char * rpath = "/";
    insert_node(rpath);
    insert_file("/file.txt");
    return fuse_main(argc, argv, &operations, NULL);
}

//gcc operations.c -o operations `pkg-config fuse --cflags --libs`

//./operations -f [mount point]
