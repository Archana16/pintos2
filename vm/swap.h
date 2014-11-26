#ifndef VM_SWAP_H
#define VM_SWAP_H 1

#include <stdbool.h>
#include "devices/block.h"
#include "threads/synch.h"
#include <bitmap.h>

#define SWAP_FREE 0

#define SWAP_IN_USE 1

#define SECTORS_PER_PAGE (PGSIZE/BLOCK_SECTOR_SIZE)

struct lock swap_lock;

struct block *swap_block;

struct bitmap swap_map;

void swap_init (void);
void swap_in (struct frame *);
bool swap_out (struct frame *);

#endif /* vm/swap.h */
