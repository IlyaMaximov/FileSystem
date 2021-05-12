#include "../headers/Block.h"

int getBit(u_int bit_num, BlockBitmap* block_bitmap) {
    char* data_block = (char *) block_bitmap;
    u_int block_num = bit_num / 8;
    u_int bit_pos = 7 - bit_num % 8;
    return (data_block[block_num] & (1 << bit_pos));
}

void setBit(u_int bit_num, u_int val, BlockBitmap* block_bitmap) {
    if (getBit(bit_num, block_bitmap) && val == 1 || !getBit(bit_num, block_bitmap) && val == 0) {
        return;
    }
    char* data_block = (char *) block_bitmap;
    u_int block_num = bit_num / 8;
    u_int bit_pos = 7 - bit_num % 8;
    if (getBit(bit_num, block_bitmap) && val == 0) {
        data_block[block_num] -= (u_int)(1 << bit_pos);
    } else if (!getBit(bit_num, block_bitmap) && val == 1) {
        data_block[block_num] += (u_int)(1 << bit_pos);
    }
}

int getFreeBit(u_int bit_cnt, BlockBitmap* block_bitmap) {
    for (u_int bit_num = 0; bit_num < bit_cnt; ++bit_num) {
        if (getBit(bit_num, block_bitmap) == 0) {
            return bit_num;
        }
    }
    return -1;
}

int isEmptyDirEntry(DirectoryEntry* dir_entry) {
    return dir_entry->inode_id == 0 && strcmp(dir_entry->obj_name, "") == 0;
}

int isDefaultDirEntry(DirectoryEntry* dir_entry) {
    return strcmp(dir_entry->obj_name, "") == 0 || strcmp(dir_entry->obj_name, ".") == 0 ||
           strcmp(dir_entry->obj_name, "..") == 0;
}

DirectoryEntry* getFreeDirectoryEntry(DirectoryBlock* block) {
    for (u_int elem_num = 0; elem_num < 64; ++elem_num) {
        if (isEmptyDirEntry(&block->dir_entries[elem_num])) {
            return &block->dir_entries[elem_num];
        }
    }
    return NULL;
}

int getInodeNumByName(DirectoryBlock* block, char entry_name[]) {
    for (u_int elem_num = 0; elem_num < 64; ++elem_num) {
        if (strcmp(block->dir_entries[elem_num].obj_name, entry_name) == 0) {
            return block->dir_entries[elem_num].inode_id;
        }
    }
    return -1;
}
