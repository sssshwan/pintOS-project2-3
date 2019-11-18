#include "page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "threads/interrupt.h"
#include <string.h>

static unsigned vm_hash_func (const struct hash_elem *, void * UNUSED);
static bool vm_less_func (const struct hash_elem *, const struct hash_elem *, void * UNUSED);
static void vm_destroy_func (struct hash_elem *e, void *aux UNUSED);

/* initialize vm */
void vm_init (struct hash *vm)
{
  hash_init (vm, vm_hash_func, vm_less_func, NULL);   
}


static unsigned
vm_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  return hash_int (hash_entry (e, struct vm_entry, elem)->vaddr);
}


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
  // struct vm_entry *vme = (struct vm_entry *) malloc (sizeof (struct vm_entry));
  // struct hash_elem *e;

  // vme->vaddr = pg_round_down (vaddr);

  // e = hash_find (vm, &vme->elem);
  // free(vme);
  // if (e == NULL)
  //   return NULL;
  // return hash_entry (e, struct vm_entry, elem);
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
vm_destroy (struct hash *vm)
{
  hash_destroy (vm, vm_destroy_func);
}


static void
vm_destroy_func (struct hash_elem *e, void *aux UNUSED)
{
  struct vm_entry *vme = hash_entry (e, struct vm_entry, elem);
  free (vme);
}


bool 
load_file (void *kaddr, struct vm_entry *vme) 
{
  // printf("== load_file in ==\n");
  off_t num_read;

  /* file_read_at return # of bytes acturally read */
  // printf("file_length: %d\n", file_length (vme->file));
  num_read = file_read_at (vme->file, kaddr, vme->read_bytes, vme->offset);

  if (num_read != vme->read_bytes)
  {
    // printf ("problem in file_read_at\n");
    // printf ("num_read: %d, read_bytes: %d\n",num_read, vme->read_bytes);
    return false;
  }

  memset (kaddr + num_read, 0, vme->zero_bytes);
  return true;
}
static void collect (void)
{

  struct page *victim = lru_pop ();

  bool dirty = pagedir_is_dirty (victim->thread->pagedir, victim->vme->vaddr);
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
  free_page(victim);
}

struct page *
alloc_page (enum palloc_flags flags)
{
  struct page *page;
  page = (struct page *)malloc (sizeof (struct page));
  if (page == NULL)
    return NULL;
  memset (page, 0, sizeof (struct page));
  page->thread = thread_current ();
  ASSERT (page->thread);
  ASSERT (page->thread->magic == 0xcd6abf4b);
  page->kaddr = palloc_get_page (flags);
  while (page->kaddr == NULL)
    {
      collect ();
      page->kaddr = palloc_get_page (flags);
    }
  return page;
}
// /* allocate physical page for given flag */
// struct page *
// alloc_page (enum palloc_flags flags)
// {
//   struct page *page;
//   page = (struct page *) malloc (sizeof (struct page));

//   page->thread = thread_current ();
//   page->kaddr = palloc_get_page (flags);
//   if (page->kaddr == NULL)
//   {
//     printf ("palloc_get_page return NULL!!\n");
//   }

//   lru_list_insert (page);

//   return page;
// }

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
