#include <stdio.h>
#include <stdlib.h>
#include <libsync.h>

#include "MiniFs.h"

extern Inode* USER_TMP_INODE;

void help() {
    printf("mkdir  *dir_name*  : create empty directories     \n");
    printf("rmdir  *dir_name*  : delete empty directories     \n");
    printf("ls     *dir_name*  : show directories             \n");
    printf("cd     *dir_name*  : navigate between directories \n");
    printf("touch  *file_name* : create empty regular file    \n");
    printf("write  *file_name* : write in regular file        \n");
    printf("cat    *file_name* : read your regular file       \n");
    printf("rm     *file_name* : delete your regular file     \n");
    printf("exit   *file_name* : leave file system            \n");
    printf("-------------------------------------------------\n");
    printf("Special functions: \n");
    printf("save   *file_name*: save mini fs in file on your PC\n");
    printf("load   *file_name*: load mini fs from file on your PC \n");
}

void mkdirCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (isDirContainName(inode, dir_name)) {
        printf("This directory already exist\n");
        fflush(stdout);
        return ;
    }

    Inode* new_dir_inode = getFreeInode(miniFs);

    if (new_dir_inode == NULL) {
        printf("File system can not add new directory\n");
        fflush(stdout);
        return ;
    }

    initDirectoryInode(new_dir_inode);
    addDirectoryEntry(new_dir_inode, miniFs, new_dir_inode->i_id, ".");
    addDirectoryEntry(new_dir_inode, miniFs, inode->i_id, "..");

    DirectoryEntry* new_dir_note = getFreeDirEntryFromInode(inode);
    new_dir_note->inode_id = new_dir_inode->i_id;
    strncpy(new_dir_note->obj_name, dir_name, 14);
}

void lsCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (strlen(dir_name) != 0 && strcmp(dir_name, "/") != 0) {
        inode = goToNextInode(inode, dir_name, miniFs);
    }

    if (inode == NULL) {
        printf("This path does not exist \n");
        return;
    } else if (!isDirectory(inode)) {
        printf("Unable to call ls command for regular file \n");
        return;
    }

    for (u_int addr_num = 0; inode->i_block[addr_num] != NULL && addr_num < 15; ++addr_num) {
        Block* data_block = inode->i_block[addr_num];
        for (u_int elem_num = 0; elem_num < 64; ++elem_num) {
            if (!isEmptyDirEntry(&data_block->dir_entries[elem_num])) {
                printf("%s \n", data_block->dir_entries[elem_num].obj_name);
            }
        }
    }
}

void cdCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (strlen(dir_name) != 0 && strcmp(dir_name, "/") != 0) {
        inode = goToNextInode(inode, dir_name, miniFs);
    }

    if (inode == NULL) {
        printf("This path does not exist \n");
        return;
    } else if (!isDirectory(inode)) {
        printf("Unable to call cd command for regular file \n");
        return;
    }

    USER_TMP_INODE = inode;
}

void rmdirCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    Inode* inode = NULL;
    if (strlen(dir_name) != 0 && strcmp(dir_name, "/") != 0) {
        inode = goToNextInode(parent_inode, dir_name, miniFs);
    }

    if (inode == NULL) {
        printf("This directory does not exist \n");
        return;
    } else if (!isDirectory(inode)) {
        printf("Unable to call rmdir command for regular file \n");
        return;
    } else if (!isEmptyDir(inode, miniFs)) {
        printf("This directory is not empty \n");
        fflush(stdout);
        return;
    }


    rmDirectoryEntry(parent_inode, dir_name);
    freeInode(inode, miniFs);
}

void touchCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (isDirContainName(parent_inode, dir_name)) {
        printf("This file or directory already exist\n");
        fflush(stdout);
        return ;
    }

    Inode* new_file_inode = getFreeInode(miniFs);

    if (new_file_inode == NULL) {
        printf("File system can not add new file\n");
        fflush(stdout);
        return ;
    }

    initFileInode(new_file_inode);

    DirectoryEntry* new_dir_note = getFreeDirEntryFromInode(parent_inode);
    new_dir_note->inode_id = new_file_inode->i_id;
    strncpy(new_dir_note->obj_name, dir_name, 14);
}

void writeCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    char file_text[100000];
    memset(file_text, 0, sizeof(file_text));
    u_int text_size = 0;
    printf("Use ':q' for exit and ':wq' for save result and exit \n");
    while (1) {
        printf("> ");
        fflush(stdout);

        char str_text[12288];
        memset(str_text, 0, sizeof(str_text));
        if (fgets(str_text, sizeof(str_text), stdin) == NULL || strcmp(str_text, ":q\n") == 0) {
            return;
        } else if (strcmp(str_text, ":wq\n") == 0) {
            break;
        } else {
            strcpy(file_text + text_size, str_text);
            text_size += strlen(str_text);
        }
    }
    --text_size;

    Inode* inode = goToNextInode(parent_inode, dir_name, miniFs);
    if (inode == NULL) {
        touchCommand(parent_inode, dir_name, miniFs);
    }
    inode = goToNextInode(parent_inode, dir_name, miniFs);

    u_int text_shift = 0;
    for (u_int addr_num = 0; text_size > 0 && addr_num < 15; ++addr_num) {
        if (inode->i_block[addr_num] == NULL) {
            inode->i_block[addr_num] = getFreeBlock(miniFs);
        }
        Block *data_block = inode->i_block[addr_num];

        u_int text_block_size = text_size;
        if (text_block_size > miniFs->super_block.s_log_block_size) {
            text_block_size = miniFs->super_block.s_log_block_size;
        }

        strncpy((char *) data_block, file_text + text_shift, text_block_size);
        text_shift += text_block_size;
        text_size -= text_block_size;
    }
}

void catCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (strlen(dir_name) != 0 && strcmp(dir_name, "/") != 0) {
        inode = goToNextInode(inode, dir_name, miniFs);
    }

    if (inode == NULL) {
        printf("This file does not exist \n");
        return;
    } else if (!isFile(inode)) {
        printf("Unable to call cat command for directory \n");
        return;
    }

    for (u_int addr_num = 0; addr_num < 15; ++addr_num) {
        Block *data_block = inode->i_block[addr_num];
        if (data_block == NULL) {
            continue;
        }

        puts((char *) data_block);
    }
}

void rmCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    Inode* inode = goToNextInode(parent_inode, dir_name, miniFs);

    if (inode == NULL) {
        printf("This file does not exist \n");
        return;
    } else if (!isFile(inode)) {
        printf("Unable to call rm command for regular file \n");
        return;
    }

    rmDirectoryEntry(parent_inode, dir_name);
    freeInode(inode, miniFs);
}

void saveMiniFs(MiniFs* miniFs, char file_name[MAX_FILE_NAME_LEN]) {
    int fd = open(file_name, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    if (fd == -1) {
        printf("Couldn't open the file for writing\n");
        return;
    }

    int bytes_cnt = write(fd, (char *)miniFs, sizeof(struct MiniFs));
    if (bytes_cnt == -1) {
        fprintf(stderr, "An unexpected error occurred\n");
    }
    close(fd);
}

void loadMiniFs(MiniFs* miniFs, char file_name[MAX_FILE_NAME_LEN]) {
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        printf("Couldn't open the file\n");
        return;
    } else if (getFileSize(file_name) < sizeof(struct MiniFs)) {
        printf("File is too small for file system\n");
        close(fd);
        return;
    }

    int bytes_cnt = read(fd, (char *)miniFs, sizeof(struct MiniFs));
    if (bytes_cnt == -1) {
        fprintf(stderr, "An unexpected error occurred\n");
    }
    close(fd);
}
