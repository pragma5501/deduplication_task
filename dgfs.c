#include "dgfs.h"

/**
 * Create DGFS file with superblock, data/inode bitmap, and inode region. 
 * @param	file	Name of DGFS file to create
 * @return  Return 0 in case of success and return 1 in case of failure
 */
int is_name_dup = 0;

int DGFS_create(char* file)
{
	_superblock_t _superblock;
	struct stat stat_file;

	if( stat( file, &stat_file ) != -1 ) {
		FILE *fp2_dgfs = fopen( file, "rb+" );
		read_superblock(fp2_dgfs);
		fclose( fp2_dgfs );
		printf("%s already exist\n", file);
		return 1;
	}

	// create super block and write it into a file
	FILE *fp_dgfs = fopen( file, "wb+" );
	
	_superblock = create_superblock( 	 fp_dgfs );
	create_inode_bitmap( fp_dgfs, _superblock.offset_inode_bitmap );
	create_data_bitmap(  fp_dgfs, _superblock.offset_data_bitmap  );
	create_inode_region( fp_dgfs, _superblock.offset_inode_region );

	fclose( fp_dgfs );
	printf("DGFS create %s\n", file);
	
	return 0;
}
//done
_superblock_t create_superblock( FILE * dgfs_file ) {
	_superblock_t _superblock;
	unsigned int offset_cur = 0;

	_superblock.offset_superblock = offset_cur;
	offset_cur += _SIZE_SUPERBLOCK;

	_superblock.offset_inode_bitmap = offset_cur;
	offset_cur += _SIZE_INODE_BITMAP;

	_superblock.offset_data_bitmap  = offset_cur;
	offset_cur += _SIZE_DATA_BITMAP;

	_superblock.offset_inode_region = offset_cur;
	offset_cur += _SIZE_INODE_REGION;

	_superblock.offset_data_region  = offset_cur;
	offset_cur += _SIZE_DATA_REGION;

	
	_superblock.total_size = 0;
	_superblock.total_inodes = 0;

	
	fseek( dgfs_file, 0, SEEK_SET );
	fwrite( &_superblock, sizeof(_superblock_t), 1, dgfs_file );

	return _superblock;
}
// done
_superblock_t read_superblock( FILE *dgfs_file ) {
	_superblock_t _superblock;
	
	fseek(dgfs_file,0, SEEK_SET);
	fread( &_superblock, sizeof( _superblock_t ), 1, dgfs_file );

	return _superblock;
}
_superblock_t modify_superblock( FILE *dgfs_file, _superblock_t _superblock_modified ) {
	fseek(dgfs_file,0, SEEK_SET);
	fwrite( &_superblock_modified, sizeof(_superblock_t), 1, dgfs_file );

	return _superblock_modified;
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
		read_inode(   dgfs_file, offset + i * _SIZE_INODE );
	}
	return 0;
}
// done
_inode_t create_inode( FILE *dgfs_file, unsigned int offset, unsigned int inode_number ) {
	fseek( dgfs_file, offset, SEEK_SET );
	
	_inode_t inode;
	// filename = DUMMY VALUE

	// init block numbers
	/*
	for(int i = 0; i <= BLOCK_DIRECT; i++) {
		inode.i_block[ i ] = inode_number * (BLOCK_DIRECT + 1) + i;
	}
	*/
	fwrite( &inode, sizeof(inode), 1, dgfs_file );

	return inode;
}
_inode_t read_inode( FILE *dgfs_file, unsigned int offset ) {
	fseek( dgfs_file, offset, SEEK_SET );

	_inode_t inode;

	fread( &inode, sizeof(inode), 1, dgfs_file );
	/*
	for(int i = 0; i <= BLOCK_DIRECT; i++) {
		printf("%d ",inode.i_block[ i ] );
	}
	printf("\n");
	*/
	return inode;
}

_inode_t modify_inode(FILE *dgfs_file, unsigned int offset, _inode_t inode_modified ) {
	fseek( dgfs_file, offset, SEEK_SET );
	fwrite( &inode_modified, sizeof(_inode_t), 1, dgfs_file );
}

