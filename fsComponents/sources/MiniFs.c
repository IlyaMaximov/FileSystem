#include "../headers/MiniFs.h"

Inode* USER_TMP_INODE = NULL;

Inode* getInodePtr(MiniFs* miniFs, u_int16_t inode_num) {
    u_int group_num = inode_num / miniFs->super_block.s_inodes_per_group;
    u_int inode_group_num = inode_num % miniFs->super_block.s_inodes_per_group;
    return &miniFs->groups_blocks[group_num].inode_table[inode_group_num];
}

Inode* getFreeInode(MiniFs* miniFs) {
    for (u_int group_num = 0; group_num < miniFs->super_block.s_groups_count; ++group_num) {
        int bit_num = getFreeBit(miniFs->super_block.s_inodes_per_group,
                                 miniFs->groups_descriptors[group_num].bg_inode_bitmap);
        if (bit_num != -1) {
            --miniFs->super_block.s_free_inodes_count;
            --miniFs->groups_descriptors[group_num].bg_free_inodes_count;
            setBit(bit_num, 1, miniFs->groups_descriptors[group_num].bg_inode_bitmap);
            Inode* new_inode = &(*miniFs->groups_descriptors[group_num].bg_inode_table) + bit_num;
            new_inode->i_id = group_num * miniFs->super_block.s_inodes_per_group + bit_num;
            new_inode->i_gid = group_num;
            return new_inode;
        }
    }
    return NULL;
}

Block* getFreeBlock(MiniFs* miniFs) {
    for (u_int group_num = 0; group_num < miniFs->super_block.s_groups_count; ++group_num) {
        int bit_num = getFreeBit(miniFs->super_block.s_blocks_per_group,
                                 miniFs->groups_descriptors[group_num].bg_block_bitmap);
        if (bit_num != -1) {
            --miniFs->super_block.s_free_blocks_count;
            --miniFs->groups_descriptors[group_num].bg_free_blocks_count;
            setBit(bit_num, 1, miniFs->groups_descriptors[group_num].bg_block_bitmap);
            return &miniFs->groups_descriptors[group_num].bg_data_blocks[bit_num];
        }
    }
    return NULL;
}

void freeBlock(DirectoryBlock* block, MiniFs* miniFs) {
    u_int group = -1;

    for (u_int group_num = 0; group_num < miniFs->super_block.s_groups_count; ++group_num) {
        if ((Block*) &miniFs->groups_blocks[group_num] > block) {
            break;
        }
        ++group;
    }

    Block* data_blocks = miniFs->groups_descriptors[group].bg_data_blocks;
    u_int block_num = block - data_blocks;

    --miniFs->groups_descriptors[group].bg_free_blocks_count;
    setBit(block_num, 0, miniFs->groups_descriptors[group].bg_block_bitmap);
    memset(&miniFs->groups_descriptors[group].bg_data_blocks[block_num], 0, sizeof(Block));
}

void freeInode(Inode* inode, MiniFs* miniFs) {
    GroupDescriptors* group_desc = &miniFs->groups_descriptors[inode->i_gid];
    u_int inode_num_group = inode->i_id % miniFs->super_block.s_inodes_per_group;

    ++group_desc->bg_free_inodes_count;
    setBit(inode_num_group, 0, group_desc->bg_inode_bitmap);

    Block* block_ptr;
    FOREACH_BLOCK_IN_INODE(block_ptr, inode) {
        freeBlock(block_ptr, miniFs);
    }
    if (inode->i_block[INDIRECT_ADDR_BLOCK_NUM] != NULL) {
        freeBlock(inode->i_block[INDIRECT_ADDR_BLOCK_NUM], miniFs);
    }

    memset(inode, 0, sizeof(Inode));
}

#define RETURN_FREE_DIR_EMPTY(block_ptr, miniFs) \
    if (*(block_ptr) == NULL) { \
        *(block_ptr) = getFreeBlock((miniFs)); \
    } \
    DirectoryEntry* new_dir_entry = getFreeDirectoryEntry(*(block_ptr)); \
    if (new_dir_entry != NULL) { \
        return new_dir_entry; \
    } \

