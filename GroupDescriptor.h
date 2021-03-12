#include "Block.h"

#ifndef MINIFS_GROUPDESCRIPTOR_H
#define MINIFS_GROUPDESCRIPTOR_H

#include "Inode.h"
#include "SuperBlock.h"

typedef struct GroupDescriptor GroupDescriptor;
struct GroupDescriptor {
    BlockBitmap* bg_block_bitmap;  // Адрес битовой карты свободных блоков
    BlockBitmap* bg_inode_bitmap;  // Адрес битовой карты свободных индексных дескрипторов
    Inode* bg_inode_table;        // Адрес таблицы индексных дескрипторов
    Block* bg_data_blocks;         // Адрес блоков данных

    uint32_t bg_free_blocks_count; // Количество свободных блоков в группе
    uint32_t bg_free_inodes_count; // Количество свободных индексных дескрипторов в группе
    uint32_t bg_pad[6];            // Выравнивание до размера 64B (теперь весь BGD влезает в блок)
};

void initGroupsDescriptors(GroupDescriptor* groups_descriptors_ptr, SuperBlock* super_block);


#endif //MINIFS_GROUPDESCRIPTOR_H