// return data number
int read_data_bitmap( FILE* dgfs_file, _superblock_t _superblock ) {
	unsigned int offset_data_bitmap = _superblock.offset_data_bitmap;

	fseek( dgfs_file, offset_data_bitmap, SEEK_SET );


	__le16 data_bitmap[ _SIZE_DATA_BITMAP_LE16 ];
	fread( &data_bitmap, sizeof( __le16 ), _SIZE_DATA_BITMAP_LE16, dgfs_file );
	
    for(int i = 0; i < _SIZE_DATA_BITMAP_LE16; i++ ) {
        for( int j = 0; j < _SIZE_DATA_BITMAP_UNIT * 8; j++ ) {
            
            // check data is used, if so continue
            if( get_le_bit_from_byte( data_bitmap[i], j ) == 1 ) continue;

			write_data_bitmap( dgfs_file, offset_data_bitmap + i * _SIZE_DATA_BITMAP_UNIT * 8, j, data_bitmap[i] );

			// data is unwritten
            unsigned int data_number = cal_data_number(i, j) + 1;

			return data_number;
            
			//unsigned int offset_data = _superblock.offset_data_region + data_number * _SIZE_DATA;
            //_inode_t inode = read_inode( dgfs_file, offset_inode, inode_number );
        }
    }

	// data bitmap is full
	printf("Error : DATA BITMAP IS FULL");
	return -1;
}
int read_data_bitmap_written(  FILE*dgfs_file, _superblock_t _superblock ) {
	unsigned int offset_data_bitmap = _superblock.offset_data_bitmap;
	fseek( dgfs_file, offset_data_bitmap, SEEK_SET );

	__le16 data_bitmap[ _SIZE_DATA_BITMAP_LE16 ];
	fread( &data_bitmap, sizeof( __le16 ), _SIZE_DATA_BITMAP_LE16, dgfs_file );

	int allocated_blocks = 0;
	// read available inode bit
	for( int i = 0; i < _SIZE_DATA_BITMAP_LE16; i++ ) {
		for( int j = 0; j < _SIZE_DATA_BITMAP_UNIT * 8; j++ ) {
			
			// if the bit of the inode bitmap is 0, the inode numbrr is unwritten
			if( get_le_bit_from_byte( data_bitmap[i], j ) == 1 ) allocated_blocks++;

		}
	}
	printf("Allocated blocks: %d\n", allocated_blocks);
	return allocated_blocks;
}
int read_inode_bitmap_written(  FILE*dgfs_file, _superblock_t _superblock ) {
	unsigned int offset_inode_bitmap = _superblock.offset_inode_bitmap;
	fseek( dgfs_file, offset_inode_bitmap, SEEK_SET );

	__le16 inode_bitmap[ _SIZE_INODE_BITMAP_LE16 ];
	fread( &inode_bitmap, sizeof( __le16 ), _SIZE_INODE_BITMAP_LE16, dgfs_file );

	__le16 map_data_bit[ _SIZE_MAP_DATA_BIT ] = {0};

	int count_inode = 0;
	int total_size = 0;
	int allocated_blocks = 0;
	// read available inode bit
	for( int i = 0; i < _SIZE_INODE_BITMAP_LE16; i++ ) {
		for( int j = 0; j < _SIZE_INODE_BITMAP_UNIT * 8; j++ ) {
			
			// if the bit of the inode bitmap is 0, the inode numbrr is unwritten
			if( get_le_bit_from_byte( inode_bitmap[i], j ) == 0 ) continue;
			unsigned int inode_number = i * _SIZE_INODE_BITMAP_UNIT * 8 + j + 1;
			_inode_t inode = read_inode( dgfs_file, _superblock.offset_inode_region + inode_number * _SIZE_INODE );
			total_size += inode.i_size;
			
			printf("%-32s\t%ld\n", inode.i_file_name, inode.i_size);

			// count allocated_block
			for(int x = 0; x < _EXT2_N_BLOCKS_DIRECT; x++) {
				printf("%d ", inode.i_block[x] );
				if( (map_data_bit[ inode.i_block[x] ] == 1) || inode.i_block[x] == 0) continue;
				map_data_bit[ inode.i_block[x] ] = 1;
				allocated_blocks += 1;
			}
			printf("\n");

			count_inode ++;
			// every inode detected
			if( count_inode >= _superblock.total_inodes ) {
				printf("Total size: %d\n", total_size);
				printf("Allocated blocks: %d\n", allocated_blocks);
				return 0; 
			}
		}
	}

}


