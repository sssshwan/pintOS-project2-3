#include <stdio.h>
#include <syscall-nr.h>
#include <stddef.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "lib/user/syscall.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "vm/page.h"

static void syscall_handler (struct intr_frame *);

void halt (void);
void exit (int status);
pid_t exec (const char *cmd_line);
int wait (pid_t pid);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
int mmap (int fd, void *addr);
void munmap (int mapid);

/* pj3 */
struct vm_entry * check_address (void* addr, void* esp /*Unused*/);
void check_valid_buffer (void* buffer, unsigned size, void* esp, bool to_write);
void check_valid_string (const void* str, void* esp);


static struct lock filesys_lock; 

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&filesys_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{
  // printf ("==syscall_handelr in!==\n");

  int *ptr = f->esp;
  struct thread *cur = thread_current(); 
  cur->esp = f->esp;

  /* pj3 */
  check_address (ptr, ptr);

  /* pj2: check vality of pointer */
  if (!is_user_vaddr (ptr))  
    return;

  if (!pagedir_get_page (cur->pagedir, ptr))
    exit(-1);

  if (*ptr < SYS_HALT || *ptr > SYS_INUMBER)
    return;

  /* case each system call */
  switch (*ptr)
  {
    /* case halt */
    case SYS_HALT:
    { 
      halt();
      break;
    }

    /* case exit */
    case SYS_EXIT: 
    {
      /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1))
      return;

      /* pj2: call exit */
      exit (*(ptr + 1));
      break;
    }
			
    /* case exec */
    case SYS_EXEC:
    { 
      /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1))
	      return;
			
       /* pj2: call exec */
      pid_t result = exec (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case wait */
    case SYS_WAIT:
    {
       /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1))
        return;
				
       /* pj2: call wait */
      int result = wait (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case create */
    case SYS_CREATE: 
    { 
      /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1) || !is_user_vaddr (ptr + 2))
        return;

      /* pj2: call create */
      bool result = create (*(ptr + 1), *(ptr + 2));
      f->eax = result;
      break;
    }

    /* case remove */
    case SYS_REMOVE: 
    {
      /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1))
        return;

      /* pj2: call remove */
      bool result = remove (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case open */
    case SYS_OPEN: 
    { 
      /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1))
        return;

      /* pj2: call open */
      int result = open (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case filesize */
    case SYS_FILESIZE: 
    {
      /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1))
        return;

       /* pj2: call filesize */
      int result = filesize (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case read */
    case SYS_READ: 
    { 
       /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1) || !is_user_vaddr (ptr + 2) || !is_user_vaddr (ptr + 3))
        return;

      /* pj2: call read */
      int result = read (*(ptr + 1), *(ptr + 2), *(ptr + 3));
      f->eax = result;
      break; 
    }

    /* case write */
    case SYS_WRITE: 
    {
       /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1) || !is_user_vaddr (ptr + 2) || !is_user_vaddr (ptr + 3))
        return;

      /* pj2: call write */    
      int result = write (*(ptr + 1), *(ptr + 2), *(ptr + 3));
      f->eax = result;
      break;
    }
		
    /* case seek */
    case SYS_SEEK: 
    { 
      /* pj2: check vality of pointer */
      if (!is_user_vaddr (ptr + 1) || !is_user_vaddr (ptr + 2))
        return;

       /* pj2: call seek */    
      seek (*(ptr + 1), *(ptr + 2)); 
      break;
    }

    /* case tell */
    case SYS_TELL: 
    {
       /* pj2: check vality of pointer */
      if(!is_user_vaddr (ptr + 1))
        return;

      /* pj2: call tell */    
      unsigned result = tell (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case close */
    case SYS_CLOSE:
    {
      /* pj2: check vality of pointer */
      if(!is_user_vaddr (ptr + 1))
        return;

       /* pj2: call close */    
      close (*(ptr + 1));
      break;
    }

    /* case mmap */
    case SYS_MMAP:
    {
      if(!is_user_vaddr (ptr + 1) || ! is_user_vaddr(ptr + 2))
        return;
      f->eax = mmap (*(ptr +1), *(ptr+2));
      break;
    }
      
    /* case munmap */
    case SYS_MUNMAP:
    {
      if(!is_user_vaddr (ptr + 1))
        return;
      munmap (*(ptr+1));
      break;
    }

    case SYS_MKDIR:
    {
      if(!is_user_vaddr (ptr + 1))
        return;
      f->eax = mkdir (*(ptr +1));
      break;
    }

    case SYS_CHDIR:
    {
      if(!is_user_vaddr (ptr + 1))
        return;
      f->eax = chdir (*(ptr +1));
      break;
    }

    case SYS_READDIR:
    {
      if(!is_user_vaddr (ptr + 1) || ! is_user_vaddr(ptr + 2))
        return;
      f->eax = readdir (*(ptr +1), *(ptr+2));
      break;
    }
    
    case SYS_ISDIR:
    {
      if(!is_user_vaddr (ptr + 1))
        return;
      f->eax = isdir (*(ptr +1));
      break;
    }

    case SYS_INUMBER:
    {
       if(!is_user_vaddr (ptr + 1))
        return;
      f->eax = inumber (*(ptr +1));
      break;
    }
  }
  return;
}

