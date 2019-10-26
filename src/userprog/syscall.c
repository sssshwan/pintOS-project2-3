#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

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


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  int *ptr = f->esp; 

  if (!is_user_vaddr (ptr))  
    goto invalid;

  if (*ptr < SYS_HALT || *ptr > SYS_INUMBER)
    goto invalid;

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
      goto invalid;

      exit (*(ptr + 1));
      break;
    }
			
    /* case exec */
    case SYS_EXEC:
    {
      if (!is_user_vaddr (ptr + 1))
	      goto invalid;
			
      pid_t result = exec (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case wait */
    case SYS_WAIT:
    {
      if (!is_user_vaddr (ptr + 1))
        goto invalid;
				
      int result = wait (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case create */
    case SYS_CREATE: 
    {
      if (!is_user_vaddr (ptr + 1))
        goto invalid;
      if (!is_user_vaddr (ptr + 2))
        goto invalid;

      bool result = create (*(ptr + 1), *(ptr + 2));f->eax = result;
      break;
    }

    /* case remove */
    case SYS_REMOVE: 
    {
      if (!is_user_vaddr (ptr + 1))
        goto invalid;

      bool result = remove (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case open */
    case SYS_OPEN: 
    {
      if (!is_user_vaddr (ptr + 1))
        goto invalid;

      int result = open (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case fileszie */
    case SYS_FILESIZE: 
    {
      if (!is_user_vaddr (ptr + 1))
        goto invalid;

      int result = filesize (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case read */
    case SYS_READ: 
    {
      if (!is_user_vaddr (ptr + 1))
        goto invalid;
      
      if (!is_user_vaddr (ptr + 2))
        goto invalid;

      if (!is_user_vaddr (ptr + 3))
        goto invalid;
		
      int result = read (*(ptr + 1), ptr + 2, *(ptr + 3));
      f->eax = result;
      break; 
    }

    /* case write */
    case SYS_WRITE: 
    {
      if (!is_user_vaddr (ptr + 1))
        goto invalid;
			
      if (!is_user_vaddr (ptr + 2))
        goto invalid;
			
      if (!is_user_vaddr (ptr + 3))
        goto invalid;
      
      int result = write (*(ptr + 1), *(ptr + 2), *(ptr + 3));
      f->eax = result;
      break;
    }
		
    /* case seek */
    case SYS_SEEK: 
    {
      if (!is_user_vaddr (ptr + 1))
        goto invalid;
      if (!is_user_vaddr (ptr + 2))
        goto invalid;
			  
      seek (*(ptr + 1), *(ptr + 2)); 
      break;
    }

    /* case tell */
    case SYS_TELL: 
    {
      if(!is_user_vaddr (ptr + 1))
        goto invalid;
  
      unsigned result = tell (*(ptr + 1));
      f->eax = result;
      break;
    }

    /* case close */
    case SYS_CLOSE:
    {
      if(!is_user_vaddr (ptr + 1))
        goto invalid;

      close (*(ptr + 1));
      break;
    }
  }
  return;

invalid:
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
  printf("%s: exit(%d)\n" , cur -> name , status);
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
  return 0;
}

/* pj2: remove system call */
bool
remove (const char *file)
{
  return 0;
}

/* pj2: open system call */
int
open (const char *file)
{
  return 0;
}

/* pj2: filesize system call */
int
filesize (int fd) 
{
  return 0;
}

/* pj2: read system call */
int
read (int fd, void *buffer, unsigned size)
{
  return 0;
}

/* pj2: write system call */
int
write (int fd, const void *buffer, unsigned size)
{
  if (fd == 1) {
    putbuf(buffer, size);
    return size;
  }
    return -1;
}

/* pj2: seek system call */
void
seek (int fd, unsigned position) 
{
  return 0;
}

/* pj2: tell system call */
unsigned
tell (int fd) 
{
  return 0; 
}

/* pj2: close system call */
void
close (int fd)
{
  return;
}

