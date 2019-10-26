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
		case SYS_HALT:
		{ 
			halt();
			break;
		}

		case SYS_EXIT: 
		{
			if (!is_user_vaddr (ptr + 1))
				goto invalid;

			exit (*(ptr + 1));
			break;
		}
			

		case SYS_EXEC:
		{
			if (!is_user_vaddr (ptr + 1))
				goto invalid;
			
			pid_t result = exec (*(ptr + 1));
			f->eax = result;
			break;
		}

		case SYS_WAIT:
		{
			if (!is_user_vaddr (ptr + 1))
				goto invalid;
				
			int result = wait (*(ptr + 1));
			f->eax = result;
			break;
		}

		case SYS_CREATE: 
		{
			if (!is_user_vaddr (ptr + 1))
				goto invalid;
			if (!is_user_vaddr (ptr + 2))
				goto invalid;
			
			bool result = create (*(ptr + 1), *(ptr + 2));
			f->eax = result;
			break;
		}

		case SYS_REMOVE: 
		{
			if (!is_user_vaddr (ptr + 1))
				goto invalid;

			bool result = remove (*(ptr + 1));
			f->eax = result;
			break;
		}

		case SYS_OPEN: 
		{
			if (!is_user_vaddr (ptr + 1))
				goto invalid;

			int result = open (*(ptr + 1));
			f->eax = result;
			break;
		}

		case SYS_FILESIZE: 
		{
			if (!is_user_vaddr (ptr + 1))
				goto invalid;

			int result = filesize (*(ptr + 1));
			f->eax = result;
			break;
		}

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

		case SYS_SEEK: 
		{
			if (!is_user_vaddr (ptr + 1))
				goto invalid;
			if (!is_user_vaddr (ptr + 2))
				goto invalid;
			
			seek (*(ptr + 1), *(ptr + 2));
			break;
		}

		case SYS_TELL: 
		{
			if(!is_user_vaddr (ptr + 1))
				goto invalid;
			
			unsigned result = tell (*(ptr + 1));
			f->eax = result;
			break;
		}

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

void
halt (void) 
{
	shutdown_power_off();
}

void
exit (int status)
{	
	struct thread *cur = thread_current ();
  /* Save exit status at process descriptor */
  printf("%s: exit(%d)\n" , cur -> name , status);
  thread_exit();

}

pid_t
exec (const char *cmd_line)
{
  return process_execute(cmd_line);

}

int
wait (pid_t pid)
{
  return process_wait(pid);
}

bool
create (const char *file, unsigned initial_size)
{
  return 0;
}

bool
remove (const char *file)
{
  return 0;
}

int
open (const char *file)
{
  return 0;
}

int
filesize (int fd) 
{
  return 0;
}

int
read (int fd, void *buffer, unsigned size)
{
  return 0;
}

int
write (int fd, const void *buffer, unsigned size)
{
	if (fd == 1) {
		putbuf(buffer, size);
		return size;
	}
  	return -1;
}

void
seek (int fd, unsigned position) 
{
  return 0;
}

unsigned
tell (int fd) 
{
  return 0; 
}

void
close (int fd)
{
  return;
}