/* pj2: halt system call */
void
halt (void) 
{
  shutdown_power_off();
}

/* pj2: exit system call */
void
exit (int status)
{	
	struct thread *cur = thread_current ();
  /* Save exit status at process descriptor */
  printf("%s: exit(%d)\n" , thread_name () , status);
  cur->exit_status = status;

  thread_exit();
}

/* pj2: exec system call */
pid_t
exec (const char *cmd_line)
{ 
  tid_t tid;
  /* lock acquire */
  lock_acquire (&filesys_lock);
  tid = process_execute(cmd_line);

  /* lock release */
  lock_release (&filesys_lock);

  return tid;
}

/* pj2: wait system call */
int
wait (pid_t pid)
{
  return process_wait(pid);
}

/* pj2: create system call */
bool
create (const char *file, unsigned initial_size)
{
  /* check validty of file pointer */
  if(!file) exit(-1);
  if(!is_user_vaddr(file)) exit(-1);

  return filesys_create(file, initial_size);
}

/* pj2: remove system call */
bool
remove (const char *file)
{
  /* check validty of file pointer */
  if(!file) exit(-1);
  if(!is_user_vaddr(file)) exit(-1);

  return filesys_remove(file);
}

/* pj2: open system call */
int
open (const char *file)
{
  /* check validty of file pointer */
  if(!file) exit(-1);
  if(!is_user_vaddr(file)) exit(-1);

  struct file *fp = filesys_open(file);
  if(!fp) return -1;

  struct thread *cur = thread_current();

  /* pj2: ile deny write when thread is file related*/
  if(strcmp(thread_name(), file) == 0)
    file_deny_write(fp);


  /* pj2: find smallest number of available fd */
  int i;
  for(i=3; i<64; i++){
    if(!cur->file_fdt[i]){
      cur->file_fdt[i] = fp;
      break;
    }
  }

  return i;
}

/* pj2: filesize system call */
int
filesize (int fd) 
{ 
  struct thread *cur = thread_current();
  int ret;

  if(! cur->file_fdt[fd]) exit(-1);
  ret = file_length(cur->file_fdt[fd]);

  return ret;
}

/* pj2: read system call */
int
read (int fd, void *buffer, unsigned size)
{ 
  struct thread *cur = thread_current();
  int ret;
  /* pj2: check validty of pointer */
  if(!is_user_vaddr(buffer)) exit(-1);
  if(!buffer) exit(-1);

  /* pj2: case stanard input */
  /* lock acquire */
  lock_acquire (&filesys_lock);
  if (fd == 0){  
    int i;
    for (i = 0; i < size; i++)
      ((uint8_t *) buffer)[i] = input_getc();
    ret = size;   
  }
  /* pj2: case stanard output */
  else if(fd == 1)   
    ret = -1;
  /* pj2: case normal file descriptor */
  else if (fd > 2){ 
    if (!cur->file_fdt[fd]) exit(-1);
    ret = file_read(cur->file_fdt[fd], buffer, size);
  }
  /* lock release */
  lock_release (&filesys_lock);
  return ret;
}

/* pj2: write system call */
int
write (int fd, const void *buffer, unsigned size)
{
  // printf ("==write==\n");
  struct thread *cur = thread_current();
  int ret;

  /* pj2: check validty of pointer */
  if(!is_user_vaddr(buffer)) exit(-1);
  if(!buffer) exit(-1);
  if(!pagedir_get_page(cur->pagedir, buffer)) exit(-1);
  
  /* pj2: case stanard input */
  /* lock acquire */
  lock_acquire(&filesys_lock);
  if (fd == 0) 
    ret = -1;
  /* pj2: case stanard output */
  else if (fd == 1) {
    putbuf(buffer, size);
    ret = size;
  }
  else if (fd == 2) ret = -1;
  /* pj2: case normal file descriptor */
  else if (fd > 2){
    if (! cur->file_fdt[fd]) exit(-1);
    ret = file_write(cur->file_fdt[fd], buffer, size);
  }  
  /* lock release */
  lock_release (&filesys_lock);
  return ret;
}

