#include "vm/frame.h"
#include <stdio.h>
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

/* Initialize the frame manager. */

void frame_init(void) {
	list_init(&FT_list);
	lock_init(&FT_lock);
}

/* Tries to allocate and lock a frame for PAGE.
 Returns the frame if successful, false on failure. */
struct frame *
frame_alloc_and_lock(void *frame, enum palloc_flags flags) {
	if (frame) {
		update_Ftable(frame);
	} else {
		while (!frame) {
			frame = frame_evict(flags);
			lock_release(&FT_lock);
		}
		if (flags & PAL_ASSERT)
			PANIC("palloc_get: out of pages");
		update_Ftable(frame);
	}
	return frame;
}

/* Locks P's frame into memory, if it has one.
 Upon return, p->frame will not change until P is unlocked. */
/*
 void
 frame_lock (struct page *p)
 {
 }
 */

/* Releases frame F for use by another page.
 F must be locked for use by the current process.
 Any data in F is lost. */
void frame_free(struct frame *frame) {
	struct list_elem *e;
	lock_acquire(&FT_list);
	for (e = list_begin(&&FT_list); e != list_end(&&FT_list);
			e = list_next(e)) {
		struct frame *f = list_entry(e, struct frame, elem);
		if (f->frame == frame) {
			list_remove(e);
			free(f);
			palloc_free_page(frame);
			break;
		}
	}
	lock_release(&FT_lock);
}

/* Unlocks frame F, allowing it to be evicted.
 F must be locked for use by the current process. */
/*
 void
 frame_unlock (struct frame *f)
 {
 }
 */

void update_FTable(void * frame) {
	struct frame *f = malloc(sizeof(struct frame_entry));
	f->frame = frame;
	f->thread = thread_current();
	lock_acquire(&FT_lock);
	list_push_back(&FT_list, &f->elem);
	lock_release(&FT_lock);

}

void* frame_evict (enum palloc_flags flags){
	return NULL;
}

>>>>>>> 974b840f7554baaac9dca5affde58b2af8276681
