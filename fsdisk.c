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
    printf("WRITE BITMAP CALLED!\n");
    //print_bitmap(bitmap, bitmap_size);
    lseek(fd, 0, SEEK_SET);
    write(fd, bitmap, bitmap_size);
    return;
}

// Loads the bitmap on the given disk file into memory
void loadbitmap(int fd, uint8_t ** bitmap, uint64_t * bitmap_size){
    create_bitmap(bitmap, bitmap_size);
    printf("LOADBITMAP CALLED : %lu \n", (* bitmap_size));
    lseek(fd, 0, SEEK_SET);
    read(fd, (* bitmap), (* bitmap_size));
    //print_bitmap(*bitmap, *bitmap_size);
    printf("DONE LOAD\n");
    return;
}

// Creates both disk file and metadata disk file of 1MB each
int createdisk(){
    int flag = 0;
    if(!(access("fsdata", F_OK ) != -1 )){
        flag = 1;
        create_bitmap(&datamap, &datamap_size);
        set_bit(&datamap, 0);
        data_fd = open("fsdata", O_CREAT | O_RDWR | O_TRUNC, 0644);
        if(data_fd < 0){
            perror("Data Disk Creation Error!\n");
            return -1;
        }
        writebitmap(data_fd, datamap, datamap_size);
        resetdatafd();
    }
    else{
        printf("LOADING\n");
        data_fd = open("fsdata", O_RDWR, 0644);
        loadbitmap(data_fd, &datamap, &datamap_size);
        resetdatafd();
    }
    if(!(access("fsmeta", F_OK ) != -1 )){
        char * rpath = "/";
        insert_node(rpath);
        flag = 1;
        create_bitmap(&metamap, &metamap_size);
        set_bit(&metamap, 0);
        meta_fd = open("fsmeta", O_CREAT | O_RDWR | O_TRUNC, 0644);
        if(meta_fd < 0){
            perror("Metadata Disk Creation Error!\n");
            return -1;
        }
        writebitmap(meta_fd, metamap, metamap_size);
        resetmetafd();
    }
    else{
        deserialize_metadata_wrapper();
    }
    printf("Create disk returning : %d\n", flag);
    return flag;
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
        return 0;
    }
    unsigned long int parent_inode = node->parent->inode_number;
    printf("RETURNING : %lu\n", parent_inode);
    return parent_inode;
}

// Wrapper function for update_node function
int update_node_wrapper(FStree * node){
    if(meta_fd < 0){
        meta_fd = open("fsmeta", O_RDWR , 0644);
    }
    lseek(meta_fd, (node->inode_number * BLOCK_SIZE), SEEK_SET);
    update_node(meta_fd, metamap, metamap_size, node);
    return 0;
}

// Write the changes in parent node to disk
int update_node(int fd, uint8_t * bitmap, uint64_t bitmap_size, FStree * node){
    clear_bit(&bitmap, node->inode_number);
    write_diskfile(fd, bitmap, bitmap_size, node);
    writebitmap(meta_fd, metamap, metamap_size);
    return 0;
}

// Writes the node values into the file specified by the file descriptor
void write_diskfile(int fd, uint8_t * bitmap, uint64_t bitmap_size, FStree * node){
    printf("WRITING NODE TO DISK\n");
    int childnodes = 0;
    unsigned long int freeblock = find_free_block(bitmap, bitmap_size);
    unsigned long int offset = freeblock * BLOCK_SIZE;
    unsigned long int init_next_block = 0;
    printf("\nNODE   : %s\n", node->path);
    printf("OFFSET : %lu\n", offset);
    printf("BLOCK NUMBER : %lu\n", freeblock);
    unsigned long int parent = get_parent_block(fd, node, freeblock);
    printf("PARENT BLOCK : %lu\n", parent);
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
            printf("CHILD %d %lu %lu : %s\n", childnodes, node->children[childnodes]->inode_number, sizeof(((node->children[childnodes])->inode_number)), node->children[childnodes]->path);
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
    return;
}

// Wrapper function for serialize_metadata
void serialize_metadata_wrapper(FStree * node){
    FStree * temp = node;
    meta_fd = open("fsmeta", O_RDWR , 0644);
    serialize_metadata(temp);
    writebitmap(meta_fd, metamap, metamap_size);
    resetmetafd();
}

// Checks the validity of a given block
int check_validity_block(unsigned long int blocknumber){
    unsigned long int index = blocknumber / 8;
    int bit_index = blocknumber % 8;
    return ((metamap[index] >> bit_index)  & 0x01);
}

