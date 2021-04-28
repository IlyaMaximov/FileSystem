#include "Console.h"
#include "../server/Server.h"

extern Inode* USER_TMP_INODE;

u_int parsePath(char file_path[], char path[MAX_DIR_CNT_PATH][MAX_FILE_NAME_LEN]) {
    u_int path_len = strlen(file_path);
    u_int tmp_name = 0;
    u_int dir_path = 0, dir_end = 0;

    if (file_path[0] == '/') {
        strncpy(path[0], file_path, 1);
        ++dir_path;
        ++dir_end;
        ++tmp_name;
    }

    while (dir_end < path_len) {
        while (dir_end < path_len && file_path[dir_end] != '/') {
            ++dir_end;
        }
        strncpy(path[tmp_name], file_path + dir_path, dir_end - dir_path);
        dir_path = dir_end + 1;
        ++dir_end;
        ++tmp_name;
    }

    if (dir_end - dir_path > 0) {
        strncpy(path[tmp_name], file_path + dir_path, dir_end - dir_path);
        ++tmp_name;
    }
    return tmp_name - 1;

}

int parseAndExecuteCommand(char console_command[], MiniFs* miniFs) {
    char command[15];
    char file_path[50];
    char path[MAX_DIR_CNT_PATH][MAX_FILE_NAME_LEN];

    memset(command, 0, sizeof(command));
    memset(file_path, 0, sizeof(file_path));
    memset(path, 0, sizeof(path));

    sscanf(console_command, "%s %s", command, file_path);

    if (strcmp(command, "help") == 0) {
        help();
    } else if (strcmp(command, "save") == 0) {
        saveMiniFs(miniFs, file_path);
    } else if (strcmp(command, "load") == 0) {
        loadMiniFs(miniFs, file_path);
    } else if (strcmp(command, "exit") == 0) {
        return -1;
    } else {

        if (strcmp(command, "ls") != 0 && strcmp(command, "mkdir") != 0 && strcmp(command, "rm") != 0 &&
            strcmp(command, "touch") != 0 && strcmp(command, "cat") != 0 && strcmp(command, "write") != 0 &&
            strcmp(command, "cd") != 0 && strcmp(command, "rmdir") != 0) {

            sendToClient("Incorrect command \n");
        }

        u_int dir_cnt = 0;
        if (strcmp(file_path, "") != 0) {
            dir_cnt = parsePath(file_path, path);
        }
        Inode *inode = goToInode(miniFs, path, dir_cnt);
        if (inode == NULL) {
            sendToClient("Incorrect path \n");
            return 0;
        }

        if (strcmp(command, "ls") == 0) {
            lsCommand(inode, path[dir_cnt], miniFs);
        } else if (strcmp(command, "mkdir") == 0) {
            mkdirCommand(inode, path[dir_cnt], miniFs);
        } else if (strcmp(command, "cd") == 0) {
            cdCommand(inode, path[dir_cnt], miniFs);
        } else if (strcmp(command, "rmdir") == 0) {
            rmdirCommand(inode, path[dir_cnt], miniFs);
        } else if (strcmp(command, "touch") == 0) {
            touchCommand(inode, path[dir_cnt], miniFs);
        } else if (strcmp(command, "cat") == 0) {
            catCommand(inode, path[dir_cnt], miniFs);
        } else if (strcmp(command, "write") == 0) {
            writeCommand(inode, path[dir_cnt], miniFs);
        } else if (strcmp(command, "rm") == 0) {
            rmCommand(inode, path[dir_cnt], miniFs);
        }
    }
    return 0;
}


void runFs(MiniFs* miniFs) {
    int symbol;
    USER_TMP_INODE = &miniFs->groups_descriptors[0].bg_inode_table[0];

    sendToClient("Use 'help' if you are experiencing difficulties\n");
    while (1) {
        sendToClient("mini-fs:~$ ");

        char* command = receiveFromClient();
        if (strcmp(command, "\n") != 0) {
            int result_code = parseAndExecuteCommand(command, miniFs);
            if (result_code == -1) {
                return;
            }
        }

    }
}

void help() {
    sendToClient("mkdir  *dir_name*  : create empty directories     \n");
    sendToClient("rmdir  *dir_name*  : delete empty directories     \n");
    sendToClient("ls     *dir_name*  : show directories             \n");
    sendToClient("cd     *dir_name*  : navigate between directories \n");
    sendToClient("touch  *file_name* : create empty regular file    \n");
    sendToClient("write  *file_name* : write in regular file        \n");
    sendToClient("cat    *file_name* : read your regular file       \n");
    sendToClient("rm     *file_name* : delete your regular file     \n");
    sendToClient("exit   *file_name* : leave file system            \n");
    sendToClient("-------------------------------------------------\n");
    sendToClient("Special functions: \n");
    sendToClient("save   *file_name*: save mini fs in file on your PC\n");
    sendToClient("load   *file_name*: load mini fs from file on your PC \n");
}


void mkdirCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (isDirContainName(inode, dir_name)) {
        sendToClient("This directory already exist\n");
        return ;
    }

    Inode* new_dir_inode = getFreeInode(miniFs);

    if (new_dir_inode == NULL) {
        sendToClient("File system can not add new directory\n");
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
        sendToClient("This path does not exist \n");
        return;
    } else if (!isDirectory(inode)) {
        sendToClient("Unable to call ls command for regular file \n");
        return;
    }

    for (u_int addr_num = 0; inode->i_block[addr_num] != NULL && addr_num < 15; ++addr_num) {
        Block* data_block = inode->i_block[addr_num];
        for (u_int elem_num = 0; elem_num < 64; ++elem_num) {
            if (!isEmptyDirEntry(&data_block->dir_entries[elem_num])) {
                sendToClient(data_block->dir_entries[elem_num].obj_name);
                sendToClient("\n");
            }
        }
    }
}

void cdCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (strlen(dir_name) != 0 && strcmp(dir_name, "/") != 0) {
        inode = goToNextInode(inode, dir_name, miniFs);
    }

    if (inode == NULL) {
        sendToClient("This path does not exist \n");
        return;
    } else if (!isDirectory(inode)) {
        sendToClient("Unable to call cd command for regular file \n");
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
        sendToClient("This directory does not exist \n");
        return;
    } else if (!isDirectory(inode)) {
        sendToClient("Unable to call rmdir command for regular file \n");
        return;
    } else if (!isEmptyDir(inode, miniFs)) {
        sendToClient("This directory is not empty \n");
        return;
    }


    rmDirectoryEntry(parent_inode, dir_name);
    freeInode(inode, miniFs);
}


void touchCommand(Inode* parent_inode, char file_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (isDirContainName(parent_inode, file_name)) {
        sendToClient("This file or directory already exist\n");
        return ;
    }

    Inode* new_file_inode = getFreeInode(miniFs);

    if (new_file_inode == NULL) {
        sendToClient("File system can not add new file\n");
        return ;
    }

    initFileInode(new_file_inode);

    DirectoryEntry* new_dir_note = getFreeDirEntryFromInode(parent_inode);
    new_dir_note->inode_id = new_file_inode->i_id;
    strncpy(new_dir_note->obj_name, file_name, 14);
}

void writeCommand(Inode* parent_inode, char file_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    char file_text[100000];
    memset(file_text, 0, sizeof(file_text));
    u_int text_size = 0;
    sendToClient("Use ':q' for exit and ':wq' for save result and exit \n");
    while (1) {
        sendToClient("> ");

        char* str_text = receiveFromClient();
        if (strcmp(str_text, ":q\n") == 0) {
            return;
        } else if (strcmp(str_text, ":wq\n") == 0) {
            break;
        } else {
            strcpy(file_text + text_size, str_text);
            text_size += strlen(str_text);
        }
    }
    --text_size;

    Inode* inode = goToNextInode(parent_inode, file_name, miniFs);
    if (inode == NULL) {
        touchCommand(parent_inode, file_name, miniFs);
    }
    inode = goToNextInode(parent_inode, file_name, miniFs);

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

void catCommand(Inode* inode, char file_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    if (strlen(file_name) != 0 && strcmp(file_name, "/") != 0) {
        inode = goToNextInode(inode, file_name, miniFs);
    }

    if (inode == NULL) {
        sendToClient("This file does not exist \n");
        return;
    } else if (!isFile(inode)) {
        sendToClient("Unable to call cat command for directory \n");
        return;
    }

    for (u_int addr_num = 0; addr_num < 15; ++addr_num) {
        Block *data_block = inode->i_block[addr_num];
        if (data_block == NULL) {
            continue;
        }

        sendToClient((char *) data_block);
        sendToClient("\n");
    }
}

void rmCommand(Inode* parent_inode, char file_name[MAX_FILE_NAME_LEN], MiniFs* miniFs) {

    Inode* inode = goToNextInode(parent_inode, file_name, miniFs);

    if (inode == NULL) {
        sendToClient("This file does not exist \n");
        return;
    } else if (!isFile(inode)) {
        sendToClient("Unable to call rm command for directory \n");
        return;
    }

    rmDirectoryEntry(parent_inode, file_name);
    freeInode(inode, miniFs);
}


void saveMiniFs(MiniFs* miniFs, char file_name[MAX_FILE_NAME_LEN]) {
    FILE* fd = fopen(file_name, "wb");
    if (fd == NULL) {
        sendToClient("Couldn't open the file for writing\n");
        return;
    }

    saveFs(miniFs, fd);

    fclose(fd);
}

void loadMiniFs(MiniFs* miniFs, char file_name[MAX_FILE_NAME_LEN]) {
    FILE* fd = fopen(file_name, "rb");
    if (fd == NULL) {
        sendToClient("Couldn't open the file\n");
        return;
    } else if (getFileSize(file_name) < sizeof(struct MiniFs)) {
        sendToClient("File is too small for file system\n");
        fclose(fd);
        return;
    }

    loadFs(miniFs, fd);

    fclose(fd);
}
