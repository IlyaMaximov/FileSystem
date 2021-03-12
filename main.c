#include <stdio.h>
#include <stdlib.h>
#include <libsync.h>

#include "MiniFs.h"

extern Inode* USER_TMP_INODE;

void help();
void mkdirCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);
void lsCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);
void cdCommand(Inode* inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);
void rmdirCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);
void touchCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);
void writeCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);
void catCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);
void rmCommand(Inode* parent_inode, char dir_name[MAX_FILE_NAME_LEN], MiniFs* miniFs);
void loadMiniFs(MiniFs* miniFs, char file_name[MAX_FILE_NAME_LEN]);
void saveMiniFs(MiniFs* miniFs, char file_name[MAX_FILE_NAME_LEN]);


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

            fprintf(stdout, "Incorrect command \n");
            fflush(stdout);
        }

        u_int dir_cnt = 0;
        if (strcmp(file_path, "") != 0) {
            dir_cnt = parsePath(file_path, path);
        }
        Inode *inode = goToInode(miniFs, path, dir_cnt);
        if (inode == NULL) {
            fprintf(stdout, "Incorrect path \n");
            fflush(stdout);
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

    printf("Use 'help' if you are experiencing difficulties\n");
    while (1) {
        printf("mini-fs:~$ ");
        fflush(stdout);

        if ((symbol = getchar()) == '\n') {
            continue;
        } else {
            ungetc(symbol, stdin);
        }

        char command[70];
        if (fgets(command, sizeof(command), stdin) == NULL) {
            return;
        }
        int result_code = parseAndExecuteCommand(command, miniFs);
        if (result_code == -1) {
            return;
        }
    }
}


int main() {

    MiniFs* miniFs = calloc(sizeof(MiniFs), 1);
    initMiniFs(miniFs);

    runFs(miniFs);

    free(miniFs);
    return 0;
}
