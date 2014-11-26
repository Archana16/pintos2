#include "vm/swap.h"
#include <bitmap.h>
#include <debug.h>
#include <stdio.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

/* The swap device. */
static struct block *swap_device;

/* Used swap pages. */
static struct bitmap *swap_bitmap;

/* Protects swap_bitmap. */
static struct lock swap_lock;

/* Number of sectors per page. */
#define PAGE_SECTORS (PGSIZE / BLOCK_SECTOR_SIZE)

/* Sets up swap. */
void swap_init(void) {
	//block from the swap space
	swap_device = block_get_role(BLOCK_SWAP);
	if (!swap_device) {
		return;
	}
	swap_bitmap = bitmap_create(block_size(swap_device) / SECTORS_PER_PAGE);
	if (!swap_bitmap) {
		return;
	}
	bitmap_set_all(swap_bitmap, SWAP_FREE);
	lock_init(&swap_lock);
}

/* Swaps in page P, which must have a locked frame
 (and be swapped out). */
void swap_in(struct frame *f, size_t index) {
	if (!swap_device || !swap_bitmap) {
			PANIC("No swap partition available!");
		}

	lock_acquire(&swap_lock);
	if (bitmap_test(swap_bitmap, index) == SWAP_FREE) {
		lock_release(&swap_lock);
		return;
	}
	bitmap_flip(swap_bitmap, index);
	lock_release(&swap_lock);
	size_t i;
	for (i = 0; i < SECTORS_PER_PAGE; i++) {
		block_read(swap_device, index * SECTORS_PER_PAGE + i,
				(uint8_t *) f + i * BLOCK_SECTOR_SIZE);
	}

}

/* Swaps out page P, which must have a locked frame. */
bool swap_out(struct frame *f) {
	if (!swap_device || !swap_bitmap) {
		PANIC("No swap partition available!");
	}
	lock_acquire(&swap_lock);
	size_t free_index = bitmap_scan_and_flip(swap_bitmap, 0, 1, SWAP_FREE);
	lock_release(&swap_lock);

	if (free_index == BITMAP_ERROR) {
		PANIC("Swap partition is full!");
	}
	size_t i;
	for (i = 0; i < SECTORS_PER_PAGE; i++) {
		block_write(swap_device, free_index * SECTORS_PER_PAGE + i,
				(uint8_t *) f + i * BLOCK_SECTOR_SIZE);
	}
	return free_index;
}
