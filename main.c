#include <stdio.h>
#include <linux/kernel.h>
#include <string.h>
#include <stdlib.h>

typedef struct inode {
	char 	i_file_name[128]; // bytes
    __le32 	i_block[15];/* Pointers to blocks */  //참고: EXT2_N_BLOCKS = 15
    __le32 	i_uid;        /* Low 16 bits of Owner Uid */
    __le32 	i_gid;        /* Low 16 bits of Group Id */
    __le32 	i_size;        /* Size in bytes */
    __le32 	i_atime;    /* Access time */
    __le32 	i_ctime;    /* Creation time */
    __le32 	i_mtime;    /* Modification time */
    __le32 	i_dtime;    /* Deletion Time */
    __le32 	i_blocks;    /* Blocks count */
    __le32 	i_links_count;    /* Links count */
} _inode_t;


int main(void) {

    __le32 array[15] = {0};
    _inode_t inode;
    memcpy( inode.i_block, array, sizeof( __le32 ) * 15 );
    for(int i = 0; i < 15; i++) {
        printf("%d \n", inode.i_block[i]);
    }

    printf("=======\n");
    __le16 data = 1;
    for(int j = 0; j < 16; j++ ) {
        printf("%d\n", data << j );
    }
    return 0;
}

