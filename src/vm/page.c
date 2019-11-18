#include "page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "threads/interrupt.h"
#include <string.h>
#include <stdio.h>

static unsigned vm_hash_func (const struct hash_elem *, void * UNUSED);
static bool vm_less_func (const struct hash_elem *, const struct hash_elem *, void * UNUSED);
static void vm_destroy_func (struct hash_elem *e, void *aux UNUSED);

/* initialize vm using hash_int function */
void vm_init (struct hash *vm)
{
  hash_init (vm, vm_hash_func, vm_less_func, NULL);   
}

/* hash vm entry with hash_int function */
static unsigned
vm_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  return hash_int (hash_entry (e, struct vm_entry, elem)->vaddr);
}

/* return element with least vaddr */
static bool
vm_less_func (const struct hash_elem *a,
              const struct hash_elem *b, void *aux UNUSED)
{
  return hash_entry (a, struct vm_entry, elem)->vaddr
      < hash_entry (b, struct vm_entry, elem)->vaddr;
}

/* hash_insert success >> NULL, fail >> elem */
/* insert_vme success >> true, fail >> false */
bool
insert_vme (struct hash *vm, struct vm_entry *vme)
{
  return hash_insert (vm, &vme->elem) == NULL;
}

/* hash_delete success >> elem, fail >> NULL */
/* delete_vme success >> true, fail >> false */
bool
delete_vme (struct hash *vm, struct vm_entry *vme)
{
  return hash_delete (vm, &vme->elem) != NULL; 
}


/* find_vme success >> vme, fail >> NULL */
struct vm_entry *
find_vme (void *vaddr)
{
  struct hash *vm = &thread_current ()->vm;
  struct hash_iterator i;
  struct vm_entry *vme;
  int id = 0;

  hash_first (&i, vm);
  while (hash_next (&i))
  {
    vme = hash_entry (hash_cur (&i), struct vm_entry, elem);

    if (vme->vaddr == pg_round_down(vaddr))
    {
      return vme;
    }
  }
  return NULL;
}

void
show_vm ()
{
  struct hash *vm = &thread_current ()->vm;
  struct hash_iterator i;
  struct vm_entry *vme;
  int it = 0;

  hash_first (&i, vm);
  printf ("===== welcome to show_vm ======\n");
  while (hash_next (&i))
  {
    vme = hash_entry (hash_cur (&i), struct vm_entry, elem);
    printf ("%d th vme\n", ++it);
    printf ("vme->vaddr: %u\n", vme->vaddr);
    printf ("vme->type: %d\n", vme->type);
  }
  return NULL;
}

void
show_vme (struct vm_entry *vme)
{
  printf ("vme->vaddr: %u\n", vme->vaddr);
  printf ("vme->type: %d\n", vme->type);
}
/* destroy vm using hash_destroy function */
void 
vm_destroy (struct hash *vm)
{
  hash_destroy (vm, vm_destroy_func);
}

/* free vm entry for corresponding hash element */
static void
vm_destroy_func (struct hash_elem *e, void *aux UNUSED)
{
  struct vm_entry *vme = hash_entry (e, struct vm_entry, elem);
  free (vme);
}


bool 
load_file (void *kaddr, struct vm_entry *vme) 
{
  off_t num_read;

  /* file_read_at return # of bytes acturally read */
  num_read = file_read_at (vme->file, kaddr, vme->read_bytes, vme->offset);

  if (num_read != vme->read_bytes)
    return false;

  memset (kaddr + num_read, 0, vme->zero_bytes);
  return true;
}

static void collect (void)
{
  // printf ("=== collect === \n");
  struct page *victim = lru_front ();
  // show_vme (victim->vme);

  bool dirty; // = true;
  dirty = pagedir_is_dirty (victim->thread->pagedir, victim->vme->vaddr);
  //printf ("dirty : %d\n", dirty);
  //show_vme (victim->vme);
  switch (victim->vme->type)
    {
      case VM_BIN:
        if (dirty)
          {
            victim->vme->swap_slot = swap_out (victim->kaddr);
            victim->vme->type = VM_ANON;
          }
        break;
      case VM_FILE:
        if (dirty)
          {
            if (file_write_at (victim->vme->file, victim->vme->vaddr,
                               victim->vme->read_bytes, victim->vme->offset)
              != (int) victim->vme->read_bytes)
              NOT_REACHED ();
          }
        break;
      case VM_ANON:
        victim->vme->swap_slot = swap_out (victim->kaddr);
        break;
      default:
        NOT_REACHED ();
    }
  victim->vme->is_loaded = false;
  // show_vme (victim->vme);
  free_page (victim->kaddr);
}

struct page *
alloc_page (enum palloc_flags flags)
{
  struct page *page;
  // printf ("===alloc_page ===\n");
  page = (struct page *) malloc (sizeof (struct page));
  if (page == NULL)
    return NULL;
  memset (page, 0, sizeof (struct page));
  page->thread = thread_current ();
  page->kaddr = palloc_get_page (flags);
  // collect ();
  int cnt = 0;
  while (page->kaddr == NULL)
    {
      // printf ("Null occured \n");
      collect ();
      page->kaddr = palloc_get_page (flags);
    }
  // lru_list_insert (page);
  return page;
}

/* free physical page for coresponding physical address */
void
free_page(void *addr)
{
  struct page *page = lru_list_find (addr);
  if (page){
    pagedir_clear_page (page->thread->pagedir, page->vme->vaddr);
    lru_list_delete (page);
    palloc_free_page (page->kaddr);
    free (page);
  }
}
