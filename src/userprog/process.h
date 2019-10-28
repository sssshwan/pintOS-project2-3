#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

/* pj2 */
/* what we defined */
char * get_cmd_name (void *file_name_);
void stack_argument (void *file_name_, void **esp);

#endif /* userprog/process.h */
