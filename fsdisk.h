#ifndef DISKOPS
#define DISKOPS

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include "fstree.h"
#include "bitmap.h"

#define INIT_SIZE 1048576
#define BLOCK_SIZE 4096
#define OPEN_MARKER "{\n"
#define CLOSE_MARKER "}\n"

int data_fd;
int meta_fd;

uint64_t datamap_size;
uint8_t * datamap;
uint64_t metamap_size;
uint8_t * metamap;

typedef struct FStree FStree;  
typedef struct FSfile FSfile;

void resetdatafd();
void resetmetafd();
void writebitmap(int fd, uint8_t * bitmap, uint64_t bitmap_size);
void loadbitmap(int fd, uint8_t ** bitmap, uint64_t * bitmap_size);
int createdisk();
unsigned long int find_free_block(uint8_t * bitmap, uint64_t bitmap_size);
void write_diskfile(int fd, uint8_t * bitmap, uint64_t bitmap_size, FStree * node);
void serialize_metadata(FStree * temp);
void serialize_metadata_wrapper(FStree * node);
int openblock();
int closeblock();
unsigned long int get_parent_block(int fd, FStree * node, int child_blocknumber);
unsigned long int get_chained_meta_block(int fd, unsigned long int parent_blocknumber, unsigned long int child_blocknumber);
int update_parent_node_wrapper(FStree * node);
int update_parent_node(int fd, uint8_t * bitmap, uint64_t bitmap_size, FStree * node);
void deserialize_metadata(unsigned long int blknumber);
void deserialize_metadata_wrapper();
void delete_metadata_block(unsigned long int blocknumber);
int check_validity_block(unsigned long int blocknumber);
#endif