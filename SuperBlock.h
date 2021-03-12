#ifndef MINIFS_SUPERBLOCK_H
#define MINIFS_SUPERBLOCK_H

#include "Inode.h"

typedef struct SuperBlock SuperBlock;
struct SuperBlock {
    uint32_t s_inodes_count;      // Число индексных дескрипторов во всей ФС
    uint32_t s_blocks_count;      // Число блоков, отведённых под блоки данных ФС (т.е без дискриптора групп и пр.)
    uint32_t s_r_blocks_count;    // Число зарезервированных блоков данных (нужных всего для ФС)

    uint32_t s_free_blocks_count; // Число свободных блоков данных
    uint32_t s_free_inodes_count; // Число свободных индексных дескрипторов

    uint32_t s_groups_count;      // Число групп
    uint32_t s_blocks_per_group;  // Число блоков данных в группе
    uint32_t s_inodes_per_group;  // Число индексных дескрипторов в группе
    uint32_t s_log_block_size;    // Размер блока в байтах
    uint32_t s_log_inode_size;    // Размер индексного дескриптора в байтах

    uint32_t s_mnt_count;         // Количество монтирований
    uint32_t s_magic;             // Магическое число ex2fs
    uint32_t s_blocks_group_all;  // Число общее число блоков в группе

    uint32_t s_reserved[243];     // дополнение до 1024 байтов (нужно подогнать в конце)
};

void initSuperBlock(SuperBlock* super_block);

void printSuperBlock(SuperBlock* super_block);

#endif //MINIFS_SUPERBLOCK_H
