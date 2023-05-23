#include "dgfs.h"

/**
 * Create DGFS file with superblock, data/inode bitmap, and inode region. 
 * @param	file	Name of DGFS file to create
 * @return  Return 0 in case of success and return 1 in case of failure
 */
 

int DGFS_create(char* file)
{
	struct stat stat_file;
	_superblock_t *_superblock;

	if( stat( file, &stat_file ) != -1 ) {
		FILE *rfp_dgfs = fopen( file, "rb" );
		read_superblock( rfp_dgfs, _superblock );
		fclose( rfp_dgfs );
		printf("%s already exist\n", file);
		return 1;
	}

	// create super block and write it into a file
	FILE *fp_dgfs = fopen( file, "wb" );
	create_superblock( 	 fp_dgfs, _superblock );
	create_inode_bitmap( fp_dgfs, _superblock->offset_inode_bitmap );
	create_data_bitmap(  fp_dgfs, _superblock->offset_data_bitmap  );
	create_inode_region( fp_dgfs, _superblock->offset_inode_region );

	fclose( fp_dgfs );
	printf("DGFS create %s\n", file);
	
	return 0;
}
//done
int create_superblock( FILE * dgfs_file, _superblock_t *_superblock ) {
	unsigned int offset_cur = 0;

	_superblock->offset_superblock = offset_cur;
	offset_cur += _SIZE_SUPERBLOCK;

	_superblock->offset_inode_bitmap = offset_cur;
	offset_cur += _SIZE_INODE_BITMAP;

	_superblock->offset_data_bitmap  = offset_cur;
	offset_cur += _SIZE_DATA_BITMAP;

	_superblock->offset_inode_region = offset_cur;
	offset_cur += _SIZE_INODE_REGION;

	_superblock->offset_data_region  = offset_cur;
	offset_cur += _SIZE_DATA_REGION;

	_superblock->num_inodes = 0;
	_superblock->total_size = 0;
	_superblock->offset_inode_bitmap_last;

	fwrite( &_superblock, sizeof(_superblock), 1, dgfs_file );
}
// done
int read_superblock( FILE *dgfs_file, _superblock_t* _superblock) {
	fseek( dgfs_file, 0, SEEK_SET );

	fread( _superblock, sizeof( _superblock_t ), 1, dgfs_file );
	
	for( int i = 0; i < _SIZE_SUPERBLOCK_UNSIGNED_INT_UNIT; i++ ) {
		// reading superblock test
		printf("superblock data : %d\n", *(&(_superblock->offset_superblock ) + i) );
	}
}
// done
int create_inode_bitmap( FILE* dgfs_file, unsigned int offset ) {

	__le16 bitmap[ _SIZE_INODE_BITMAP_LE16 ] = {0};

	fseek( dgfs_file, offset, SEEK_SET);
	fwrite( bitmap, sizeof( __le16 ), _SIZE_INODE_BITMAP_LE16, dgfs_file );

	return 0;
}
// done
int create_data_bitmap( FILE* dgfs_file, unsigned int offset ) {

	__le16 bitmap[ _SIZE_DATA_BITMAP_LE16 ] = {0};

	fseek( dgfs_file, offset, SEEK_SET);
	fwrite( bitmap, sizeof( __le16 ), _SIZE_DATA_BITMAP_LE16, dgfs_file );
	return 0;
}
// done
int create_inode_region( FILE* dgfs_file, unsigned int offset ) {
	for(unsigned int i = 0; i < _SIZE_INODE_REGION / _SIZE_INODE; i++) {
		create_inode( dgfs_file, offset + i * _SIZE_INODE, i );
	}
	return 0;
}

