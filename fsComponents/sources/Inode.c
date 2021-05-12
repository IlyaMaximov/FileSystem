#include "../headers/Inode.h"
#include <string.h>

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
    Block* block;
    FOREACH_BLOCK_IN_INODE(block, inode) {
        if (getInodeNumByName(block, dir_name) != -1) {
            return true;
        }
    }
    return false;
}
