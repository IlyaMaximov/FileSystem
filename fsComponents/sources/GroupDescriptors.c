#include "../headers/GroupDescriptors.h"

void initGroupsDescriptors(GroupDescriptors* groups_descriptors_ptr, SuperBlock* super_block) {
    for (u_int32_t group_num = 0; group_num < super_block->s_groups_count; ++group_num) {

        GroupDescriptors* group_descriptor = groups_descriptors_ptr + group_num;
        Block* group_blocks_ptr = (Block*) (groups_descriptors_ptr + super_block->s_groups_count) +
                group_num * super_block->s_blocks_group_all;

        group_descriptor->bg_block_bitmap = group_blocks_ptr;
        group_descriptor->bg_inode_bitmap = group_blocks_ptr + 1;
        group_descriptor->bg_inode_table = (Inode*) (group_blocks_ptr + 2);
        group_descriptor->bg_data_blocks = group_blocks_ptr + 2 +
                                           (super_block->s_inodes_per_group * sizeof(Inode) / super_block->s_log_block_size);

        group_descriptor->bg_free_blocks_count = super_block->s_blocks_per_group;
        group_descriptor->bg_free_inodes_count = super_block->s_inodes_per_group;
    }
}