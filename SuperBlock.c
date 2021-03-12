#include "SuperBlock.h"

void initSuperBlock(SuperBlock* super_block) {
    super_block->s_inodes_count = (1 << 16);
    super_block->s_blocks_count = (1 << 17);

    super_block->s_free_blocks_count = (1 << 17);
    super_block->s_free_inodes_count = (1 << 16);

    super_block->s_groups_count = 16;
    super_block->s_blocks_per_group = (1 << 13);
    super_block->s_inodes_per_group = (1 << 12);
    super_block->s_log_block_size = (1 << 10);
    super_block->s_log_inode_size = (1 << 8);

    super_block->s_mnt_count = 0;
    super_block->s_magic = 0xEF53;

    // В каждой группе блоков: Blocks Bitmap, Inode Bitmap, Inode Table, Blocks for Data
    super_block->s_blocks_group_all = 2 + super_block->s_inodes_per_group * sizeof(Inode) /
                                          super_block->s_log_block_size + super_block->s_blocks_per_group;
    // Первые три блока: Loader, SuperBlock, Groups Descriptions Array, потом блоки с данными
    super_block->s_r_blocks_count = 3 + super_block->s_groups_count *
                                        (2 + super_block->s_inodes_per_group * sizeof(Inode) / super_block->s_log_block_size + super_block->s_blocks_per_group);
}

void printSuperBlock(SuperBlock* super_block) {
    printf("Super block size: %ld (should be 1024)\n", sizeof(*super_block));

    printf("Inodes count: %d \n", super_block->s_inodes_count);
    printf("Blocks count: %d \n", super_block->s_blocks_count);
    printf("All blocks count: %d \n", super_block->s_r_blocks_count);

    printf("Free blocks count: %d \n", super_block->s_free_blocks_count);
    printf("Free inodes count: %d \n", super_block->s_free_inodes_count);

    printf("Groups count: %d \n", super_block->s_groups_count);
    printf("Blocks for data in group count: %d \n", super_block->s_blocks_per_group);
    printf("All blocks in group count: %d \n", super_block->s_blocks_group_all);
    printf("Inodes in group count: %d \n", super_block->s_inodes_per_group);
    printf("Block size: %d \n", super_block->s_log_block_size);
    printf("Inode size: %d \n", super_block->s_log_inode_size);

    printf("Mounting count: %d \n", super_block->s_mnt_count);
    printf("Magic number: %x \n", super_block->s_magic);

}