// return inode number
int read_inode_bitmap( FILE *dgfs_file, _superblock_t* _superblock) {
	// find the start point of the inode bitmap of the file
	unsigned int offset_inode_bitmap = _superblock->offset_inode_bitmap;
	fseek( dgfs_file, offset_inode_bitmap, SEEK_SET );

	__le16 inode_bitmap[ _SIZE_INODE_BITMAP_LE16 ];
	fread( &inode_bitmap, sizeof( __le16 ), _SIZE_INODE_BITMAP_LE16, dgfs_file );

	// read available inode bit 
	int i = _superblock->offset_inode_bitmap_last;
	for( i; i < _SIZE_INODE_BITMAP_LE16; i++ ) {
		for( int j = 0; j < _SIZE_INODE_BITMAP_UNIT * 8; j++ ) {
			
			// if the bit of the inode bitmap is 0, the inode numbrr is available
			if( get_le_bit_from_byte( inode_bitmap[i], j ) == 0 ) {
				printf("bitmap : %d\n", inode_bitmap[i]);
				_superblock->offset_inode_bitmap_last = i;
				return i * _SIZE_INODE_BITMAP_UNIT * 8 + j;
			}

		}
	}

	i = _superblock->offset_inode_bitmap_last - 1;
	for( i; i > 0; i-- ) {
		for( int j = 0; j < _SIZE_INODE_BITMAP_UNIT * 8; j++ ) {

			// if the bit of the inode bitmap is 0, the inode numbrr is available
			if( get_le_bit_from_byte( inode_bitmap[i], j ) == 0 ) {
				write_inode_bitmap( dgfs_file, offset_inode_bitmap +(i-1) * _SIZE_INODE_BITMAP_UNIT * 8, j, inode_bitmap[i] );
				_superblock->offset_inode_bitmap_last = i;
				return (i-1) * _SIZE_INODE_BITMAP_UNIT * 8 + j;
			}

		}
	}
	printf("Error : INODE BITMAP FULL");
	return -1;
}

int write_inode_bitmap( FILE *dgfs_file, __le32 offset,  __le16 position, __le16 bitmap_data, __le16 flag) {
	fseek( dgfs_file, offset, SEEK_SET );
	if( flag == MODE_BITMAP_WRITE_PLUS ) {
		bitmap_data += (  (__le16)1 << position );
	}
	if( flag == MODE_BITMAP_WRITE_MINUS ) {
		bitmap_data -= (  (__le16)1 << position );
	}

	fwrite( &bitmap_data, sizeof( bitmap_data ), 1, dgfs_file );
	return 0;
}

// done
int create_inode( FILE *dgfs_file, unsigned int offset, unsigned int inode_number ) {
	fseek( dgfs_file, offset, SEEK_SET );
	
	_inode_t inode;
	// filename = DUMMY VALUE

	// init block numbers
	for(int i = 0; i <= BLOCK_DIRECT; i++) {
		inode.i_block[ i ] = inode_number * (BLOCK_DIRECT + 1) + i;
	}
	fwrite( &inode, sizeof(inode), 1, dgfs_file );
}

int write_new_inode( FILE *dgfs_file, FILE *file, char* filename, int file_hashed_size, _superblock_t* _superblock ) {

	unsigned int offset_inode_new = read_inode_bitmap( dgfs_file, _superblock );

	struct stat stat_file;
	if( stat( filename, &stat_file ) == -1 ) {
		return 1;
	}
	_inode_t inode;

	strcpy( inode.i_file_name, filename );
	
	inode.i_uid = stat_file.st_uid;
	inode.i_gid = stat_file.st_gid;
	inode.i_size = stat_file.st_size;
	inode.i_atime = stat_file.st_atime;
	inode.i_ctime = stat_file.st_ctime;
	inode.i_mtime = stat_file.st_mtime;
	inode.i_blocks = stat_file.st_blocks;
	inode.i_links_count = 1;
	

	fwrite( &inode, sizeof( __le32 ), _SIZE_INODE_LE16_UNIT, dgfs_file );
	// insert file name into inode
}


