#ifndef VM_SWAP_H
#define VM_SWAP_H 1

#include <stdbool.h>
#include "devices/block.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include <bitmap.h>

#define SWAP_FREE 0

#define SWAP_IN_USE 1

#define SECTORS_PER_PAGE (PGSIZE/BLOCK_SECTOR_SIZE)

void swap_init (void);
void swap_in(void *f, size_t index);
bool swap_out (void * frame);

#endif /* vm/swap.h */
