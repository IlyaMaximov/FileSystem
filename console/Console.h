#ifndef FSCOMPONENTS_CONSOLE_H
#define FSCOMPONENTS_CONSOLE_H

#include <stdio.h>
#include <stdlib.h>
#include <libsync.h>
#include "../fsComponents/headers/MiniFs.h"

int64_t getFileSize(char file_name[]);
u_int parsePath(char file_path[], char path[MAX_DIR_CNT_PATH][FILE_TITLE_MAX_LEN]);
int parseAndExecuteCommand(char console_command[], MiniFs* miniFs);

void runFs(MiniFs* miniFs);
void help();

void mkdirCommand(Inode* inode, char dir_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);
void lsCommand(Inode* inode, char dir_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);
void cdCommand(Inode* inode, char dir_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);
void rmdirCommand(Inode* parent_inode, char dir_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);

void touchCommand(Inode* parent_inode, char file_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);
void writeCommand(Inode* parent_inode, char file_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);
void catCommand(Inode* parent_inode, char file_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);
void rmCommand(Inode* parent_inode, char file_name[FILE_TITLE_MAX_LEN], MiniFs* miniFs);

void loadMiniFs(MiniFs* miniFs, char file_name[FILE_TITLE_MAX_LEN]);
void saveMiniFs(MiniFs* miniFs, char file_name[FILE_TITLE_MAX_LEN]);

#endif //FSCOMPONENTS_CONSOLE_H
