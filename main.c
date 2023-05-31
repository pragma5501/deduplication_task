#include <stdio.h>
#include <linux/kernel.h>
#include <string.h>
#include <stdlib.h>

#define _SIZE_DATA_BITMAP_LE16      64 * 1024

typedef struct data_bitmap {
    __le16 bitmap[ _SIZE_DATA_BITMAP_LE16 ];
} _data_bitmap_t;


int main(void) {

    printf("%d\n", (int)sizeof(_data_bitmap_t));
    return 0;
}

