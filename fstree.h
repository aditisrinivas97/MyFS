#ifndef FSTREE
#define FSTREE

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct FSfile{
	char * path;
	char * name;
	char * data;
	long int offset;
	off_t size;
};
struct FStree{
    char * path;                // Path upto node
    char * name;                // Name of the file / directory
    char * type;                // Type : "directory" or "file"
    mode_t permissions;		// Permissions 
    uid_t user_id;		// userid
    gid_t group_id;		// groupid
    int num_children;           // Number of children nodes
    int num_files;		// Number of files
    time_t a_time;
    time_t m_time;
    time_t c_time;
    off_t size;
    struct FStree * parent;     // Pointer to parent node
    struct FStree ** children;  // Pointers to children nodes
    struct FSfile ** fchildren; // Pointers to files in the directory
};

//struct FSfile{};

typedef struct FStree FStree;  

typedef struct FSfile FSfile;

time_t t;

extern FStree * root;       // root of the FS tree

char * extract_path(char ** copy_path);
char * reverse(char * str, int mode);
char * extract_dir(char ** copy_path);
FStree * search_node(char * path);
FStree * init_node(const char * path, char * name, FStree * parent,int type);
void insert_node(const char * path);
FSfile * init_file(const char * path,char * name);
void insert_file(const char * path);
void delete_file(const char *path);
void delete_node(const char * path);
FSfile * find_file(const char * path);

#endif