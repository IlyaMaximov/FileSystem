#ifndef MINIFS_MINIFS_H
#define MINIFS_MINIFS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libsync.h>
#include <stdbool.h>

#include "SuperBlock.h"
#include "Block.h"
#include "GroupDescriptors.h"
#include "GroupBlocks.h"

#define MAX_DIR_CNT_PATH 15

typedef struct MiniFs MiniFs;
struct MiniFs {
    Block loader;
    SuperBlock super_block;
    GroupDescriptors groups_descriptors[16];
    GroupBlocks groups_blocks[16];
};

Inode* getInodePtr(MiniFs* miniFs, u_int16_t inode_num);
Inode* getFreeInode(MiniFs* miniFs);
Block* getFreeBlock(MiniFs* miniFs);
void freeInode(Inode* inode, MiniFs* miniFs);
DirectoryEntry* getFreeDirEntryFromInode(Inode* inode, MiniFs* miniFs);

bool isFreeDirBlock(Block* block);
bool isEmptyDir(Inode* inode);
int addDirectoryEntry(Inode* inode, MiniFs* miniFs, u_int inode_num, char file_name[]);
int rmNameFromDirectory(Inode* inode, char dir_name[]);

Inode* getRootDirInode(MiniFs* miniFs);
Inode* goToNextInode(Inode* prev_inode, char dir_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);
Inode* goToInode(MiniFs* miniFs, char path[MAX_DIR_CNT_PATH][FILE_TITLE_MAX_LEN], u_int path_depth);

void initRootDirectory(MiniFs* miniFs);
void initMiniFs(MiniFs* miniFs);

void saveFs(MiniFs* miniFs, FILE* file_stream);
void loadFs(MiniFs* miniFs, FILE* file_stream);

#endif //MINIFS_MINIFS_H
