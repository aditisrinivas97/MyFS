#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

/*
    This file contains all the functions necessary to support an in memory FS tree.
*/

struct FStree{
    char * path;                // Path upto node
    char * name;                // Name of the file / directory
    char * type;                // Type : "directory" or "file"
    int num_children;           // Number of children nodes
    struct FStree * parent;     // Pointer to parent node
    struct FStree ** children;  // Pointers to children nodes
    //struct FSfile ** fchildren; // Pointers to files in the directory - To be implemented
};

//struct FSfile{};

typedef struct FStree FStree;  

//typedef struct FSfile FSfile;

FStree * root = NULL;       // root of the FS tree

// Function to extract next directory in a given path. E.g "a/b/c" returns "a" and changes the input string to "b/c"
char * extract_path(char ** copy_path){
    char * retval = (char *)calloc(sizeof(char), 1);
    int retlen = 0;
    char temp;
    char * tempstr;
    temp = **(copy_path);
    while(temp != '\0'){    
        if(temp == '/'){
            (*copy_path)++;
            break;
        }
        tempstr = (char *)malloc(sizeof(char) * (retlen + 1));
        strcpy(tempstr, retval);
        retlen += 1;
        tempstr[retlen - 1] = temp;
        retval = (char *)realloc(retval, sizeof(char) * (retlen));
        strcpy(retval, tempstr);
        (*copy_path)++;
        temp = **(copy_path);
        free(tempstr);
    }
    return retval;
}

/* General purpose function to reverse the content of a string. Mode signifies use for dropping extra '/' in directory.
E.g "/a/d/" should produce current dir as 'd' and not '/' */

char * reverse(char * str, int mode){
    int i;
    int len = strlen(str);
    char * retval = (char *)malloc(sizeof(char) * len);
    for(i = 0; i <= len/2; i++){
        retval[i] = str[len - 1 -i];
        retval[len - i - 1] = str[i];
    }
    if(retval[0] == '/' && mode){   // if mode is set to 1, then drop the leading '/'. Set to 0 for normal reversing.
        retval++;
    }
    return retval;
}

// Function to extract current directory in a given path. E.g "/a/b/c" returns "c".
char * extract_dir(char ** copy_path){
    char * retval = (char *)calloc(sizeof(char), 1);
    int retlen = 0;
    char temp;
    char * tempstr;
    *copy_path = reverse(*copy_path, 1);    // change "a/b/c" to "c/b/a" and extract content upto the first '/'
    temp = **(copy_path);
    while(temp != '/'){    
        tempstr = (char *)malloc(sizeof(char) * (retlen + 1));
        strcpy(tempstr, retval);
        retlen += 1;
        tempstr[retlen - 1] = temp;
        retval = (char *)realloc(retval, sizeof(char) * (retlen));
        strcpy(retval, tempstr);
        (*copy_path)++;
        temp = **(copy_path);
        free(tempstr);
    }
    (*copy_path)++;                     // remove the leading '/' from "/b/a" after extracting 'c'
    retval = reverse(retval, 0);        // reverse the content of the return value. E.g if dir was abc, retval would be cba and would need to be reversed.
    *copy_path = reverse(*copy_path, 0);    // reverse the orginial path
    return retval;
}

// Function to search for a node in the FS tree, given the path. Path should not have leading or trailing '/'.
FStree * search_node(char * path){
    FStree * temp = root;
    FStree * retval = NULL;
    char * curr_node = NULL;
    int flag = 0, i = 0;
    while(temp != NULL){
        curr_node = extract_path(&path);
        if(strlen(curr_node) == 0){
            break;
        }
        for(i = 0; i < temp->num_children; i++){
            if(strcmp(temp->children[i]->name, curr_node) == 0){
                retval = temp->children[i];
                temp = temp->children[i];
                flag = 1;
            }
        }
        if(!flag){
            return NULL;
        }
        else{
            flag = 0;
        }
    }
    return retval;
}

