#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

typedef int pid_t;

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

struct lock file_lock;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{
  int *ptr = f->esp; 

  if (!is_user_vaddr (ptr))  
    return;

  if (*ptr < SYS_HALT || *ptr > SYS_INUMBER)
    return;

  //printf("syscall num : %d\n", *(uint32_t *)(f->esp));
  //hex_dump(f->esp, f->esp, 100, 1); 
	  
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
      if (!is_user_vaddr (ptr + 1))
      return;

      exit (*(ptr + 1));
      break;
    }
			
    /* case exec */
    case SYS_EXEC:
    {
      if (!is_user_vaddr (ptr + 1))
	      return;
			
      pid_t result = exec (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case wait */
    case SYS_WAIT:
    {
      if (!is_user_vaddr (ptr + 1))
        return;
				
      int result = wait (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case create */
    case SYS_CREATE: 
    {
      if (!is_user_vaddr (ptr + 1))
        return;
      if (!is_user_vaddr (ptr + 2))
        return;

      bool result = create (*(ptr + 1), *(ptr + 2));f->eax = result;
      break;
    }

    /* case remove */
    case SYS_REMOVE: 
    {
      if (!is_user_vaddr (ptr + 1))
        return;

      bool result = remove (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case open */
    case SYS_OPEN: 
    {
      if (!is_user_vaddr (ptr + 1))
        return;

      int result = open (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case fileszie */
    case SYS_FILESIZE: 
    {
      if (!is_user_vaddr (ptr + 1))
        return;

      int result = filesize (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case read */
    case SYS_READ: 
    {
      if (!is_user_vaddr (ptr + 1))
        return;
      
      if (!is_user_vaddr (ptr + 2))
        return;

      if (!is_user_vaddr (ptr + 3))
        return;
		
      int result = read (*(ptr + 1), *(ptr + 2), *(ptr + 3));
      f->eax = result;
      break; 
    }

    /* case write */
    case SYS_WRITE: 
    {
      if (!is_user_vaddr (ptr + 1))
        return;
			
      if (!is_user_vaddr (ptr + 2))
        return;
			
      if (!is_user_vaddr (ptr + 3))
        return;
      
      int result = write (*(ptr + 1), *(ptr + 2), *(ptr + 3));
      f->eax = result;
      break;
    }
		
    /* case seek */
    case SYS_SEEK: 
    {
      if (!is_user_vaddr (ptr + 1))
        return;
      if (!is_user_vaddr (ptr + 2))
        return;
			  
      seek (*(ptr + 1), *(ptr + 2)); 
      break;
    }

    /* case tell */
    case SYS_TELL: 
    {
      if(!is_user_vaddr (ptr + 1))
        return;
  
      unsigned result = tell (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case close */
    case SYS_CLOSE:
    {
      if(!is_user_vaddr (ptr + 1))
        return;

      close (*(ptr + 1));
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

  /* close all files in file descriptor table */
  int fd;
  for(fd=3; fd<64; fd++){
    if (cur->file_fdt[fd] != NULL){
      close(fd);
    }
  }
  thread_exit();
}

/* pj2: exec system call */
pid_t
exec (const char *cmd_line)
{
  return process_execute(cmd_line);

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
  is_valid_file(file);

  return filesys_create(file, initial_size);
}

/* pj2: remove system call */
bool
remove (const char *file)
{
  /* check validty of file pointer */
  is_valid_file(file);

  return filesys_remove(file);
}

/* pj2: open system call */
int
open (const char *file)
{
  /* check validty of file pointer */
  is_valid_file(file);

  struct file *fp = filesys_open(file);
  if(!fp) return -1;

  struct thread *cur = thread_current();

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
  if(! cur->file_fdt[fd])
    exit(-1);
  return file_length(cur->file_fdt[fd]);
}

/* pj2: read system call */
int
read (int fd, void *buffer, unsigned size)
{ 
  struct thread *cur = thread_current();
  //is_user_vaddr(buffer);

  if (fd == 0){
    int i;
    for (i = 0; i < size; i ++) {
      if (((char *)buffer)[i] == '\0') 
        return i;
    }   
  }
  else if(fd == 1)
    return -1;
  else if (fd > 2){
    if (!cur->file_fdt[fd]) 
      exit(-1);
    return file_read(cur->file_fdt[fd], buffer, size);
  }
}

/* pj2: write system call */
int
write (int fd, const void *buffer, unsigned size)
{
  struct thread *cur = thread_current();
  int ret;

  //lock_acquire(&file_lock);
  if (fd == 0)
    ret = -1;
  else if (fd == 1) {
    putbuf(buffer, size);
    ret = size;
  } 
  else if (fd > 2){
    if (! cur->file_fdt[fd])
      //lock_release(&file_lock); 
      exit(-1);
    ret = file_write(cur->file_fdt[fd], buffer, size);
  }
  //lock_release(&file_lock); 
  
  return ret;
}

/* pj2: seek system call */
void
seek (int fd, unsigned position) 
{
  struct thread *cur = thread_current();
  if(!cur->file_fdt[fd])
    exit(-1);
  return file_seek(cur->file_fdt[fd], position); 
}

/* pj2: tell system call */
unsigned
tell (int fd) 
{
  struct thread *cur = thread_current();
  if(!cur->file_fdt[fd])
    exit(-1);
  return file_tell(cur->file_fdt[fd]); 
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

  return ;
}

void
is_valid_file (const char *file)
{
  if(!file) exit(-1);
  if(!is_user_vaddr(file)) exit(-1);
  //if(pagedir_get_page(cur->pagedir, file)) exit(-1);
}
