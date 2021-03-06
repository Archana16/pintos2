#include "vm/frame.h"
#include <stdio.h>
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "vm/frame.h"

void frame_init(void) {
	list_init(&F_Table);
	lock_init(&F_lock);
}

void *frame_alloc(enum palloc_flags flags,struct page * pte) {
	if ((flags & PAL_USER) == 0) {
		return NULL;
	}
	void *frame = palloc_get_page(flags);
	if (frame) {
		Update_Ftable(frame,pte);
	} else {
		frame = frame_evict();
		if (!frame) {
			PANIC("Frame could not be evicted because swap is full!");
		}
		Update_Ftable(frame,pte);
	}
	return frame;
}

void frame_free(void *frame) {
	struct list_elem *e;
	lock_acquire(&F_lock);
	for (e = list_begin(&F_Table); e != list_end(&F_Table); e = list_next(e)) {
		struct frame *fte = list_entry(e, struct frame, elem);
		if (fte->frame == frame) {
			list_remove(e);
			free(fte);
			break;
		}
	}
	lock_release(&F_lock);

	//free that page in memory
	palloc_free_page(frame);
}

void Update_Ftable(void *f,struct page * pte) {
	struct frame *fte = malloc(sizeof(struct frame));
	fte->frame = f;
	//fte->thread = thread_current();
	fte->spte = pte;
	lock_acquire(&F_lock);
	list_push_back(&F_Table, &fte->elem);
	lock_release(&F_lock);
}
void * frame_evict(void) {
	//clock algorithm

	struct list_elem *e;
	lock_acquire(&F_lock);
	for (e = list_begin(&F_Table); e != list_end(&F_Table); e = list_next(e)) {
		struct frame *fte = list_entry(e, struct frame, elem);
		if (pagedir_is_accessed(thread_current()->pagedir,
				fte->spte->u_vaddr)) {
			pagedir_set_accessed(thread_current()->pagedir, fte->spte->u_vaddr,
					false);
		} else {
			//need to evict
			if (pagedir_is_dirty(thread_current()->pagedir, fte->spte->u_vaddr)
					|| fte->spte->page_flag == SWAP) {
				fte->spte->swap_index = swap_out(fte->frame);
			}
			fte->spte->is_loaded = false;
			return fte->frame;
		}
	}
	lock_release(&F_Table);
	return NULL;
}

