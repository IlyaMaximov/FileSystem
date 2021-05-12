#ifndef MINIFS_BLOCK_H
#define MINIFS_BLOCK_H

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define FILE_TITLE_MAX_LEN 14
#define DIRECT_ADDR_IN_INODE_CNT 14
#define INDIRECT_ADDR_BLOCK_NUM 14
#define DIRECT_ADDR_IN_BLOCK_CNT 128

#define FOREACH_BLOCK_IN_INODE(block_ptr, inode) \
    Block* all_blocks[DIRECT_ADDR_IN_INODE_CNT + DIRECT_ADDR_IN_BLOCK_CNT + 1]; \
    memset(all_blocks, 0, sizeof(all_blocks));   \
    u_int32_t block_num = 0; \
    for (u_int addr_num = 0; addr_num < DIRECT_ADDR_IN_INODE_CNT; ++addr_num) { \
        if ((inode)->i_block[addr_num] != NULL) { \
            all_blocks[block_num++] = (inode)->i_block[addr_num]; \
        } \
    } \
    if ((inode)->i_block[INDIRECT_ADDR_BLOCK_NUM] != NULL) { \
        for (u_int addr_num = 0; addr_num < DIRECT_ADDR_IN_BLOCK_CNT; ++addr_num) { \
            if ((inode)->i_block[INDIRECT_ADDR_BLOCK_NUM]->blocks_ptr[addr_num] != NULL) { \
                all_blocks[block_num++] = (inode)->i_block[INDIRECT_ADDR_BLOCK_NUM]->blocks_ptr[addr_num]; \
            } \
        } \
    } \
    (block_ptr) = all_blocks[0]; \
    for (int i = 0; (block_ptr) != NULL; ++i, (block_ptr) = all_blocks[i])

typedef struct DirectoryEntry DirectoryEntry;
struct DirectoryEntry {
    u_int16_t inode_id; // номер i-node
    char obj_name[FILE_TITLE_MAX_LEN];
};

typedef union BlockData Block;
typedef union BlockData BlockBitmap;
typedef union BlockData DirectoryBlock;
union BlockData {
    char data[1024]; // Блок данных размером 1KB
    DirectoryEntry dir_entries[64];
    Block* blocks_ptr[128];
};

void printBlock(Block* block);

void setBit(u_int bit_num, u_int val, BlockBitmap* block_bitmap);
int getFreeBit(u_int bit_cnt, BlockBitmap* block_bitmap);

int isEmptyDirEntry(DirectoryEntry* dir_entry);
int isDefaultDirEntry(DirectoryEntry* dir_entry);

DirectoryEntry* getFreeDirectoryEntry(DirectoryBlock* block);
int getInodeNumByName(DirectoryBlock* block, char entry_name[]);

#endif //MINIFS_BLOCK_H
