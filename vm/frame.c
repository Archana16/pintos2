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

void *frame_alloc(enum palloc_flags flags) {
	if ((flags & PAL_USER) == 0) {
		return NULL;
	}
	void *frame = palloc_get_page(flags);
	if (frame) {
		Update_Ftable(frame);
	} else {
		if (!frame_evict(frame)) {
			PANIC("Frame could not be evicted because swap is full!");
		}
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

void Update_Ftable(void *f) {
	struct frame *fte = malloc(sizeof(struct frame));
	fte->frame = f;
	//fte->thread = thread_current();
	lock_acquire(&F_lock);
	list_push_back(&F_Table, &fte->elem);
	lock_release(&F_lock);
}
bool frame_evict(void *frame) {
	return false;
// Use clock algorithm with 2 hands
}
