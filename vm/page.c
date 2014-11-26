#include "vm/page.h"
#include <stdio.h>
#include <string.h>
#include "vm/frame.h"
#include "vm/swap.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

/* Maximum size of process stack, in bytes. */
#define STACK_MAX (1024 * 1024)

/*initializes the supplementary page table*/
void page_table_init(struct hash *pt) {
	hash_init(pt, page_hash, page_less, NULL);
}

/* Destroys a page, which must be in the current process's
 page table.  Used as a callback for hash_destroy(). */
static void destroy_page(struct hash_elem *p_, void *aux UNUSED) {
	struct page *page = hash_entry(p_, struct page, elem);
	free(page);
}

/*deletes each entry of the page table*/
/*static void page_delete (struct hash_elem *e, void *aux UNUSED)
 {
 struct page *page = hash_entry(e, struct page,elem);
 if (page->is_loaded)
 {
 frame_free(pagedir_get_page(thread_current()->pagedir, page->u_vaddr));
 //remove the entry from the page dir
 pagedir_clear_page(thread_current()->pagedir, page->u_vaddr);
 }
 free(page);
 }*/

/* Destroys the current process's page table. */
void page_exit(struct hash *pt) {
	hash_destroy(pt, destroy_page);
}

/* Returns the page containing the given virtual ADDRESS,
 or a null pointer if no such page exists.
 Allocates stack pages as necessary. */
struct page * page_for_addr(const void *address) {
	struct page pte;
	pte.u_vaddr = pg_round_down(address);
	struct hash_elem *e = hash_find(&thread_current()->page_table, &pte.elem);
	if (!e) {
		return NULL;
	}
	return hash_entry(e, struct page, elem);
}

/* Locks a frame for page P and pages it in.
 Returns true if successful, false on failure. */
static bool do_page_in(struct page *p) {

}

/* Faults in the page containing FAULT_ADDR.
 Returns true if successful, false on failure. */
bool page_in(void *fault_addr) {
	struct page *pte = page_for_addr(fault_addr);
	if (!pte) {
		return false;
	}
	bool success = false;
	switch (pte->page_flag) {
	case FILE:
		success = load_file(pte);
		break;
	case SWAP:
		success = load_swap(pte);
		break;
	case MMAP:
		success = load_file(pte);
		break;
	}
	return success;
}

bool load_file(struct page *pte) {
//	void* addr = pagedir_get_page(thread_current()->pagedir, pte->u_vaddr);
	uint8_t *frame = frame_alloc(PAL_USER);
	if (!frame) {
		return false;
	}
	if ((int) pte->read_bytes
			!= file_read_at(pte->file, frame, pte->read_bytes,
					pte->file_offset)) {
		frame_free(frame);
		return false;
	}
	memset(frame + pte->read_bytes, 0, pte->zero_bytes);
	if (!install_page(pte->u_vaddr, frame, pte->write)) {
		frame_free(frame);
		return false;
	}
	// Set frame->pte = spte
	pte->is_loaded = true;
	return true;
}
bool load_swap(struct page *pte) {

	uint8_t *frame = frame_alloc(PAL_USER);
	if (!frame) {
		return false;
	}
	swap_in(pte->swap_index, frame);
	if (!install_page(pte->u_vaddr, frame, pte->write)) {
		frame_free(frame);
		return false;
	}
	pte->is_loaded = true;
	return true;
}
bool load_mmap(struct page *pte) {
	return false;
}
/* Evicts page P.
 P must have a locked frame.
 Return true if successful, false on failure. */
bool page_out(struct page *p) {
}

/* Returns true if page P's data has been accessed recently,
 false otherwise.
 P must have a frame locked into memory. */
bool page_accessed_recently(struct page *p) {
}

/* Adds a mapping for user virtual address VADDR to the page hash
 table.  Fails if VADDR is already mapped or if memory
 allocation fails. */
struct page *
page_allocate(void *vaddr, bool read_only) {
}

bool add_file_to_page_table(struct file *file, int32_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes,
		bool writable) {
	struct page *pte = malloc(sizeof(struct page));
	if (!pte) {
		return false;
	}
	pte->file = file;
	pte->file_offset = ofs;
	pte->u_vaddr = upage;
	pte->read_bytes = read_bytes;
	pte->zero_bytes = zero_bytes;
	pte->write = writable;
	pte->is_loaded = false;
	pte->page_flag = FILE;
	return (hash_insert(&thread_current()->page_table, &pte->elem) == NULL);
}

bool add_mmap_to_page_table(struct file *file, int32_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes) {
	struct page *pte = malloc(sizeof(struct page));
	if (!pte) {
		return false;
	}
	pte->file = file;
	pte->file_offset = ofs;
	pte->u_vaddr = upage;
	pte->read_bytes = read_bytes;
	pte->zero_bytes = zero_bytes;
	pte->is_loaded = false;
	pte->page_flag = MMAP;
	pte->write = true;
	if (!add_mapping(pte)) {
		free(pte);
		return false;
	}
	return (hash_insert(&thread_current()->page_table, &pte->elem) == NULL);
}

/* Evicts the page containing address VADDR
 and removes it from the page table. */
void page_deallocate(void *vaddr) {
}

/* Returns a hash value for the page that E refers to. */
unsigned page_hash(const struct hash_elem *e, void *aux UNUSED) {
	struct page *pte = hash_entry(e, struct page, elem);
	return hash_int((int) pte->u_vaddr);
}

/* Returns true if page A precedes page B. */
bool page_less(const struct hash_elem *a_, const struct hash_elem *b_,
		void *aux UNUSED) {
	struct page *pa = hash_entry(a_, struct page, elem);
	struct page *pb = hash_entry(b_, struct page, elem);
	return (pa->u_vaddr < pb->u_vaddr);
}

/* Tries to lock the page containing ADDR into physical memory.
 If WILL_WRITE is true, the page must be writeable;
 otherwise it may be read-only.
 Returns true if successful, false on failure. */
bool page_lock(const void *addr, bool will_write) {
}

/* Unlocks a page locked with page_lock(). */
void page_unlock(const void *addr) {
}

bool stack_grow(void *u_vaddr) {
	if (PHYS_BASE - pg_round_down(u_vaddr) > STACK_SIZE) {
		return false;
	}
	struct page *pte = malloc(sizeof(struct page));
	if (!pte) {
		return false;
	}
	pte->u_vaddr = pg_round_down(u_vaddr);
	pte->is_loaded = true;
	pte->page_flag = SWAP;
	uint8_t *frame = frame_alloc(PAL_USER);
	if (!frame) {
		free(pte);
		return false;
	}
	if (!install_page(pte->u_vaddr, frame, true)) {
		free(pte);
		frame_free(frame);
		return false;
	}
	return (hash_insert(&thread_current()->page_table, &pte->elem) == NULL);
}
