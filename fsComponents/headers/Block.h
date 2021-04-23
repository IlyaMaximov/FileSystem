#ifndef MINIFS_BLOCK_H
#define MINIFS_BLOCK_H

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>

typedef struct DirectoryEntry DirectoryEntry;
struct DirectoryEntry {
    u_int16_t inode_id; // номер i-node
    char obj_name[14];
};

typedef union BlockData Block;
typedef union BlockData BlockBitmap;
typedef union BlockData DirectoryBlock;
union BlockData {
    char data[1024]; // Блок данных размером 1KB
    DirectoryEntry dir_entries[64];
};

void printBlock(Block* block);

int getBit(u_int bit_num, BlockBitmap* block_bitmap);

void setBit(u_int bit_num, u_int val, BlockBitmap* block_bitmap);

int getFreeBit(u_int bit_cnt, BlockBitmap* block_bitmap);

int isEmptyDirEntry(DirectoryEntry* dir_entry);

DirectoryEntry* getFreeDirectoryEntry(DirectoryBlock* block);

int getInodeNumByName(DirectoryBlock* block, char entry_name[]);

#endif //MINIFS_BLOCK_H