// return inode number
int read_inode_bitmap( FILE *dgfs_file, _superblock_t _superblock ) {
	// find the start point of the inode bitmap of the file
	unsigned int offset_inode_bitmap = _superblock.offset_inode_bitmap;
	fseek( dgfs_file, offset_inode_bitmap, SEEK_SET );

	__le16 inode_bitmap[ _SIZE_INODE_BITMAP_LE16 ];
	fread( &inode_bitmap, sizeof( __le16 ), _SIZE_INODE_BITMAP_LE16, dgfs_file );

	// read available inode bit 
	
	for( int i = 0; i < _SIZE_INODE_BITMAP_LE16; i++ ) {
		for( int j = 0; j < _SIZE_INODE_BITMAP_UNIT * 8; j++ ) {
			
			// if the bit of the inode bitmap is 0, the inode numbrr is available
			if( get_le_bit_from_byte( inode_bitmap[i], j ) == 1 ) continue;

			// modify inode bitmap : unused -> used;
			write_inode_bitmap( dgfs_file, offset_inode_bitmap + i * _SIZE_INODE_BITMAP_UNIT * 8, j, inode_bitmap[i] );
			//printf("bitmap : %d\n", inode_bitmap[i]);

			_superblock.total_inodes += 1;
			modify_superblock( dgfs_file, _superblock);

			return i * _SIZE_INODE_BITMAP_UNIT * 8 + j;

		}
	}

	printf("Error : INODE BITMAP IS FULL");
	return -1;
}

int write_inode_bitmap( FILE *dgfs_file, __le32 offset,  __le16 position, __le16 bitmap_data) {
	fseek( dgfs_file, offset, SEEK_SET );


	bitmap_data += (  (__le16)1 << position );
	
	fwrite( &bitmap_data, sizeof( bitmap_data ), 1, dgfs_file );
	return 0;
}
int remove_inode_bitmap( FILE *dgfs_file, __le32 offset,  __le16 position, __le16 bitmap_data) {
	fseek( dgfs_file, offset, SEEK_SET );

	bitmap_data -= (  (__le16)1 << position );

	fwrite( &bitmap_data, sizeof( bitmap_data ), 1, dgfs_file );
	return 0;
}


_inode_t create_new_inode_w( char* filename, _superblock_t _superblock ) {
	_inode_t inode;
	struct stat stat_file;
	if( stat( filename, &stat_file ) == -1 ) {
		return inode;
	}

	
	strcpy( inode.i_file_name, filename );

	__le32 block[ _EXT2_N_BLOCKS ] = {0};
	memcpy( inode.i_block, block, sizeof(__le32)* _EXT2_N_BLOCKS );

	inode.i_uid = stat_file.st_uid;
	inode.i_gid = stat_file.st_gid;
	inode.i_size = stat_file.st_size;
	inode.i_atime = stat_file.st_atime;
	inode.i_ctime = stat_file.st_ctime;
	inode.i_mtime = stat_file.st_mtime;
	//inode.i_blocks = stat_file.st_blocks;
	memcpy( inode.i_links_count, block, sizeof(__le32)*_EXT2_N_BLOCKS_DIRECT );



	return inode;
	// insert file name into inode
}

int write_data_bitmap( FILE *dgfs_file, __le32 offset,  __le16 position, __le16 bitmap_data) {
	fseek( dgfs_file, offset, SEEK_SET );
	
	bitmap_data += (  (__le16)1 << position );

	fwrite( &bitmap_data, sizeof( bitmap_data ), 1, dgfs_file );
	return 0;
}

