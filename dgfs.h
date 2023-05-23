#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <linux/kernel.h>

#define _SIZE_SUPERBLOCK	1024
#define _SIZE_INODE_BITMAP 	2 * 1024
#define _SIZE_DATA_BITMAP	128 * 1024
#define _SIZE_INODE_REGION	4 * 1024 * 1024
#define _SIZE_DATA_REGION 	20 * 1024 * 1024

#define _SIZE_BLOCK_OFFSET	4 * 1024

#define _SIZE_DATA_TYPE_INODE_BITMAP    2

#define _SIZE_INODE_BITMAP_LE16     512
#define _SIZE_DATA_BITMAP_LE16      128 * 512
#define _SIZE_INODE_REGION_LE16     2 * 1024 * 1024


#define _SIZE_INODE_BITMAP_UNIT 2
#define _SIZE_DATA_BITMAP_UNIT  2 
#define _SIZE_INODE 256

#define _SIZE_SUPERBLOCK_UNSIGNED_INT_UNIT  8
#define _SIZE_INODE_LE16_UNIT               8

#define _EXT2_N_BLOCKS       15

#define BLOCK_DIRECT          11
#define BLOCK_SINGLE_INDIRECT 12
#define BLOCK_DOUBLE_INDIRECT 13
#define BLOCK_TRIPLE_INDIRECT 14


#define SUPERBLOCK 		0
#define INODE_BITMAP 	1
#define DATA_BITMAP 	2
#define INODE_REGION 	3
#define DATA_REGION 	4


typedef struct superblock {
	unsigned int 	offset_superblock;
	unsigned int 	offset_inode_bitmap;
	unsigned int 	offset_data_bitmap;
	unsigned int 	offset_inode_region;
	unsigned int 	offset_data_region;
	unsigned int 	num_inodes;
    unsigned int    total_size;
    unsigned int    offset_inode_bitmap_last;
} _superblock_t;



typedef struct inode {
	char 	i_file_name[128]; // bytes
    __le32 	i_block[ _EXT2_N_BLOCKS ];/* Pointers to blocks */  //참고: EXT2_N_BLOCKS = 15
    __le32 	i_uid;        /* Low 16 bits of Owner Uid */
    __le32 	i_gid;        /* Low 16 bits of Group Id */
    __le32 	i_size;        /* Size in bytes */
    __le32 	i_atime;    /* Access time */
    __le32 	i_ctime;    /* Creation time */
    __le32 	i_mtime;    /* Modification time */
    __le32 	i_blocks;    /* Blocks count */
    __le32 	i_links_count;    /* Links count */
} _inode_t;


int create_superblock( FILE * dgfs_file, _superblock_t *_superblock );
int create_inode_bitmap( FILE* dgfs_file, unsigned int offset );
int create_data_bitmap( FILE* dgfs_file, unsigned int offset );
int create_inode_region( FILE* dgfs_file, unsigned int offset );

int create_inode( FILE *dgfs_file, unsigned int offset, unsigned int inode_number );

int write_new_inode( FILE *dgfs_file, FILE *file, char* filename, int file_hashed_size, _superblock_t* _superblock );

int read_superblock( FILE *dgfs_file, _superblock_t* _superblock);
unsigned int read_inode_bitmap( FILE *dgfs_file, _superblock_t* _superblock);



int DGFS_create(char* file);
int DGFS_add(char* dgfs_file, char* filename);
int DGFS_ls();
int DGFS_remove(char* filename);
int DGFS_extract(char* filename, char* output_filename);


int get_le_bit_from_byte( __le16 data ,int offset ) {
	return ( data << ( sizeof(__le16)*8 - 1 - offset ) ) >> ( sizeof(__le16)*8 - 1 );
}



int make_file_hash( FILE *fp1, char* _hash ) {
    MD5_CTX mdContext;
    unsigned char data[1024];
    int bytes;

    // make md5 hash
    // ==================
    MD5_Init( &mdContext );
    while( ( bytes = fread( data, 1, 1024, fp1 ) ) != 0 ) {
        MD5_Update( &mdContext, data, bytes );
    }
    MD5_Final( _hash ,&mdContext );
}