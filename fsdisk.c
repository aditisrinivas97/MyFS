#include <stdint.h>
#include "fsdisk.h"

uint64_t datamap_size = 32768;
uint8_t * datamap = NULL;
uint64_t metamap_size = 32768;
uint8_t * metamap = NULL;

// Reset file descriptors - signifies that data file are closed
void resetdatafd(){
    close(data_fd);
    data_fd = -1;
    return;
}
// Reset metadata file descriptor - signifies that metadata file is closed
void resetmetafd(){
    close(meta_fd);
    meta_fd = -1;
    return;
}

// Writes the bitmap to the given disk file specified by fd
void writebitmap(int fd, uint8_t * bitmap, uint64_t bitmap_size){
    lseek(fd, 0, SEEK_SET);
    write(fd, bitmap, bitmap_size);
    return;
}

// Loads the bitmap on the given disk file into memory
void loadbitmap(int fd, uint8_t ** bitmap, uint64_t  bitmap_size){
    int i = 0;
    for(i = 0; i < bitmap_size; i++){
        (* bitmap)[i] = '\0';
    }
    lseek(fd, 0, SEEK_SET);
    read(fd, (* bitmap), bitmap_size);
    return;
}

// Creates both disk file and metadata disk file of 1MB each
int createdisk(){
    int wrval = 0;
    if(!(access("fsdata", F_OK ) != -1 )){
        create_bitmap(&datamap, &datamap_size);
        set_bit(&datamap, 0);
        data_fd = open("fsdata", O_CREAT | O_RDWR | O_TRUNC, 0644);
        if(data_fd < 0){
            perror("Data Disk Creation Error!\n");
            return -1;
        }
        while(wrval < INIT_SIZE){
            write(data_fd, '\0', 1);
            wrval++;
        }
        writebitmap(data_fd, datamap, datamap_size);
        resetdatafd();
    }
    if(!(access("fsmeta", F_OK ) != -1 )){
        create_bitmap(&metamap, &metamap_size);
        set_bit(&metamap, 0);
        wrval = 0;
        meta_fd = open("fsmeta", O_CREAT | O_RDWR | O_TRUNC, 0644);
        if(meta_fd < 0){
            perror("Metadata Disk Creation Error!\n");
            return -1;
        }
        while(wrval < INIT_SIZE){
            write(data_fd, '\0', 1);
            wrval++;
        }
        writebitmap(meta_fd, metamap, metamap_size);
        resetmetafd();
    }
    return 0;
}

// Returns the next free block in the given disk file
unsigned long int find_free_block(uint8_t * bitmap, uint64_t bitmap_size){
    unsigned long int freeblock = get_first_unset_bit(bitmap, bitmap_size);
    set_bit(&bitmap, freeblock);
    return freeblock;
}

// Updated the parent block's children array with the given node
void updatechildren(int fd, int parent_blocknumber, FStree * node, int child_blocknumber){
    FILE * fp = fdopen(fd, "r+");
    //printf("UPDATING PARENT : %d\n", parent_blocknumber);
    fseek(fp, parent_blocknumber * BLOCK_SIZE - 1, SEEK_SET);
    //printf("INITIAL : %lu %lu\n", ftell(fp), lseek(fd, 0, SEEK_CUR));
    char *line = NULL;
    size_t len = 0;
    ssize_t lread = 0;
    char * ref = "CPTR=";
    int pos = 0;
    char ch[1] = "\0";
    while ((lread = getline(&line, &len, fp)) != -1){
        //printf("LINE : %s %lu\n", line, ftell(fp));
        pos = 0;
        while((line[pos] == ref[pos]) && pos < 5){
            //printf("%c\t", line[pos]);
            pos++;
        }
        
        if(pos == 5){
            //printf("\n\nIMP : %s", line);
            //printf("LREAD : %ld\n", lread);
            //printf("MID : %lu %lu\n", ftell(fp), lseek(fd, 0, SEEK_CUR));
            int off = -(lread + 1);
            fseek(fp, off, SEEK_CUR);
            lseek(fd, (off_t)ftell(fp), SEEK_SET);
            //printf("NEW : %lu %lu\n", ftell(fp), lseek(fd, 0, SEEK_CUR));
            while(read(fd, ch, 1) > 0){
                //printf("%s\t",ch);
                if(strcmp(ch, "|") == 0){
                    break;
                }
            }
            //printf("\n");
            write(fd, &(child_blocknumber), sizeof(child_blocknumber));
            write(fd, "|\0\n", 3);
            return;
        }
    }
    return;
}

