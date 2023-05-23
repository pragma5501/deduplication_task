#include <stdio.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define BUF_SIZE 100
/**
 * Compare contents of file_1 and file_2 and make hard link between two files
 * if the contents of two files are indentical
 * @param file_1    Name of file
 * @param file_2    Name of file
 * @param hash      Indicates methods of file contents comparision
 * @return  Return 0 in case of success and return 1 in case of failure
 */

void print_hash( unsigned char* hash1, unsigned char* hash2, int size, char* file_1, char* file_2) {
    for( int i = 0; i < size; i++ ) {
        printf( "%02x", hash1[i] );
    }
    printf(" %s\n", file_1);

    for( int i = 0; i < size; i++ ) {
        printf( "%02x", hash2[i] );
    }
    printf(" %s\n", file_2);
}

int is_dup_bin_ver(FILE* fp1, FILE* fp2) {
    char buffer_bin1[ BUF_SIZE ], buffer_bin2[ BUF_SIZE ];
    char *line1, *line2;

    while( 1 ) {
        line1 = fgets( buffer_bin1, BUF_SIZE, fp1 );
        line2 = fgets( buffer_bin2, BUF_SIZE, fp2 );
        
        // is not dup
        if( strcmp( buffer_bin1, buffer_bin2 ) != 0 ) {
            return 0;
        }

        if( line1 == NULL || line2 == NULL ) return 1;
    }
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

int is_dup_hash_ver(FILE* fp1, FILE* fp2, char* file_1, char* file_2) {

    unsigned char md5_result1[ MD5_DIGEST_LENGTH ], md5_result2[ MD5_DIGEST_LENGTH ];

    MD5_CTX mdContext;
    unsigned char data[1024];
    int bytes;

    // make md5 hash
    // ==================
    MD5_Init( &mdContext );
    while( ( bytes = fread( data, 1, 1024, fp1 ) ) != 0 ) {
        MD5_Update( &mdContext, data, bytes );
    }
    MD5_Final( md5_result1 ,&mdContext );

    MD5_Init( &mdContext );
    while( ( bytes = fread( data, 1, 1024, fp2 ) ) != 0 ) {
        MD5_Update( &mdContext, data, bytes );
    }
    MD5_Final( md5_result2 ,&mdContext );
    //====================

    // is dup
    if( strncmp( md5_result1, md5_result2, MD5_DIGEST_LENGTH ) == 0 ) {
        return 1;
    }

    print_hash( md5_result1, md5_result2, MD5_DIGEST_LENGTH, file_1, file_2 );
    return 0;
}


int file_dedup(char* file_1, char* file_2, int hash)
{
    struct stat stat_file;

    FILE* f1 = fopen(file_1, "rb");
    FILE* f2 = fopen(file_2, "rb");

    if( stat(file_1, &stat_file ) == -1 ) {
        printf("diff: %s: No such file or directory\n", file_1);
        exit(1);
    }
    if( stat(file_2, &stat_file ) == -1 ) {
        printf("diff: %s: No such file or directory\n", file_2);
        exit(1);
    }

    int flag_dup;

    if( hash == 0 ) {
        flag_dup = is_dup_bin_ver( f1, f2 );
    }

    if( hash == 1 ) {
        flag_dup = is_dup_hash_ver( f1, f2, file_1, file_2 );
    }
    fclose( f1 );
    fclose( f2 );

    if( flag_dup == 1 ) {
        if( unlink( file_2 ) != 0 ) {
            printf("Removing a duplicated file is failed");
        }
        if( link( file_1, file_2 ) != 0 ) {
            printf("Creating hard link is failed\n");
        }
    }
    if( flag_dup == 0 ) {
        printf( "Binary file %s and %s differ \n", file_1, file_2 );
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int result;

    if (argc != 4) {
        printf("Usage: ./file_dedup hash/byte file_1 file_2\n");
        return 0;
    }


    if (strcmp(argv[1], "byte") == 0) {
        file_dedup(argv[2], argv[3], 0);
    }
    else if (strcmp(argv[1], "hash") == 0) {
        file_dedup(argv[2], argv[3], 1);
    }

    return 0;
}
