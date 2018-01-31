#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include<sys/stat.h>
/*
    This file contains all the functions necessary to support an in memory FS tree.
*/
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

FStree * root = NULL;       // root of the FS tree

time_t t;
// Function to extract next directory in a given path. E.g "a/b/c" returns "a" and changes the input string to "b/c"
char * extract_path(char ** copy_path){
    char * retval = (char *)calloc(sizeof(char), 1);
    int retlen = 0;
    char temp;
    char * tempstr;
    temp = **(copy_path);
    while(temp != '\0'){    
        if(temp == '/'){
            if(strlen(*copy_path) > 1){
                (*copy_path)++;
            }
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
    retval = (char *)realloc(retval, sizeof(char) * (retlen + 1));
    retval[retlen] = '\0';
    printf("PATH : %s\n", retval);
    return retval;
}

/* General purpose function to reverse the content of a string. Mode signifies use for dropping extra '/' in directory.
E.g "/a/d/" should produce current dir as 'd' and not '/' */

char * reverse(char * str, int mode){
    int i;
    int len = strlen(str);
    printf("REVERSE : %s %d\n", str, len);
    char * retval = (char *)calloc(sizeof(char), (len + 1));
    for(i = 0; i <= len/2; i++){
        retval[i] = str[len - 1 -i];
        retval[len - i - 1] = str[i];
    }
    if(retval[0] == '/' && mode == 1){   // if mode is set to 1, then drop the leading '/'. Set to 0 for normal reversing.
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
    printf("BEGIN EXTRACT : %s\n", *copy_path);
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
    if(strlen(*copy_path) > 1){
        (*copy_path)++;                     // remove the leading '/' from "/b/a" after extracting 'c'
    }
    retval = (char *)realloc(retval, sizeof(char) * (retlen + 1));
    retval[retlen] = '\0';
    retval = reverse(retval, 0);        // reverse the content of the return value. E.g if dir was abc, retval would be cba and would need to be reversed.
    *(copy_path) = reverse(*(copy_path), 0);    // reverse the orginial path
    printf("done reversing\n");
    printf("END OF EXTRACT : %s %s\n", *copy_path, retval);
    return retval;
}

// Function to search for a node in the FS tree, given the path. Path should not have leading or trailing '/'.
FStree * search_node(char * path){
    FStree * temp = root;
    FStree * retval = NULL;
    char * curr_node = NULL;
    int flag = 0, i = 0;
    if(path[0] == '/'){
        path++;
        printf("SEARCH %s\n",path);
    }
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
FStree * init_node(const char * path, char * name, FStree * parent,int type){
    FStree * new = (FStree *)malloc(sizeof(FStree));
    new->path = (char *)path;
    new->name = name;
    if(type==1){
    	new->type = "directory";  
        new->permissions =S_IFDIR | 0755;
    }     
    if(type==0){
	new->type = "file"; 
    	new->permissions = S_IFREG | 0644; 
    }    
    new->group_id = getgid();
    new->user_id = getuid();
    new->c_time=time(&t);
    new->a_time=time(&t);
    new->m_time=time(&t);
    new->num_children = 0;
    new->parent = parent;
    new->children = NULL;
    new->num_files=0;
    new->size=0;
    return new;
}

// Function to insert a node into the FS tree
void insert_node(const char * path){
    if(root == NULL){
        root = init_node("/", "root", NULL,1);
        return;
    }
    else{
        printf("%s\n", path);
        char * copy_path = (char *)path;
        char * dir = extract_dir(&copy_path);
        FStree * dir_node = NULL;
        printf("%s %s\n", copy_path, dir);
        if(strlen(copy_path) == 1){     // if the new directory belongs to the root node
            root->num_children++;
            if(root->children == NULL){
                root->children = (FStree **)malloc(sizeof(FStree *));
                root->children[0] = init_node(path, dir, root,1);
            }
            else{
                root->children = (FStree **)realloc(root->children, sizeof(FStree *) * root->num_children);
                root->children[root->num_children - 1] = init_node(path, dir, root,1);
            }
        }
        else{
            dir_node = search_node(copy_path);  // get the parent directory's address
            if(dir_node != NULL){
                dir_node->num_children++;
                dir_node->children = (FStree **)realloc(dir_node->children, sizeof(FStree *) * dir_node->num_children);
                dir_node->children[dir_node->num_children - 1] = init_node(path, dir, dir_node,1);
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
        if(strlen(copy_path) == 1){
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
                        dir_node->parent->children[j-1] = dir_node->parent->children[j];
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
//function to intialise a file node
FSfile * init_file(const char * path,char * name){
	FSfile * new = (FSfile *)malloc(sizeof(FSfile));
	new->path = (char *)path;
	new->name = name;
	new->data = "";
	new->size=0;
	new->offset=0;
	//new->c_time=time(&t);
	return new;
}
//function to insert file into FStree
void insert_file(const char * path){
	char * copy_path = (char *)path;
        char * name = extract_dir(&copy_path);
	copy_path++;
        if(strlen(copy_path) == 0){ // for files in root
		//printf("filename is :%s and parent directory is root\n",name);
		root->num_children++;
            	if(root->children == NULL){ // if root has no sub-dir or files
                	root->children = (FStree **)malloc(sizeof(FStree *));
                	root->children[0] = init_node(path, name, root,0);
            	}
            	else{ 
                	root->children = (FStree **)realloc(root->children, sizeof(FStree *) * root->num_children);
                	root->children[root->num_children - 1] = init_node(path, name, root,0);
            	}
		if(root->fchildren==NULL){
			root->num_files++;
			root->fchildren= (FSfile **)malloc(sizeof(FSfile *));
			root->fchildren[0]=init_file(path,name);
		}
		else{
			root->num_files++;
			root->fchildren=(FSfile **)realloc(root->fchildren, sizeof(FSfile *) * root->num_files);
			root->fchildren[root->num_files - 1]=init_file(path,name);
		}
	}
	else{
		char * rpath = reverse(reverse(copy_path,0),1);
		FStree * parent_dir_node = search_node(rpath);
		//printf("filename is :%s and parent directory is:%s\n",name,parent_dir_node->name);
		if(parent_dir_node != NULL){
		        parent_dir_node->num_children++;
		        parent_dir_node->children = (FStree **)realloc(parent_dir_node->children, sizeof(FStree *) * parent_dir_node->num_children);
		        parent_dir_node->children[parent_dir_node->num_children - 1] = init_node(path, name, parent_dir_node,0);
			if(parent_dir_node->fchildren==NULL){
				parent_dir_node->num_files++;
				parent_dir_node->fchildren= (FSfile **)malloc(sizeof(FSfile*));
				parent_dir_node->fchildren[0]=init_file(path,name);
			}
			else{
				parent_dir_node->num_files++;
				parent_dir_node->fchildren=(FSfile **)realloc(parent_dir_node->fchildren, sizeof(FSfile *) * parent_dir_node->num_files);
				parent_dir_node->fchildren[parent_dir_node->num_files -1]=init_file(path,name);
			}
            	}
            	else{
                	printf("No such file or directory in the given path!\n");
		}
	}
	printf("\nINSERTION OF FILE INTO TREE DONE\n");
}
//function to delete file from FStree
void delete_file(const char *path)
{
	if(root == NULL){
        	return;
	}
	else{	
		int i,j;
		FStree * parent_dir_node;
		
		char * copy_path = (char *)path;
		char * name = extract_dir(&copy_path);
		copy_path++;
       		if(strlen(copy_path) == 0){
			parent_dir_node=root;
		}
		else{
			
			char * rpath = reverse(reverse(copy_path,0),1);
			parent_dir_node = search_node(rpath);
		}
		for(i=0;i<parent_dir_node->num_children;i++){
			if(strcmp(parent_dir_node->children[i]->name,name)==0){
				
				for(j = i + 1; j <=parent_dir_node->num_children - 1; j++){
                        		parent_dir_node->children[j-1] = parent_dir_node->children[j];
                    		}
                    		break;
                	}
		}
		
		parent_dir_node->num_children--;
		if(parent_dir_node->num_children == 0){
                	parent_dir_node->children = NULL;
            	}
            else{
                	parent_dir_node->children = (FStree **)realloc(parent_dir_node->children,sizeof(FStree *) * parent_dir_node->num_children);
            	}
		for(i=0;i<parent_dir_node->num_files;i++){
			if(strcmp(parent_dir_node->fchildren[i]->name,name)==0){
				
				for(j=i+1;j<=parent_dir_node->num_files-1;j++){
					parent_dir_node->fchildren[j-1]=parent_dir_node->fchildren[j];
				}
				break;
			}
		}
		parent_dir_node->num_files--;
		if(parent_dir_node->num_files == 0){
                	parent_dir_node->fchildren = NULL;
            	}
            	else{
                	parent_dir_node->fchildren = (FSfile **)realloc(parent_dir_node->fchildren,sizeof(FSfile *) * parent_dir_node->num_files);
            	}
		
		printf("\nFile %s deleted",name);
		free(parent_dir_node);
            }
}
//function to search for a file in FStree
FSfile * find_file(const char * path)
{
	char * copy_path = (char *)path;
        char * name = extract_dir(&copy_path);
	copy_path++;
	FStree * parent_dir_node;
	FStree * my_file_tree_node;
	FSfile * my_file;
	int flag=0;
	int i;
	if(strlen(copy_path) == 0){ 
		parent_dir_node = root;
	}
	else{
		char * rpath = reverse(reverse(copy_path,0),1);
		parent_dir_node = search_node(rpath);
	}
	printf("\nparent is :%s",parent_dir_node->name);
	for(i=0;i<parent_dir_node->num_files;i++)
	{
		if(strcmp(parent_dir_node->fchildren[i]->name,name)==0){
			my_file=parent_dir_node->fchildren[i];
			return my_file;
		}
	}
	return NULL;
}		
// For testing purposes only
/*int main(void){
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
    char * newpath_file1="/a/b/c/file1.txt";
    insert_file(newpath_file1);
    char * newpath_file2="/a/b/c/file2.txt";
    insert_file(newpath_file2);
    char * newpath_file6="/a/b/c/file3.txt";
    insert_file(newpath_file6);
    char * newpath_file3="/file3.txt";
    insert_file(newpath_file3);
    char * newpath_file4="/file4.txt";
    insert_file(newpath_file4);
    char * newpath_file5="/file";
    insert_file(newpath_file5);
    FStree * node = search_node("/");
    delete_node("/");
    node = search_node("/file");
    if(node == NULL){
        printf("failed! No such directory!\n");
    }
    else{
        printf("passed\n");
        printf("node name : %s \n", node->name);
        printf("node path : %s \n", node->path);
        printf("node type : %s \n", node->type);
        printf("node parent : %s \n", node->parent->name);
	printf("no of children : %d \n", node->num_children);
    }
    /*
    find_files("/");
    delete_file("/file4.txt");
    printf("\nAfter deleting:");
    find_files("/");
    return 0;
}
*/
