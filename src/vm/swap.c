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
    size_t used_index = 8; // 4KB/512
    swap_bitmap = bitmap_create (used_index); // memory
    swap_partition = block_get_role (BLOCK_SWAP); // disk
}
void 
swap_in (size_t used_index, void* kaddr)
{
  int i;

  for (i = 0; i < 8; i++)
  {
    block_read (swap_partition, used_index*8 + i, kaddr + BLOCK_SECTOR_SIZE * i);
  }
  
  bitmap_set (swap_bitmap, used_index, true);
}
size_t 
swap_out(void* kaddr)
{
    // struct vm_entry *vme;
    // struct page *page;

    // vme = find_vme (kaddr); // vme with input kaddr.
    // page = lru_list_find (vme); // victim page?
    size_t swap_index = bitmap_scan (swap_bitmap, 0, 1, true);
    int i;

    for (i = 0; i < 8; i++)
    {
        block_write (swap_partition, swap_index*8 + i, kaddr + BLOCK_SECTOR_SIZE * i);
    }

    bitmap_set(swap_bitmap, swap_index, false);

    return swap_index;


    // if (pagedir_is_dirty (page->thread->pagedir, page->kaddr))
    // {
    //     // page is dirty
    //     // 변경내용을 디스크에 기록.. 어떻게 하누
    // }

    // lru_list_delete (page); // page deleted from lru (pysical memory list)
}