// Deletes a metadata block
void delete_metadata_block(unsigned long int blocknumber){
    printf("DELETING BLOCK NUMBER : %lu\n", blocknumber);
    meta_fd = open("fsmeta", O_RDWR , 0644);
    clear_bit(&metamap, blocknumber);
    writebitmap(meta_fd, metamap, metamap_size);
    resetmetafd();
}

// Wrapper function for deseralize metadata
void deserialize_metadata_wrapper(){
    meta_fd = open("fsmeta", O_RDWR , 0644);
    printf("LOADING BITMAP\n");
    loadbitmap(meta_fd, &metamap, &metamap_size);
    //print_bitmap(metamap, metamap_size);
    deserialize_metadata(1);
    resetmetafd();
}

// Load the metadata into memory
void deserialize_metadata(unsigned long int blknumber){
    if(root == NULL){
        printf("NULL\n");
    }
    char buffer[1] = {0};
    int i = 0;
    int pathlen = 1;
    int typelen = 1;
    int num_children = 0;
    mode_t permissions = 0;
    uid_t user_id = 0;
    gid_t group_id = 0;
    time_t a_time = 0;
    time_t b_time = 0;
    time_t c_time = 0;
    time_t m_time = 0;
    off_t size = 0;
    unsigned long int nblk = 0;
    unsigned long int parent = 0;
    unsigned long int inode = 0;
    char * path = (char *)calloc(sizeof(char), 1);
    char * type = (char *)calloc(sizeof(char), 1);
    unsigned long int * children = (unsigned long int *)calloc(sizeof(unsigned long int), 1);
    lseek(meta_fd, blknumber * BLOCK_SIZE, SEEK_SET);
    int readbytes = read(meta_fd, &buffer, 1);
    printf("READING\n");
    while(readbytes){
        switch(buffer[0]){
            case 'P':
                readbytes = read(meta_fd, &buffer, 1);
                switch(buffer[0]){
                    case 'A':
                        for(i = 0; i < 3; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        while(buffer[0] != '\0'){
                            pathlen++;
                            path = (char *)realloc(path, sizeof(char) * pathlen);
                            path[pathlen - 1] = '\0';
                            read(meta_fd, &path[pathlen - 2], 1);
                            buffer[0] = path[pathlen - 2];
                        }
                        printf("PATH FOUND : %s\n", path);
                        if(search_node(path) != NULL){
                            return; 
                        }
                        break;
                    case 'E':
                        readbytes = read(meta_fd, &buffer, 1);
                        if(buffer[0] != 'R'){
                            break;
                        }
                        for(i = 0; i < 2; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        read(meta_fd, &permissions, sizeof(permissions));
                        printf("PERMISSIONS FOUND : (%3o)\n", permissions&0777);
                        break;
                    case 'P':
                        for(i = 0; i < 3; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        read(meta_fd, &parent, sizeof(parent));
                        printf("PARENT FOUND : %lu\n", parent);
                        break;
                }   
                break;
            case 'I':
                readbytes = read(meta_fd, &buffer, 1);
                switch(buffer[0]){
                    case 'N':
                        for(i = 0; i < 3; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        read(meta_fd, &inode, sizeof(inode));
                        printf("INODE FOUND : %lu\n", inode);
                        break;
                }  
                break; 
            case 'T':
                readbytes = read(meta_fd, &buffer, 1);
                switch(buffer[0]){
                    case 'Y':
                        for(i = 0; i < 3; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        while(buffer[0] != '\0'){
                            typelen++;
                            type = (char *)realloc(type, sizeof(char) * typelen);
                            type[typelen - 1] = '\0';
                            read(meta_fd, &type[typelen - 2], 1);
                            buffer[0] = type[typelen - 2];
                        }
                        printf("TYPE FOUND : %s\n", type);
                        break;
                }   
                break;
            case 'N':
                readbytes = read(meta_fd, &buffer, 1);
                switch(buffer[0]){
                    case 'U':
                        for(i = 0; i < 3; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        read(meta_fd, &user_id, sizeof(user_id));
                        printf("USER ID FOUND : %d\n", user_id);
                        break;
                    case 'G':
                        for(i = 0; i < 3; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        read(meta_fd, &group_id, sizeof(group_id));
                        printf("GROUP ID FOUND : %d\n", group_id);
                        break;
                    case 'B':
                        for(i = 0; i < 3; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        read(meta_fd, &nblk, sizeof(nblk));
                        printf("NEXT BLOCK FOUND : %lu\n", nblk);
                        break;
                }  
                break;
            case 'A':
                for(i = 0; i < 3; i++){
                    readbytes = read(meta_fd, &buffer, 1);
                }
                if(buffer[0] != 'M'){
                    break;
                }
                readbytes = read(meta_fd, &buffer, 1);
                read(meta_fd, &a_time, sizeof(a_time));
                printf("ACCESS TIME FOUND : %s", ctime(&a_time));
                break;
            case 'M':
                readbytes = read(meta_fd, &buffer, 1);
                if(buffer[0] != 'T'){
                    break;
                }
                for(i = 0; i < 3; i++){
                    readbytes = read(meta_fd, &buffer, 1);
                }
                readbytes = read(meta_fd, &buffer, 1);
                read(meta_fd, &m_time, sizeof(m_time));
                printf("MODIFIED TIME FOUND : %s", ctime(&m_time));
                break;
            case 'C':
                readbytes = read(meta_fd, &buffer, 1);
                switch(buffer[0]){
                    case 'T':
                        for(i = 0; i < 3; i++){
                            readbytes = read(meta_fd, &buffer, 1);
                        }
                        readbytes = read(meta_fd, &buffer, 1);
                        read(meta_fd, &c_time, sizeof(c_time));
                        printf("CREATION TIME FOUND : %s", ctime(&c_time));
                        break;
                    case 'P':
                        while(buffer[0] != '\0'){
                            printf("CHECK THIS : \n\n %c\n\n", buffer[0]);
                            if(buffer[0] == '<'){
                                printf("%d\n", num_children);
                                num_children++;
                                children = (unsigned long int *)realloc(children, sizeof(unsigned long int) * (num_children));
                                read(meta_fd, &children[num_children - 1], sizeof(children[num_children - 1]));
                                printf("\n\nCHECK : %lu\t", children[num_children - 1]); 
                            }
                            printf("\n\n");
                            read(meta_fd, &buffer, 1);
                        }
                        printf("CHILDREN FOUND : ");
                        for(i = 0; i < num_children; i++){
                            printf("%lu\t", children[i]); 
                        }
                        if(check_validity_block(inode)){
                            load_node(path, type, group_id, user_id, c_time, m_time, a_time, b_time, inode, size);
                            printf("\n");
                            pathlen = 1;
                            typelen = 1;
                            num_children = 0;
                            free(type);
                            free(path);
                            free(children);
                            path = (char *)calloc(sizeof(char), 1);
                            type = (char *)calloc(sizeof(char), 1);
                            children = (unsigned long int *)calloc(sizeof(unsigned long int), 1);
                            inode = -1;
                            permissions = 0;
                            user_id = 0; 
                            group_id = 0;
                            a_time = 0;
                            b_time = 0;
                            c_time = 0;
                            m_time = 0;
                            size = 0;
                            parent = 0;
                            nblk = 0;
                            for(i = 0; i < num_children; i++){
                                deserialize_metadata(children[i]);
                            }
                        }
                        else{
                            printf("\n");
                            pathlen = 1;
                            typelen = 1;
                            num_children = 0;
                            free(type);
                            free(path);
                            free(children);
                            path = (char *)calloc(sizeof(char), 1);
                            type = (char *)calloc(sizeof(char), 1);
                            children = (unsigned long int *)calloc(sizeof(unsigned long int), 1);
                            inode = -1;
                            permissions = 0;
                            user_id = 0; 
                            group_id = 0;
                            a_time = 0;
                            b_time = 0;
                            c_time = 0;
                            m_time = 0;
                            size = 0;
                            parent = 0;
                            nblk = 0;
                        }
                        break;
                }
                break;
            case 'B':
                readbytes = read(meta_fd, &buffer, 1);
                if(buffer[0] != 'T'){
                    break;
                }
                for(i = 0; i < 3; i++){
                    readbytes = read(meta_fd, &buffer, 1);
                }
                readbytes = read(meta_fd, &buffer, 1);
                read(meta_fd, &b_time, sizeof(b_time));
                printf("B TIME FOUND : %s", ctime(&b_time));
                break;
            case 'S':
                for(i = 0; i < 4; i++){
                    readbytes = read(meta_fd, &buffer, 1);
                }
                read(meta_fd, &size, sizeof(size));
                printf("SIZE FOUND : %ld\n", size);
                break;

        }
        readbytes = read(meta_fd, &buffer, 1);
    }
    return;
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
    //close(data_fd);
    //close(meta_fd);
    printf("CLOSED!\n");
    //resetmetafd();
    return 0;
}