_inode_t write_data( FILE* dgfs_file, _superblock_t _superblock, _inode_t inode, hash_list* head) {
	unsigned int offset_data_bitmap = _superblock.offset_data_bitmap;
	unsigned int inode_number = read_inode_bitmap( dgfs_file, _superblock ) + 1;
	
	int count_duplicated = 0;
	
	hash_list* cur = head->next;
	for(int i = 0; i <= BLOCK_DIRECT; i++) {
		
		if( cur == NULL ) {
			break;
		} 

		// already link
		if( inode.i_block[i] != 0 ) {
			count_duplicated ++;
			cur = cur->next;
			continue;
		}
		int temp_data_number = find_dup_hash(head, cur);

		if( temp_data_number != -1 ) {
			count_duplicated ++;
			inode.i_block[i] = temp_data_number;
			cur = cur->next;
			continue;
		}
		// write data
		__le32 data_number = read_data_bitmap( dgfs_file, _superblock ) + 1;
		
		inode.i_block[i] = data_number;
		unsigned int offset_data = _superblock.offset_data_region + data_number * _SIZE_DATA;
		

		fseek( dgfs_file, offset_data, SEEK_SET );
		fwrite( cur->hash, sizeof(unsigned char), MD5_DIGEST_LENGTH, dgfs_file );

		//unsigned char temp[MD5_DIGEST_LENGTH];
		//fseek( dgfs_file, offset_data, SEEK_SET );
		//fread( temp, sizeof(unsigned char), MD5_DIGEST_LENGTH, dgfs_file );
		//printf("%s\n",temp); 
		cur->data_number = data_number;
		cur = cur->next;
	}


	unsigned int offset_inode = _superblock.offset_inode_region + inode_number * _SIZE_INODE;
	fseek( dgfs_file, offset_inode, SEEK_SET );
	fwrite( &inode, sizeof( inode ), 1, dgfs_file );

	printf("%d block(s) deduplicated\n", count_duplicated);

	return inode;
}

int read_inode_bitmap_removing(  FILE*dgfs_file, _superblock_t _superblock, char* filename ) {
	
	unsigned int offset_inode_bitmap = _superblock.offset_inode_bitmap;
	fseek( dgfs_file, offset_inode_bitmap, SEEK_SET );

	__le16 inode_bitmap[ _SIZE_INODE_BITMAP_LE16 ];
	fread( &inode_bitmap, sizeof( __le16 ), _SIZE_INODE_BITMAP_LE16, dgfs_file );

	int count_inode = 0;

	// read available inode bit
	for( int i = 0; i < _SIZE_INODE_BITMAP_LE16; i++ ) {
		for( int j = 0; j < _SIZE_INODE_BITMAP_UNIT * 8; j++ ) {
			//printf("%d, %d : i, j : bit(%d)\n", i, j, get_le_bit_from_byte( inode_bitmap[i], j ) );
			// if the bit of the inode bitmap is 0, the inode numbrr is unwritten
			if( get_le_bit_from_byte( inode_bitmap[i], j ) == 0 ) continue;
			
			unsigned int inode_number = i * _SIZE_INODE_BITMAP_UNIT * 8 + j + 1;
			_inode_t inode = read_inode( dgfs_file, _superblock.offset_inode_region + inode_number * _SIZE_INODE );
			printf("filename : %-32s\n", inode.i_file_name);
			// if find filename
			if( strncmp( filename, inode.i_file_name, strlen(filename) ) == 0) {
				remove_inode_bitmap( dgfs_file, offset_inode_bitmap + i * _SIZE_DATA_BITMAP_UNIT * 8, j, inode_bitmap[i]);
				
				_superblock.total_inodes -= 1;
				
				modify_superblock( dgfs_file, _superblock );
				return 0;
			}

			count_inode ++;
			// every inode detected, but not found
			if( count_inode >= _superblock.total_inodes ) {
				
				return 1; 
			}
		}
	}
	return 1;
}

/**
 * Add file in DGFS. If there is block with same contents in DGFS,
 * DGFS will deduplicate this block and print the number of deduplicated blocks.
 * @param	filename	Name of file to add in DGFS
 * @return  Return 0 in case of success and return 1 in case of failure
 */