DirectoryEntry* getFreeDirEntryFromInode(Inode* inode, MiniFs* miniFs) {
    for (u_int addr_num = 0; addr_num < DIRECT_ADDR_IN_INODE_CNT; ++addr_num) {
        RETURN_FREE_DIR_EMPTY(&inode->i_block[addr_num], miniFs);
    }

    if (inode->i_block[INDIRECT_ADDR_BLOCK_NUM] == NULL) {
        inode->i_block[INDIRECT_ADDR_BLOCK_NUM] = getFreeBlock(miniFs);
    }

    for (u_int addr_num = 0; addr_num < DIRECT_ADDR_IN_BLOCK_CNT; ++addr_num) {
        RETURN_FREE_DIR_EMPTY(&inode->i_block[INDIRECT_ADDR_BLOCK_NUM]->blocks_ptr[addr_num], miniFs);
    }

    return NULL;
}

////////////////////////////////////////////////////////////////

bool isFreeDirBlock(Block* block) {
    u_int32_t names_cnt = 0;
    for (u_int elem_num = 0; elem_num < 64; ++elem_num) {
        if (!isDefaultDirEntry(&block->dir_entries[elem_num])) {
            ++names_cnt;
        }
    }
    return (names_cnt == 0);
}

bool isEmptyDir(Inode* inode) {
    Block* block_ptr;
    FOREACH_BLOCK_IN_INODE(block_ptr, inode) {
        if (!isFreeDirBlock(block_ptr)) {
            return false;
        }
    }
    return true;
}

int addDirectoryEntry(Inode* inode, MiniFs* miniFs, u_int inode_num, char file_name[]) {
    DirectoryEntry* new_dir_entry = getFreeDirEntryFromInode(inode, miniFs);

    if (new_dir_entry == NULL) {
        return -1;
    }

    new_dir_entry->inode_id = inode_num;
    strncpy(new_dir_entry->obj_name, file_name, FILE_TITLE_MAX_LEN);
    return 0;

}

int rmNameFromDirectory(Inode* inode, char dir_name[]) {
    Block* block;
    FOREACH_BLOCK_IN_INODE(block, inode) {
        for (u_int elem_num = 0; elem_num < 64; ++elem_num) {
            if (strcmp(block->dir_entries[elem_num].obj_name, dir_name) == 0) {
                memset(&block->dir_entries[elem_num], 0, sizeof(struct DirectoryEntry));
                return 0;
            }
        }
    }
    return -1;
}

////////////////////////////////////////////////////////////////

Inode* getRootDirInode(MiniFs* miniFs) {
    return miniFs->groups_descriptors[0].bg_inode_table;
}

Inode* goToNextInode(Inode* prev_inode, char dir_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs) {
    for (u_int addr_num = 0; prev_inode->i_block[addr_num] != NULL && addr_num < 16; ++addr_num) {
        int i_node_num = getInodeNumByName(prev_inode->i_block[addr_num], dir_name);
        if (i_node_num != -1 && isDirectory(prev_inode)) {
            return getInodePtr(miniFs, i_node_num);
        }
    }
    return NULL;
}

Inode* goToInode(MiniFs* miniFs, char path[MAX_DIR_CNT_PATH][FILE_TITLE_MAX_LEN], u_int path_depth) {
    Inode* inode_ptr;
    u_int dir_num = 0;
    if (strcmp(path[0], "/") == 0) {
        ++dir_num;
        inode_ptr = getRootDirInode(miniFs);
    } else {
        if (!isDirectory(USER_TMP_INODE)) {
            USER_TMP_INODE = &miniFs->groups_descriptors[0].bg_inode_table[0];
        }
        inode_ptr = USER_TMP_INODE;
    }

    while (dir_num < path_depth) {
        if (inode_ptr == NULL) {
            return inode_ptr;
        }
        inode_ptr = goToNextInode(inode_ptr, path[dir_num], miniFs);
        ++dir_num;
    }
    return inode_ptr;
}

////////////////////////////////////////////////////////////////

void initRootDirectory(MiniFs* miniFs) {
    // создание корневой папки это инициализация, поэтому не учитываем её в монтированиях
    Inode* root_dir_inode = getFreeInode(miniFs);
    initDirectoryInode(root_dir_inode);

    addDirectoryEntry(root_dir_inode, miniFs, 0, ".");
    addDirectoryEntry(root_dir_inode, miniFs, 0, "..");
}

void initMiniFs(MiniFs* miniFs) {
    memset(miniFs, 0, sizeof(MiniFs));
    initSuperBlock(&miniFs->super_block);
    initGroupsDescriptors((GroupDescriptors *)&miniFs->groups_descriptors, &miniFs->super_block);
    initRootDirectory(miniFs);
}

