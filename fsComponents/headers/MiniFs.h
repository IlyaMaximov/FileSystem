#ifndef MINIFS_MINIFS_H
#define MINIFS_MINIFS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libsync.h>
#include <time.h>
#include <stdbool.h>

#include "SuperBlock.h"
#include "Block.h"
#include "GroupDescriptors.h"
#include "GroupBlocks.h"

#define MAX_DIR_CNT_PATH 10
#define MAX_FILE_NAME_LEN 15


typedef struct MiniFs MiniFs;
struct MiniFs {
    Block loader;
    SuperBlock super_block;
    GroupDescriptors groups_descriptors[16];
    GroupBlocks groups_blocks[16];
};

Inode* getInodePtr(MiniFs* miniFs, u_int16_t inode_num);

Inode* getFreeInode(MiniFs* miniFs);

void freeInode(Inode* inode, MiniFs* miniFs);

Block* getFreeBlock(MiniFs* miniFs);

void initRootDirectory(MiniFs* miniFs);

void initMiniFs(MiniFs* miniFs);

int32_t getFileSize(char file_name[]);

bool isEmptyDir(Inode* inode, MiniFs* miniFs);

int addDirectoryEntry(Inode* inode, MiniFs* miniFs, u_int inode_num, char file_name[]);

int rmDirectoryEntry(Inode* inode, char dir_name[]);

Inode* getRootDirInode(MiniFs* miniFs);

Inode* goToNextInode(Inode* prev_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);

Inode* goToInode(MiniFs* miniFs, char path[MAX_DIR_CNT_PATH][MAX_FILE_NAME_LEN], u_int path_depth);

void saveFs(MiniFs* miniFs, FILE* file_stream);

void loadFs(MiniFs* miniFs, FILE* file_stream);

#endif //MINIFS_MINIFS_H
