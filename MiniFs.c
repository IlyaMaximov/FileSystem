#include "MiniFs.h"

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

void freeEmptyBlock(DirectoryBlock* block, MiniFs* miniFs) {
    u_int group = -1;

    for (u_int group_num = 0; group_num < miniFs->super_block.s_groups_count; ++group_num) {
        if ((Block*) &miniFs->groups_blocks[group_num] > block) {
            break;
        }
        ++group;
    }

    u_int block_num = 0;
    Block* data_blocks = miniFs->groups_descriptors[group].bg_data_blocks;
    for (;block_num < miniFs->super_block.s_blocks_per_group; ++block_num) {
        if (data_blocks + block_num == block) {
            return;
        }
    }

    --miniFs->groups_descriptors[group].bg_free_blocks_count;
    setBit(block_num, 0, miniFs->groups_descriptors[group].bg_block_bitmap);
    memset(&miniFs->groups_descriptors[group].bg_data_blocks[block_num], 0, sizeof(Block));
}

void freeInode(Inode* inode, MiniFs* miniFs) {
    GroupDescriptor* group_desc = &miniFs->groups_descriptors[inode->i_gid];
    u_int inode_num_group = inode->i_id % miniFs->super_block.s_inodes_per_group;

    ++group_desc->bg_free_inodes_count;
    setBit(inode_num_group, 0, group_desc->bg_inode_bitmap);

    for (u_int addr_num = 0; addr_num < 15; ++ addr_num) {
        if (inode->i_block[addr_num] == NULL) {
            continue;
        }
        freeEmptyBlock(inode->i_block[addr_num], miniFs);
    }

    memset(inode, 0, sizeof(Inode));
}

bool isFreeDirBlock(Block* block) {
    int names_cnt = 0;
    for (u_int elem_num = 0; elem_num < 64; ++elem_num) {
        if (!isEmptyDirEntry(&block->dir_entries[elem_num])) {
            ++names_cnt;
        }
    }
    return (names_cnt <= 2);
}

bool isEmptyDir(Inode* inode, MiniFs* miniFs) {
    for (u_int addr_num = 0; addr_num < 15; ++ addr_num) {
        if (inode->i_block[addr_num] == NULL) {
            continue;
        }
        if (!isFreeDirBlock(inode->i_block[addr_num])) {
            return false;
        }
    }
    return true;
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

int addDirectoryEntry(Inode* inode, MiniFs* miniFs, u_int inode_num, char file_name[]) {
    for (u_int addr_num = 0; addr_num < 15; ++ addr_num) {

        if (inode->i_block[addr_num] == NULL) {
            inode->i_block[addr_num] = getFreeBlock(miniFs);
        }

        DirectoryEntry* new_entry = getFreeDirectoryEntry(inode->i_block[addr_num]);
        if (new_entry != NULL) {
            new_entry->inode_id = inode_num;
            strncpy(new_entry->obj_name, file_name, 14);
            return 0;
        }
    }
    return -1;
}

int rmDirectoryEntry(Inode* inode, char dir_name[]) {
    for (u_int addr_num = 0; addr_num < 15; ++ addr_num) {

        Block* block = inode->i_block[addr_num];
        if (block == NULL) {
            continue;
        }
        for (u_int elem_num = 0; elem_num < 64; ++elem_num) {
            if (strcmp(block->dir_entries[elem_num].obj_name, dir_name) == 0) {
                memset(&block->dir_entries[elem_num], 0, sizeof(struct DirectoryEntry));
                return 0;
            }
        }
    }
    return -1;
}

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
    initGroupsDescriptors((GroupDescriptor *)&miniFs->groups_descriptors, &miniFs->super_block);
    initRootDirectory(miniFs);
}

int32_t getFileSize(char file_name[]){
    int32_t file_size = 0;
    struct stat file_stat_buff;
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        return -1;
    }

    if ((fstat(fd, &file_stat_buff) != 0) || (!S_ISREG(file_stat_buff.st_mode))) {
        file_size = -1;
    }
    else {
        file_size = file_stat_buff.st_size;
    }
    close(fd);
    return file_size;
}

Inode* getRootDirInode(MiniFs* miniFs) {
    return miniFs->groups_descriptors[0].bg_inode_table;
}

Inode* goToNextInode(Inode* prev_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {
    for (u_int addr_num = 0; prev_inode->i_block[addr_num] != NULL && addr_num < 16; ++addr_num) {
        int i_node_num = getInodeNumByName(prev_inode->i_block[addr_num], dir_name);
        if (i_node_num != -1 && isDirectory(prev_inode)) {
            return getInodePtr(miniFs, i_node_num);
        }
    }
    return NULL;
}

Inode* goToInode(MiniFs* miniFs, char path[MAX_DIR_CNT_PATH][MAX_FILE_NAME_LEN], u_int path_depth) {
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