int DGFS_add(char* dgfs_file, char* filename)
{
	FILE *fp_dgfs = fopen( dgfs_file, "rb+" );
	FILE *fp_file = fopen( filename,  "rb+" );

	
	//make_file_hash( fp_file, result_file_hash );

	int file_hashed_size = MD5_DIGEST_LENGTH;

	_superblock_t _superblock;
	_superblock = read_superblock( fp_dgfs );

	_inode_t inode;
	inode = create_new_inode_w( filename, _superblock );


	hash_list* head = make_file_hash( fp_file, inode );

	inode = link_duplicated_hash( fp_dgfs, _superblock, inode, head );
	if( is_name_dup == 0 ) {
		inode = write_data( fp_dgfs, _superblock, inode, head  );
	} else {
		printf("Duplicate file name!\n");
	}
	

	hash_list_free( head );

	fclose(fp_dgfs);
	fclose(fp_file);

	printf("DGFS add: %s\n", filename);
	//printf("%d block(s) deduplicated\n", deduplicated_block_count);
	return 0;
}

/**
 * Print list of files in DGFS. After printing list of files, 
 * print total size of files and number of data blocks in use.
 * @return  Return 0 in case of success and return 1 in case of failure
 */
int DGFS_ls(char* filename)
{
	FILE *fp_dgfs = fopen( filename, "rb");

	// file does not exist
	if( fp_dgfs == NULL ) return 1;

	_superblock_t _superblock = read_superblock( fp_dgfs );
	
	read_inode_bitmap_written( fp_dgfs, _superblock );
	//read_data_bitmap_written(  fp_dgfs, _superblock );
	
	fclose( fp_dgfs);
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
int DGFS_remove(char* dgfs_filename,char* filename)
{

	FILE *fp_dgfs = fopen( dgfs_filename, "rb+" );
	if( fp_dgfs == NULL ) {
		return 1;
	}
	_superblock_t _superblock = read_superblock( fp_dgfs );

	int is_remove = read_inode_bitmap_removing( fp_dgfs, _superblock, filename);
	
	// remove success
	if( is_remove == 0 ) {
		printf("DGFS remove : %s\n", filename);
	}
	if( is_remove == 1 ) {
		printf("%s not found!\n", filename);
	}

	fclose( fp_dgfs );
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
		DGFS_ls(argv[1]);
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
		DGFS_remove(argv[1],argv[3]);
		return 0;
	}
	else {
		printf("Usage: ./DGFS create DGFS_file or DGFS_file (add/extract/remove/ls)\n");
		return 0;
	}

	return 0;
}

hash_list* hash_list_push(hash_list *cur, char* hash) {
    hash_list* new_hash_node = (hash_list*)malloc( sizeof(hash_list) );

    strncpy( new_hash_node->hash, hash,  MD5_DIGEST_LENGTH ); 
    new_hash_node->next = NULL;
	new_hash_node->data_number = -1;
    cur->next = new_hash_node;
	
    return new_hash_node;
}
void hash_list_free(hash_list *head) {
    
    while( head->next != NULL ) {
        hash_list* cur = head->next;
        head->next = cur->next;
        free( cur );
    };
	free(head);
}
int find_dup_hash( hash_list* head, hash_list* cur ) {
	hash_list* temp = head->next;
	
	while( temp != NULL ) {
		if( temp == cur ) return -1;
		if( strncmp(temp->hash, cur->hash, MD5_DIGEST_LENGTH) == 0 ) {
			
			cur->data_number = temp->data_number;
			return cur->data_number;
		}
		temp = temp->next;
	}
	return -1;
}

