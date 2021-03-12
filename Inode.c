#include "Inode.h"

void initInode(Inode* inode) {

    inode->i_size = 0;
    time(&inode->i_ctime);
    inode->i_atime = inode->i_ctime;
    inode->i_mtime = inode->i_ctime;

    inode->i_blocks_cnt = 1;
}

void initDirectoryInode(Inode* inode) {
    inode->i_mode = (0x4000)|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
    initInode(inode);
}

void initFileInode(Inode* inode) {
    inode->i_mode = (0x8000)|S_IRWXU|S_IRWXG|S_IRWXO;
    initInode(inode);
}

bool isDirectory(Inode* inode) {
    return (inode->i_mode & (0x4000));
}

bool isFile(Inode* inode) {
    return (inode->i_mode & (0x8000));
}


bool isDirContainName(Inode* inode, char dir_name[]) {
    for (u_int addr_num = 0; inode->i_block[addr_num] != NULL && addr_num < 15; ++addr_num) {
        if (getInodeNumByName(inode->i_block[addr_num], dir_name) != -1) {
            return true;
        }
    }
    return false;
}

DirectoryEntry* getFreeDirEntryFromInode(Inode* inode) {
    for (u_int addr_num = 0; inode->i_block[addr_num] != NULL && addr_num < 15; ++addr_num) {
        DirectoryEntry* new_dir = getFreeDirectoryEntry(inode->i_block[addr_num]);
        if (new_dir != NULL) {
            return new_dir;
        }
    }
    return NULL;
}
