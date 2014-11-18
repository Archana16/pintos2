#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include "threads/synch.h"
#include "threads/palloc.h"

/* A physical frame. */
struct frame {
void *frame;
struct thread* thread;
uint32_t *pte;
struct list_elem elem;
};

struct list F_Table;
struct lock F_lock;

void frame_init (void);

void* frame_alloc(enum palloc_flags);

void frame_lock (struct page *);

void frame_free(void *frame);

void frame_unlock (struct frame *);

void Update_Ftable(void *frame);

bool frame_evict(void *frame);

#endif /* vm/frame.h */
