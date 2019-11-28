#include "filesys/buffer_cache.h"
#include "threads/palloc.h"
#include <string.h>
#include <debug.h>

#define BUFFER_CACHE_ENTRY_NB 64
static char p_buffer_cache[BUFFER_CACHE_ENTRY_NB * BLOCK_SECTOR_SIZE];
static struct buffer_head buffer_head[BUFFER_CACHE_ENTRY_NB];
static struct buffer_head *clock_hand;


void
bc_init (void)
{
  struct buffer_head *head;
  char *cache = p_buffer_cache;

  for (head = buffer_head; head != buffer_head + BUFFER_CACHE_ENTRY_NB; head++, cache += BLOCK_SECTOR_SIZE)
  {
    memset (head, 0, sizeof (struct buffer_head));
    head->data = cache;
  }

  clock_hand = buffer_head;
}


void
bc_term (void)
{
  struct buffer_head *head;

  for (head = buffer_head; head != buffer_head + BUFFER_CACHE_ENTRY_NB; head++)
    bc_flush_entry (head);
}


bool
bc_read (block_sector_t sector_idx, void *buffer, off_t bytes_written, int chunk_size, int sector_ofs)
{
  struct buffer_head *head;

  if (!(head = bc_lookup (sector_idx)))
  {
    head = bc_select_victim ();
    bc_flush_entry (head);
    head->valid = true;
    head->dirty = false;
    head->sector = sector_idx;
    block_read (fs_device, sector_idx, head->data);
  }

  head->clock = true;
  memcpy (buffer + bytes_written, head->data + sector_ofs, chunk_size);
  return true;
}

bool
bc_write (block_sector_t sector_idx, void *buffer, off_t bytes_written, int chunk_size, int sector_ofs)
{
  struct buffer_head *head;

  if (!(head = bc_lookup (sector_idx)))
  {
    head = bc_select_victim ();
    bc_flush_entry (head);
    head->valid = true;
    head->sector = sector_idx;
    block_read (fs_device, sector_idx, head->data);
  }

  head->clock = true;
  head->dirty = true;
  memcpy (head->data + sector_ofs, buffer + bytes_written, chunk_size);
  return true;
}

struct buffer_head *
bc_lookup (block_sector_t sector)
{
  struct buffer_head *head;

  for (head = buffer_head; head != buffer_head + BUFFER_CACHE_ENTRY_NB; head++)
  {
    if (head->valid && head->sector == sector)
      return head;
  }

  return NULL;
}

struct buffer_head *
bc_select_victim (void)
{
  while (true)
  {
    while (clock_hand != buffer_head + BUFFER_CACHE_ENTRY_NB)
    {
      if (!clock_hand->valid || !clock_hand->clock)
        return clock_hand++;
      clock_hand->clock = false;
      clock_hand++;
    }
    clock_hand = buffer_head;
  }
}

void
bc_flush_entry (struct buffer_head *p_flush_entry)
{
  if (!p_flush_entry->valid || !p_flush_entry->dirty)
    return;

  p_flush_entry->dirty = false;
  block_write (fs_device, p_flush_entry->sector, p_flush_entry->data);

}