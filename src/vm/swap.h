#include <stdio.h>
#include <stddef.h>

#ifndef VM_FRAME_H
#define VM_FRAME_H

void swap_init ();
void swap_in (size_t used_index, void* kaddr);
size_t swap_out (void* kaddr);
#endif