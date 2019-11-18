  
#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "vm/page.h"

void lru_list_init (void);
void lru_list_insert (struct page *);
void lru_list_delete (struct page *);
struct page *lru_list_find (void *);
struct page *lru_pop ();

#endif