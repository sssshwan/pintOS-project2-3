#include "vm/swap.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "devices/block.h"
#include <bitmap.h>
#include <stddef.h>
#include <stdio.h>

static struct bitmap *swap_bitmap; // memory
static struct block *swap_partition; //disk

void 
swap_init ()
{
    // printf ("=== swap_init! ===\n");
    // size_t used_index = 8; // 4KB/512
    swap_partition = block_get_role (BLOCK_SWAP); // disk
    swap_bitmap = bitmap_create (block_size (swap_partition)/8); // memory
    bitmap_set_all (swap_bitmap, true);
}
void
swap_in (size_t used_index, void* kaddr)
{
    // printf ("=== swap_in! ===\n");
    // printf ("with slot %d\n", used_index);


    int i;  

    for (i = 0; i < 8; i++)
        block_read (swap_partition, used_index*8 + i, kaddr + BLOCK_SECTOR_SIZE * i);

    
    bitmap_set (swap_bitmap, used_index, true);
    // printf ("=== end swap_in ===\n");
}
size_t 
swap_out(void* kaddr)
{
    // printf ("=== swap_out! ===\n");
    size_t swap_index = bitmap_scan (swap_bitmap, 0, 1, true);

    int i;
    for (i = 0; i < 8; i++)
        block_write (swap_partition, swap_index*8 + i, kaddr + BLOCK_SECTOR_SIZE * i);

    bitmap_set(swap_bitmap, swap_index, false);

    // printf ("with slot %d\n", swap_index);

    return swap_index;
}