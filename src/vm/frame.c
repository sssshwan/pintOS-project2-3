#include "vm/frame.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/interrupt.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"


/* lru_list manage physical pages in use as a list of pages */
static struct list lru_list;

/* initializer lru list*/
void
lru_list_init (void)
{
  list_init (&lru_list);
}

/* insert corresponding page to lru_list */
void
lru_list_insert (struct page *page)
{
  list_push_back (&lru_list, &page->lru);
}

/* delete corresponding page from lru_list */
void
lru_list_delete (struct page *page)
{
  list_remove (&page->lru);
}

/* find corresponding page from lru_list */
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

/* select victim of swapping */
struct page *
evict_frame (void)
{
  struct list_elem *e;
  struct page *page;
  struct vm_entry *vme;
  bool dirty;
  bool accessed;

  while (true){
    for (e = list_begin (&lru_list); e != list_end (&lru_list); e = list_next(e))
    { 
      page = list_entry (e, struct page, lru);
      
      dirty = pagedir_is_dirty (page->thread->pagedir, page->vme->vaddr);
      accessed = pagedir_is_accessed(page->thread->pagedir, page->vme->vaddr);

      if (!accessed && !dirty && !(vme->is_loaded)){
        lru_list_delete(page);
        return page;
      }
    
    }
    for (e = list_begin (&lru_list); e != list_end (&lru_list); e = list_next(e))
    { 
      page = list_entry (e, struct page, lru);
      
      dirty = pagedir_is_dirty (page->thread->pagedir, page->vme->vaddr);
      accessed = pagedir_is_accessed(page->thread->pagedir, page->vme->vaddr);

      if (!accessed && !dirty && !(vme->is_loaded)){
        lru_list_delete(page);
        return page;
      }
      if (accessed && !(vme->is_loaded)){
	        pagedir_set_accessed (page->thread->pagedir, page->vme->vaddr, false);
      }
    }
  }

  return page;
}


  