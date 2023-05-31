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
#define _SIZE_DATA_BITMAP_LE16      64 * 1024
#define _SIZE_INODE_REGION_LE16     2 * 1024 * 1024


#define _SIZE_INODE_BITMAP_UNIT 2
#define _SIZE_DATA_BITMAP_UNIT  2 
#define _SIZE_INODE 256
#define _SIZE_DATA MD5_DIGEST_LENGTH

#define _SIZE_SUPERBLOCK_UNSIGNED_INT_UNIT  8
#define _SIZE_INODE_LE16_UNIT               8

#define _EXT2_N_BLOCKS          15
#define _EXT2_N_BLOCKS_DIRECT   12

#define BLOCK_DIRECT          11
#define BLOCK_SINGLE_INDIRECT 12
#define BLOCK_DOUBLE_INDIRECT 13
#define BLOCK_TRIPLE_INDIRECT 14


#define SUPERBLOCK 		0
#define INODE_BITMAP 	1
#define DATA_BITMAP 	2
#define INODE_REGION 	3
#define DATA_REGION 	4

#define MODE_BITMAP_WRITE_PLUS  0
#define MODE_BITMAP_WRITE_MINUS 1

typedef struct superblock {
	unsigned int 	offset_superblock;
	unsigned int 	offset_inode_bitmap;
	unsigned int 	offset_data_bitmap;
	unsigned int 	offset_inode_region;
	unsigned int 	offset_data_region;

    unsigned int    total_size;
    unsigned int 	total_inodes;

} _superblock_t;

typedef struct inode_bitmap {
    __le16 bitmap[ _SIZE_INODE_BITMAP_LE16 ];
} _inode_bitmap_t;

typedef struct data_bitmap {
    __le16 bitmap[ _SIZE_DATA_BITMAP_LE16 ];
} _data_bitmap_t;

typedef struct hash_list {
    unsigned char hash[ MD5_DIGEST_LENGTH ];
    struct hash_list* next;
} hash_list;



typedef struct inode {
	char 	i_file_name[128]; // bytes
    
    // pointers to data bitmap
    __le32 	i_block[ _EXT2_N_BLOCKS ];/* Pointers to blocks */  //참고: EXT2_N_BLOCKS = 15
    __le32 	i_uid;        /* Low 16 bits of Owner Uid */
    __le32 	i_gid;        /* Low 16 bits of Group Id */
    __le32 	i_size;        /* Size in bytes */
    __le32 	i_atime;    /* Access time */
    __le32 	i_ctime;    /* Creation time */
    __le32 	i_mtime;    /* Modification time */
    __le32 	i_blocks;    /* Blocks count */
    __le32 	i_links_count[ _EXT2_N_BLOCKS_DIRECT ];    /* Links count */
    
} _inode_t;

typedef struct data_region {
    
} _data_region_t;


// function : I/O file
_superblock_t create_superblock( FILE * dgfs_file );
_superblock_t read_superblock( FILE *dgfs_file);
_superblock_t modify_superblock( FILE *dgfs_file, _superblock_t _superblock_modified );

int create_inode_bitmap( FILE* dgfs_file, unsigned int offset );
int create_data_bitmap( FILE* dgfs_file, unsigned int offset );
int create_inode_region( FILE* dgfs_file, unsigned int offset );

_inode_t create_inode( FILE *dgfs_file, unsigned int offset, unsigned int inode_number );
_inode_t read_inode( FILE *dgfs_file, unsigned int offset );
_inode_t modify_inode(FILE *dgfs_file, unsigned int offset, _inode_t inode_modified );


// return inode number
int read_inode_bitmap( FILE *dgfs_file, _superblock_t _superblock);
// return data number
int read_data_bitmap(  FILE* dgfs_file, _superblock_t _superblock );



int write_inode_bitmap( FILE *dgfs_file, __le32 offset,  __le16 position, __le16 bitmap_data);
int write_data_bitmap( FILE *dgfs_file, __le32 offset,  __le16 position, __le16 bitmap_data);
_inode_t create_new_inode_w( char* filename, _superblock_t _superblock );


// Function : main
int DGFS_create(char* file);
int DGFS_add(char* dgfs_file, char* filename);
int DGFS_ls();
int DGFS_remove(char* filename);
int DGFS_extract(char* filename, char* output_filename);

// Function : util

int cal_inode_number(int offset_inode_bitmap_unit, int position );
int cal_data_number( int offset_data_bitmap_unit, int position );
int get_le_bit_from_byte( __le16 data ,int offset );

hash_list* hash_list_push(hash_list *cur, char* hash);
void hash_list_free(hash_list *head);

hash_list* make_file_hash( FILE *fp1, _inode_t inode );

_inode_t link_duplicated_hash( FILE* dgfs_file, _superblock_t _superblock, _inode_t new_inode, hash_list* head );
_inode_t check_dup(FILE *dgfs_file,_superblock_t _superblock, _inode_t inode, _inode_t new_inode,hash_list* head );