hash_list* make_file_hash( FILE *fp1, _inode_t inode ) {

    // head hash value = NULL
    hash_list *head = (hash_list*)malloc(sizeof(hash_list));
    head->next = NULL;
	head->data_number = -1;
    hash_list *cur = head;

    MD5_CTX mdContext;
    unsigned char _hash[ MD5_DIGEST_LENGTH ];
    unsigned char data[1024]; // 1024 byte = 1 KB
    int bytes;

    // make md5 hash
    // ==================
    int i = -1;
    while( ( bytes = fread( data, 1, 1024, fp1 ) ) != 0 ) {
		i++;
        if( i % 4 == 0 ) MD5_Init( &mdContext );

        MD5_Update( &mdContext, data, bytes );
        if( i % 4 == 3 ) {
            MD5_Final( _hash ,&mdContext );
            cur = hash_list_push(cur, _hash); 
			
        }
        
    }

    // Before read 4KB, i $ 4 != 3
    if( i % 4 != 3 ) {
        MD5_Final( _hash ,&mdContext );
        cur = hash_list_push(cur, _hash); 
    }

    return head;

}
_inode_t link_duplicated_hash( FILE* dgfs_file, _superblock_t _superblock, _inode_t new_inode, hash_list* head ) {
    // inode bitmap search
    __le32 count = 0;
	unsigned int offset_inode_bitmap = _superblock.offset_inode_bitmap;
	fseek( dgfs_file, offset_inode_bitmap, SEEK_SET );

	__le16 inode_bitmap[ _SIZE_INODE_BITMAP_LE16 ];
	fread( &inode_bitmap, sizeof( __le16 ), _SIZE_INODE_BITMAP_LE16, dgfs_file );

    for(int i = 0; i < _SIZE_INODE_BITMAP_LE16; i++ ) {
        for( int j = 0; j < _SIZE_INODE_BITMAP_UNIT * 8; j++ ) {
            
            // check inode is unused, if so continue
            if( get_le_bit_from_byte( inode_bitmap[i], j ) == 0 ) continue;

            unsigned int inode_number = cal_inode_number(i, j) + 1;
            unsigned int offset_inode = _superblock.offset_inode_region + inode_number * _SIZE_INODE;
            
            _inode_t inode = read_inode( dgfs_file, offset_inode );
           
            // check duplication
            new_inode = check_dup( dgfs_file ,_superblock, inode, new_inode, head );
			if( is_name_dup == 1 ) {
				return new_inode;
			}
            // if duplicated
            //modify_inode( dgfs_file, offset_inode, inode );

            count++;
            if(count >= _superblock.total_inodes ) return new_inode;
        }
    }
    return new_inode;
}
// inode is origin inode
_inode_t check_dup(FILE *dgfs_file,_superblock_t _superblock, _inode_t inode, _inode_t new_inode,hash_list* head ) {
    if( !strcmp(inode.i_file_name, new_inode.i_file_name ) ) {
		is_name_dup = 1;
		return new_inode;
	}
	
	hash_list *cur = head;

    for(int i = 0; i < _EXT2_N_BLOCKS; i++) {
        cur = cur->next;
        __le32 data_number = inode.i_block[i] + 1;

        //__le16 data_bit = get_le_bit_from_byte( (int)(data_number / sizeof(__le16) / 8 ), data_number % ( sizeof(_le16) *  8 ) );

        // already duplicated
        if( new_inode.i_block[i] != 0 ) {
            continue;
        }
        if( cur == NULL ) {
            break;
        }

        // then comparison between origin and new hash.
        unsigned int offset_data = _superblock.offset_data_region + data_number * _SIZE_DATA;
        
        // origin_hash
        unsigned char _hash[ MD5_DIGEST_LENGTH ];
        
        fseek( dgfs_file, offset_data, SEEK_SET );
        fread( _hash, sizeof(unsigned char), MD5_DIGEST_LENGTH, dgfs_file );

        // if not duplicated
        if( strncmp(cur->hash, _hash, MD5_DIGEST_LENGTH ) != 0 ) {
            continue;
        }

        // if duplicated
        new_inode.i_block[i] = data_number;
        
    }
    // dup is true
    return new_inode;
}

int cal_inode_number( int offset_inode_bitmap_unit, int position ) {
    return offset_inode_bitmap_unit * _SIZE_INODE_BITMAP_UNIT * 8 + position;
}
int cal_data_number( int offset_data_bitmap_unit, int position ) {
	return offset_data_bitmap_unit * _SIZE_DATA_BITMAP_UNIT * 8 + position;
}

int get_le_bit_from_byte( __le16 data ,int offset ) {
	__le16 bit = ( data << ( sizeof(__le16)*8 - offset - 1) );
    return bit >> ( sizeof(__le16)*8 -1  );
}