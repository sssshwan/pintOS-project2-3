#include "vm/frame.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/interrupt.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

static struct list lru_list;

void
lru_list_init (void)
{
  list_init (&lru_list);
}

void
lru_list_insert (struct page *page)
{
  list_push_back (&lru_list, &page->lru);
}

void
lru_list_delete (struct page *page)
{
  list_remove (&page->lru);
}


struct page *
lru_list_find (void *kaddr)
{
  struct list_elem *e;
  for (e = list_begin (&lru_list); e != list_end (&lru_list); e = list_next (e))
    {
        struct page *page = list_entry (e, struct page, lru);
        if (page->kaddr == kaddr)
        return page;
    }
  return NULL;
}
