#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "lib/user/syscall.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

/* pj2 */
/* what we defined */
char * get_cmd_name (void *file_name_);

/* pj3 */
bool handle_mm_fault (struct vm_entry *vme);
void stack_growth (void *addr);

#endif /* userprog/process.h */

