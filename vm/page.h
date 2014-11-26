#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

// 256 KB
#define STACK_SIZE (1 << 18)


enum page_flags
  {
    FILE = 001,           /* for file pages */
    SWAP = 002,             /* swapped out pages*/
    MMAP = 004              /* page in memory*/
  };

/* Supplementary page table. */
struct page {
	void *u_vaddr;
	bool write;
	bool is_loaded;
	enum page_flags page_flag;

// For files
	struct file *file;
	size_t file_offset;
	size_t read_bytes;
	size_t zero_bytes;

// For swap
	size_t swap_index;
	struct hash_elem elem;
};

void page_table_init (struct hash *pt);
void page_exit(struct hash *pt);

struct page *page_allocate(void *, bool read_only);
void page_deallocate(void *vaddr);

bool page_in(struct page *);
bool page_out(struct page *);
bool page_accessed_recently(struct page *);
struct page * page_for_addr(const void *address);

bool page_lock(const void *, bool will_write);
void page_unlock(const void *);

bool load_file(struct page *);
bool load_swap(struct page *);
bool load_mmap(struct page *);

bool add_file_to_page_table(struct file *file, int32_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes,
		bool writable);

bool add_mmap_to_page_table(struct file *file, int32_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes);


bool stack_grow (void *);

hash_hash_func page_hash;
hash_less_func page_less;

#endif /* vm/page.h */
