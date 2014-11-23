#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

struct mmap_entry {
	int map_id;
	struct page *pte;
	struct list_elem elem;
};

tid_t process_execute(const char *file_name);
int process_wait(tid_t);
void process_exit(void);
void process_activate(void);

bool install_page(void *upage, void *kpage, bool writable);

bool add_mapping (struct page *pte);
void remove_mapping (int mapping);

#endif /* userprog/process.h */