// Returns the block number of the parent of the given node - for metadata
int get_parent_block(int fd, FStree * node, int child_blocknumber){
    if(strcmp(node->path, "/") == 0){
        return -1;
    }
    char * ref = "PATH=";
    FILE * fp = fdopen(fd, "r");
    fseek(fp, BLOCK_SIZE, SEEK_SET);
    char *line = NULL;
    size_t len = 0;
    ssize_t lread = 0;
    int pos = 0;
    int parentpos = 0;
    int blocknumber = 1;
    while ((lread = getline(&line, &len, fp)) != -1) {
        //printf("LINE : %s\n", line);
        pos = 0;
        parentpos = 0;
        while((line[pos] == ref[pos]) && pos < 5){
            pos++;
        }
        if(pos == 5){
            while((line[pos] == node->parent->path[parentpos]) && (node->parent->path[parentpos] != '\0')){
                //printf("%c %c\n", line[pos], node->parent->path[parentpos]);
                pos++;
                parentpos++;
            }
            //printf("OUT\n");
            if(node->parent->path[parentpos] == '\0'){
                printf("Calling update children!\n");
                updatechildren(fd, blocknumber, node, child_blocknumber);
                //free(line);
                return blocknumber;
            }
        }
        if(strcmp(line, "}\n") == 0){
            blocknumber++;
        }
    }
    //printf("Calling update children for root!\n");
    updatechildren(fd, blocknumber, node, child_blocknumber);
    free(line);
    return -1;
}

// Writes the node values into the file specified by the file descriptor
void write_diskfile(int fd, uint8_t * bitmap, uint64_t bitmap_size, FStree * node){
    unsigned long int freeblock = find_free_block(bitmap, bitmap_size);
    unsigned long int offset = freeblock * BLOCK_SIZE;
    int i = 0;
    printf("\nNODE   : %s\n", node->path);
    printf("OFFSET : %lu\n", offset);
    printf("BLOCK NUMBER : %lu\n", freeblock);
    int parent = get_parent_block(fd, node, freeblock);
    printf("PARENT BLOCK : %d\n", parent);
    lseek(fd, offset, SEEK_SET);
    write(fd, OPEN_MARKER, 2);
    write(fd, "PATH=", 5);
    write(fd, node->path, (int)strlen(node->path));
    write(fd, "\0\n", 2);
    write(fd, "TYPE=", 5);
    write(fd, node->type, (int)strlen(node->type));
    write(fd, "\0\n", 2);
    write(fd, "PERM=", 5);
    write(fd, &(node->permissions), sizeof(node->permissions));
    write(fd, "\0\n", 2);
    write(fd, "NUID=", 5);
    write(fd, &(node->user_id), sizeof(node->user_id));
    write(fd, "\0\n", 2);
    write(fd, "NGID=", 5);
    write(fd, &(node->group_id), sizeof(node->group_id));
    write(fd, "\0\n", 2);
    write(fd, "ATIM=", 5);
    write(fd, &(node->a_time), sizeof(node->a_time));
    write(fd, "\0\n", 2);
    write(fd, "MTIM=", 5);
    write(fd, &(node->m_time), sizeof(node->m_time));
    write(fd, "\0\n", 2);
    write(fd, "CTIM=", 5);
    write(fd, &(node->c_time), sizeof(node->c_time));
    write(fd, "\0\n", 2);
    write(fd, "BTIM=", 5);
    write(fd, &(node->b_time), sizeof(node->b_time));
    write(fd, "\0\n", 2);
    write(fd, "SIZE=", 5);
    write(fd, &(node->size), sizeof(node->size));
    write(fd, "\0\n", 2);
    write(fd, "PPTR=", 5);
    write(fd, &(parent), sizeof(parent));
    write(fd, "\0\n", 2);
    if(strcmp(node->type, "directory") == 0){
        write(fd, "CPTR=|", 6);
        for(i = 0; i < 50; i++){
            write(fd, "0000", 4);
        }
    }
    write(fd, "\0\n", 2);
    //lseek(fd, (BLOCK_SIZE * (freeblock + 1) - 3), SEEK_SET); 
    //printf("HOLE : %ld\n", lseek(fd, 0, SEEK_CUR));
    write(fd, CLOSE_MARKER, 2);
    return;
}

// Write metadata to disk file
void serialize_metadata(FStree * temp){
    if(temp == NULL){
        return;
    }
    write_diskfile(meta_fd, metamap, metamap_size, temp);
    for(int i = 0; i < temp->num_children; i++){
        serialize_metadata(temp->children[i]);
    }
    return;
}

// Wrapper function for serialize_metadata
void serialize_metadata_wrapper(FStree * node){
    printf("HERE!\n");
    FStree * temp = node;
    meta_fd = open("fsmeta", O_RDWR , 0644);
    serialize_metadata(temp);
    resetmetafd();
}

// Open the data disk file to access data of file specified by <filename>
int openblock(){
    data_fd = open("fsdata", O_RDWR , 0644);
    if(data_fd < 0){
        perror("Data Disk Open Error!\n");
        return -1;
    }
    resetdatafd();
    // To be completed
    printf("OPENED!\n");
    return 0;
}

// Close the data and metadata disk file
int closeblock(){
    close(data_fd);
    close(meta_fd);
    printf("CLOSED!\n");
    resetmetafd();
    return 0;
}

/*
int main(void){
    insert_node("/\0");
    insert_node("/a\0");
    insert_node("/b\0");
    insert_node("/b/c\0");
    insert_node("/a/d\0");
    insert_node("/b/c/e\0");
    createdisk();
    serialize_metadata_wrapper(root);
    insert_node("/a/d/e");
    FStree * snode = search_node("/a/d/e");
    insert_file("/a/d/e/a.txt");
    serialize_metadata_wrapper(snode);
    return 0;
}*/