////////////////////////////////////////////////////////////////

void saveFs(MiniFs* miniFs, FILE* file_stream) {
    fwrite(miniFs, sizeof(*miniFs), 1, file_stream);

    GroupDescriptors* group_desc = miniFs->groups_descriptors;
    for (u_int group_num = 0; group_num < miniFs->super_block.s_groups_count; ++group_num) {

        long inode_bitmap_shift = (char *) group_desc[group_num].bg_inode_bitmap - (char *) group_desc;
        long block_bitmap_shift = (char *) group_desc[group_num].bg_block_bitmap - (char *) group_desc;
        long inode_table_shift =  (char *) group_desc[group_num].bg_inode_table -  (char *) group_desc;
        long block_data_shift =   (char *) group_desc[group_num].bg_data_blocks -  (char *) group_desc;

        fwrite(&inode_bitmap_shift, sizeof(inode_bitmap_shift), 1, file_stream);
        fwrite(&block_bitmap_shift, sizeof(block_bitmap_shift), 1, file_stream);
        fwrite(&inode_table_shift, sizeof(inode_table_shift), 1, file_stream);
        fwrite(&block_data_shift, sizeof(block_data_shift), 1, file_stream);
    }

    for (u_int group_num = 0; group_num < miniFs->super_block.s_groups_count; ++group_num) {
        for (u_int inode_num = 0; inode_num < miniFs->super_block.s_inodes_count / 16; ++inode_num) {

            Inode inode = miniFs->groups_blocks[group_num].inode_table[inode_num];

            long block_shifts[DIRECT_ADDR_IN_INODE_CNT + 1];
            for (u_int block_num = 0; block_num < DIRECT_ADDR_IN_INODE_CNT + 1; ++block_num) {
                if (inode.i_block[block_num] == NULL) {
                    block_shifts[block_num] = (long) NULL;
                } else {
                    block_shifts[block_num] = (char *) inode.i_block[block_num] - (char *) miniFs->groups_blocks;
                }
            }
            fwrite(block_shifts, sizeof(long), DIRECT_ADDR_IN_INODE_CNT + 1, file_stream);
        }
    }
}

void loadFs(MiniFs* miniFs, FILE* file_stream) {

    fread(miniFs, sizeof(*miniFs), 1, file_stream);

    GroupDescriptors* group_desc = miniFs->groups_descriptors;
    for (u_int group_num = 0; group_num < miniFs->super_block.s_groups_count; ++group_num) {

        long inode_bitmap_shift, block_bitmap_shift, inode_table_shift, block_data_shift;

        fread(&inode_bitmap_shift, sizeof(inode_bitmap_shift), 1, file_stream);
        fread(&block_bitmap_shift, sizeof(block_bitmap_shift), 1, file_stream);
        fread(&inode_table_shift, sizeof(inode_table_shift), 1, file_stream);
        fread(&block_data_shift, sizeof(block_data_shift), 1, file_stream);

        group_desc[group_num].bg_inode_bitmap = (BlockBitmap *) ((char *) group_desc + inode_bitmap_shift);
        group_desc[group_num].bg_block_bitmap = (BlockBitmap *) ((char *) group_desc + block_bitmap_shift);
        group_desc[group_num].bg_inode_table = (Inode *) ((char *) group_desc + inode_table_shift);
        group_desc[group_num].bg_data_blocks = (Block *) ((char *) group_desc + block_data_shift);
    }

    for (u_int group_num = 0; group_num < miniFs->super_block.s_groups_count; ++group_num) {
        for (u_int inode_num = 0; inode_num < miniFs->super_block.s_inodes_count / 16; ++inode_num) {

            Inode* inode = &miniFs->groups_blocks[group_num].inode_table[inode_num];

            long block_shifts[DIRECT_ADDR_IN_INODE_CNT + 1];
            fread(block_shifts, sizeof(long), DIRECT_ADDR_IN_INODE_CNT + 1, file_stream);

            for (u_int block_num = 0; block_num < DIRECT_ADDR_IN_INODE_CNT + 1; ++block_num) {
                if (block_shifts[block_num] == (long) NULL) {
                    inode->i_block[block_num] = NULL;
                } else {
                    inode->i_block[block_num] = (Block *) ((char *) miniFs->groups_blocks + block_shifts[block_num]);
                }
            }
        }
    }
}