// Function to initialise an FS tree node
FStree * init_node(const char * path, char * name, FStree * parent){
    FStree * new = (FStree *)malloc(sizeof(FStree));
    new->path = (char *)path;
    new->name = name;
    new->type = "directory";       // no support for files yet
    new->num_children = 0;
    new->parent = parent;
    new->children = NULL;
    return new;
}

// Function to insert a node into the FS tree
void insert_node(const char * path){
    if(root == NULL){
        root = init_node("/", "root", NULL);
        return;
    }
    else{
        char * copy_path = (char *)path;
        char * dir = extract_dir(&copy_path);
        FStree * dir_node = NULL;
        copy_path++;
        if(strlen(copy_path) == 0){     // if the new directory belongs to the root node
            root->num_children++;
            if(root->children == NULL){
                root->children = (FStree **)malloc(sizeof(FStree *));
                root->children[0] = init_node(path, dir, root);
            }
            else{
                root->children = (FStree **)realloc(root->children, sizeof(FStree *) * root->num_children);
                root->children[root->num_children - 1] = init_node(path, dir, root);
            }
        }
        else{
            dir_node = search_node(copy_path);  // get the parent directory's address
            if(dir_node != NULL){
                dir_node->num_children++;
                dir_node->children = (FStree **)realloc(dir_node->children, sizeof(FStree *) * dir_node->num_children);
                dir_node->children[dir_node->num_children - 1] = init_node(path, dir, dir_node);
            }
            else{
                printf("No such file or directory in the given path!\n");
            }
        }
        return;
    }
    return;
}

// Function to delete a node in the FS tree
void delete_node(const char * path){
    if(root == NULL){
        return;
    }
    else{
        char * copy_path = (char *)path;
        FStree * dir_node = NULL;
        int i, j;
        copy_path++;
        if(strlen(copy_path) == 0){
            printf("Cannot delete root directory!\n");  // do not allow deletion of root directory
            return;
        }
        else{
            dir_node = search_node(copy_path);  // get the directory's address
            if(dir_node->children != NULL){
                for(i = dir_node->num_children - 1; i >= 0; i--){
                    delete_node(dir_node->children[i]->path);       // recursively delete all the sub directories
                }
            }
            //printf("node name : %s \n", dir_node->name);
            for(i = 0; i < dir_node->parent->num_children; i++){
                if(dir_node->parent->children[i] == dir_node){
                    for(j = i + 1; j < dir_node->parent->num_children - 1; j++){
                        dir_node->parent->children[i] = dir_node->parent->children[i + 1];
                    }
                    break;
                }
            }
            dir_node->parent->num_children--;           // remove the node in the parent's children array
            if(dir_node->parent->num_children == 0){
                dir_node->parent->children = NULL;
            }
            else{
                dir_node->parent->children = (FStree **)realloc(dir_node->parent->children,sizeof(FStree *) * dir_node->parent->num_children);
            }
            free(dir_node);
            return;
        }
    }
    return;
}

// For testing purposes only
int main(void){
    char * path = "/";
    insert_node(path);
    char * newpath = "/a";
    insert_node(newpath);
    char * newpath1 = "/a/b";
    insert_node(newpath1);
    char * newpath2 = "/a/b/c";
    insert_node(newpath2);
    char * newpath3 = "/a/b/d";
    insert_node(newpath3);
    char * newpath4 = "/a/b/e";
    insert_node(newpath4);
    FStree * node = search_node("a/b/d");
    delete_node("/");
    node = search_node("a/b/d");
    if(node == NULL){
        printf("failed! No such directory!\n");
    }
    else{
        printf("passed\n");
        printf("node name : %s \n", node->name);
        printf("node path : %s \n", node->path);
        printf("node type : %s \n", node->type);
        printf("node parent : %s \n", node->parent->name);
    }
    return 0;
}