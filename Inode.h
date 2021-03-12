#ifndef MINIFS_INODE_H
#define MINIFS_INODE_H

#include <time.h>
#include <stdbool.h>
#include "Block.h"

typedef struct Inode Inode;
struct Inode {
    u_int16_t i_mode; //Тип, suid, sgid, sticky, права доступа
    u_int16_t i_id; //	Номер i-node
    u_int16_t i_gid; //	Группа

    u_int32_t i_size; //	Размер
    time_t i_ctime; //	Creation time
    time_t i_atime; //	Access time
    time_t i_mtime; //	Modification time

    u_int32_t i_blocks_cnt; //	Число занимаемых блоков
    Block* i_block[15];	// Указатели на блоки данных, sizeof(u_int32_t*) = 8
};


void initInode(Inode* inode);

void initDirectoryInode(Inode* inode);

bool isDirectory(Inode* inode);

bool isFile(Inode* inode);

void initFileInode(Inode* inode);

bool isDirContainName(Inode* inode, char dir_name[]);

DirectoryEntry* getFreeDirEntryFromInode(Inode* inode);

#endif //MINIFS_INODE_H
