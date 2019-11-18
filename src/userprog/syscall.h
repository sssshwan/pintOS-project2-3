#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void is_valid_file (const char *file);
struct mmap_file * find_mmap_file (mapid_t mapid);

#endif /* userprog/syscall.h */