int read_inode( FILE *dgfs_file, FILE *file, __le32 inode_number ) {
	unsigned int offset 
}


/**
 * Add file in DGFS. If there is block with same contents in DGFS,
 * DGFS will deduplicate this block and print the number of deduplicated blocks.
 * @param	filename	Name of file to add in DGFS
 * @return  Return 0 in case of success and return 1 in case of failure
 */
int DGFS_add(char* dgfs_file, char* filename)
{
	FILE *fp_dgfs = fopen( dgfs_file, "rb" );
	FILE *fp_file = fopen( filename,  "rb" );

	unsigned char result_file_hash[ MD5_DIGEST_LENGTH ];
	make_file_hash( fp_file, result_file_hash );

	int file_hashed_size = MD5_DIGEST_LENGTH;

	_superblock_t *_superblock;
	read_superblock( fp_dgfs, _superblock );

	write_new_inode( fp_dgfs, fp_file, filename, file_hashed_size, _superblock );

	printf("DGFS add: %s\n", filename);
	printf("Duplicate file name!\n");
	//printf("%d block(s) deduplicated\n", deduplicated_block_count);
	return 0;
}

/**
 * Print list of files in DGFS. After printing list of files, 
 * print total size of files and number of data blocks in use.
 * @return  Return 0 in case of success and return 1 in case of failure
 */
int DGFS_ls()
{
	printf("%-32s\tSize\n", "Filename"); // print information layout
	//printf("%-32s\t%ld\n", name, size);
	//printf("Total size: %lld\n", total_size);
	//printf("Allocated blocks: %d\n", block_count);
	return 0;
}

/**
 * Remove file with filename in DGFS.
 * @param	filename	Name of file to remove in DGFS
 * @return  Return 0 in case of success and return 1 in case of failure
 */
int DGFS_remove(char* filename)
{
	//printf("DGFS remove: %s\n", filename);
	//printf("%s not found!\n", filename);
	return 0;
}

/**
 * Extract contents of file in DGFS and save them outside of DGFS system
 * @param	filename		Name of file in DGFS to extract
 * @param	output_filename	Name of file to be extracted outside of DGFS
 * @return  Return 0 in case of success and return 1 in case of failure
 */
int DGFS_extract(char* filename, char* output_filename)
{
	//printf("DGFS extract: %s into %s\n", filename, output_filename);
	//printf("%s not exist!\n", filename);
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc == 1) {
		printf("Usage: ./DGFS create DGFS_file or DGFS_file (add/extract/remove/ls)\n");
		return 0;
	}
	if (strcmp(argv[1], "create") == 0) {
		if (argc != 3) {
			printf("Usage: ./DGFS create filename\n");
			return 0;
		}
		DGFS_create(argv[2]);
		return 0;
	}

	if (strcmp(argv[2], "add") == 0) {
		if (argc != 4) {
			printf("Usage: ./DGFS DGFS_file add filename\n");
			return 0;
		}
		DGFS_add(argv[1], argv[3]);
		return 0;
	}
	else if (strcmp(argv[2], "ls") == 0) {
		if (argc != 3) {
			printf("Usage: ./DGFS DGFS_file ls\n");
			return 0;
		}
		DGFS_ls();
		return 0;
	}
	else if (strcmp(argv[2], "extract") == 0) {
		if (argc != 5) {
			printf("Usage: ./DGFS DGFS_file extract in_file out_file\n");
			return 0;
		}
		DGFS_extract(argv[3], argv[4]);
		return 0;
	}
	else if (strcmp(argv[2], "remove") == 0) {
		if (argc != 4) {
			printf("Usage: ./DGFS DGFS_file remove filename\n");
			return 0;
		}
		DGFS_remove(argv[3]);
		return 0;
	}
	else {
		printf("Usage: ./DGFS create DGFS_file or DGFS_file (add/extract/remove/ls)\n");
		return 0;
	}

	return 0;
}
