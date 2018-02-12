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

// Returns the parent block number of the given node
unsigned long int get_parent_block(int fd, FStree * node, int child_blocknumber){
    if(node->parent == NULL){
        return -1;
    }
    unsigned long int parent_inode = node->parent->inode_number;
    return parent_inode;
}

// Wrapper function for update_parent_node function
int update_parent_node_wrapper(FStree * node){
    if(meta_fd < 0){
        meta_fd = open("fsmeta", O_RDWR , 0644);
    }
    lseek(meta_fd, (node->inode_number * BLOCK_SIZE), SEEK_SET);
    update_parent_node(meta_fd, metamap, metamap_size, node);
    return 0;
}

// Write the changes in parent node to disk
int update_parent_node(int fd, uint8_t * bitmap, uint64_t bitmap_size, FStree * node){
    clear_bit(&bitmap, node->inode_number);
    write_diskfile(fd, bitmap, bitmap_size, node);
    return 0;
}

// Writes the node values into the file specified by the file descriptor
void write_diskfile(int fd, uint8_t * bitmap, uint64_t bitmap_size, FStree * node){
    printf("WRITING NODE TO DISK\n");
    int childnodes = 0;
    unsigned long int freeblock = find_free_block(bitmap, bitmap_size);
    unsigned long int offset = freeblock * BLOCK_SIZE;
    unsigned long int init_next_block = -1;
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
    write(fd, "INOD=", 5);
    write(fd, &(freeblock), sizeof(freeblock));
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
    write(fd, "NBLK=", 5);
    write(fd, &(init_next_block), sizeof(init_next_block));
    write(fd, "\0\n", 2);
    if(strcmp(node->type, "directory") == 0){
        write(fd, "CPTR=", 5);
        printf("UPDATE CHILDREN\n");
        while(childnodes < node->num_children){
            printf("CHILD %d : %s\n", childnodes, node->children[childnodes]->path);
            write(fd, "<", 1);
            write(fd, &((node->children[childnodes])->inode_number), sizeof((node->children[childnodes])->inode_number));
            write(fd, ">", 1);
            childnodes++;
        }
        write(fd, "\0\n", 2);
    }    
    printf("CLOSE MARKER AT : %ld\n", lseek(fd, 0, SEEK_CUR));
    write(fd, CLOSE_MARKER, 2);
    node->inode_number = freeblock;
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