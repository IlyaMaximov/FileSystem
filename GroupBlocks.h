#ifndef MINIFS_GROUPBLOCKS_H
#define MINIFS_GROUPBLOCKS_H

#include "Block.h"
#include "Inode.h"

typedef struct GroupBlocks GroupBlocks;
struct GroupBlocks {
    Block blocks_bitmap;
    Block inode_bitmap;
    Inode inode_table[(1 << 12)];
    Block block_data[(1 << 13)];
};

#endif //MINIFS_GROUPBLOCKS_H
