#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include "threads/synch.h"
#include <list.h>


/* A physical frame. */
struct frame 
{
    void *frame;         /* Mapped process page, if any. */
    /* ...............          other struct members as necessary */
    struct thread * thread;
    struct list_elem elem;
};
struct lock FT_lock;
struct list FT_list;

void frame_init (void);


struct frame *frame_alloc_and_lock (void *frame,enum palloc_flags flags);

void frame_lock (struct page *);

void frame_free (struct frame *);
void frame_unlock (struct frame *);

void update_FTable(void * frame);

#endif /* vm/frame.h */