/* pj2: seek system call */
void
seek (int fd, unsigned position) 
{
  struct thread *cur = thread_current();
  if(!cur->file_fdt[fd]) 
    exit(-1);

  file_seek(cur->file_fdt[fd], position);

  return; 
}

/* pj2: tell system call */
unsigned
tell (int fd) 
{
  struct thread *cur = thread_current();
  unsigned ret;

  if(!cur->file_fdt[fd]) 
    exit(-1);

  ret = file_tell(cur->file_fdt[fd]); 

  return ret;
}

/* pj2: close system call */
void
close (int fd)
{
  struct thread *cur = thread_current();
  if (! cur->file_fdt[fd]) 
    exit(-1);

  file_close(cur->file_fdt[fd]);
  cur->file_fdt[fd] = NULL;

}

/* pj3: mmap system call */
int
mmap (int fd, void *addr)
{
  struct thread *t = thread_current();
  struct mmap_file *mmap_file;
  size_t offset = 0;

  if (addr == NULL)
    return -1;
  if (! is_user_vaddr (addr))
    exit(-1);
  if (pg_ofs(addr) != 0)
    return -1;
  if (fd == 0 || fd == 1)
    return -1;

  mmap_file = (struct mmap_file *)malloc(sizeof (struct mmap_file));
  if (!mmap_file || file_length(mmap_file) == 0)
    return -1;

  list_init (&mmap_file->vme_list);
  if (!(mmap_file->file = t->file_fdt[fd]))
    return -1;

  mmap_file->file = file_reopen(mmap_file->file);
  mmap_file->mapid = t->next_mapid++;
  
  list_push_back (&t->mmap_list, &mmap_file->elem);

  int length = file_length (mmap_file->file);

  while (length > 0)
  {
    if (find_vme (addr))
      return -1;

    struct vm_entry *vme = (struct vm_entry *) malloc(sizeof(struct vm_entry));
    
    vme->read_bytes = length < PGSIZE ? length : PGSIZE;
    vme->zero_bytes = 0;
    vme->file = mmap_file->file;
    vme->type = VM_FILE;
    vme->writable = true;
    vme->vaddr = addr;
    vme->offset = offset;

    list_push_back (&mmap_file->vme_list, &vme->mmap_elem);
    insert_vme (&t->vm, vme);
    
    offset += PGSIZE;
    addr += PGSIZE;
    length -= PGSIZE;
  }

  return mmap_file->mapid;
}

/* find correspoind mmap_file in mmap_list */
/* reuturn mmap_file if success else NULL */
struct mmap_file * 
find_mmap_file (mapid_t mapid)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  for (e = list_begin (&t->mmap_list); e != list_end (&t->mmap_list); e = list_next (e))
  {
    struct mmap_file *f = list_entry (e, struct mmap_file, elem);
    if (f->mapid == mapid)
      return f;
  }
  return NULL; 
}


/* pj3: munmap system call */
void
munmap (mapid_t mapid)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct vm_entry *vme;
  struct mmap_file *mmap_file = find_mmap_file (mapid);

  if (! mmap_file)
    exit(-1);

  do_munmap(mmap_file);
}

/* pj4: */
bool
mkdir (const char *dir)
{
  return 1;
}

/* pj4: */
bool
chdir (const char *dir)
{
  return 1;
}

/* pj4: */
bool
readdir (int fd, char *name)
{
  return 1;
}

/* pj4: */
bool
isdir (int fd)
{
  return 1;
}

/* pj4: */
int
inumber (int fd)
{
  return 1;
}

/* pj3: check address */
struct vm_entry * 
check_address (void* addr, void* esp /*Unused*/) 
{
  if(addr < (void *)0x08048000 || addr >= (void *)0xc0000000)
    exit(-1);
  /*addr이 vm_entry에 존재하면 vm_entry를 반환하도록 코드 작성 */ 
  /*find_vme() 사용*/
  return find_vme (addr);
}

void 
check_valid_buffer (void* buffer, unsigned size, void* esp, bool to_write)
{
  /* 인자로 받은 buffer부터 buffer + size까지의 크기가 한 페이지의 크기를 넘을 수도 있음 */
  /* check_address를 이용해서 주소의 유저영역 여부를 검사함과 동시 에 vm_entry 구조체를 얻음 */
  /* 해당 주소에 대한 vm_entry 존재여부와 vm_entry의 writable 멤 버가 true인지 검사 */
  /* 위 내용을 buffer 부터 buffer + size까지의 주소에 포함되는 vm_entry들에 대해 적용 */
  check_address (buffer, esp);
  check_address (buffer+size, esp);
}

void 
check_valid_string (const void* str, void* esp)
{
  check_address (str, esp);
}

