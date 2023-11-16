// SPDX-License-Identifier: BSD-3-Clause

#include "osmem.h"
#include "block.h"

struct block_meta *mmap_first(size_t size)
{
	struct block_meta *item = mmap(0, size + SIZE_T_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	DIE(item == MAP_FAILED, "Mmap failed");

	item->size = size;
	item->status = STATUS_MAPPED;
	item->next = NULL;
	item->prev = NULL;

	bm_list_init_mmap(item);
	return (item + 1);
}


struct block_meta *sbrk_first(size_t size)
{
	struct block_meta *item = sbrk(MMAP_THRESHOLD);

	DIE(item == (void *)-1, "Sbrk failed");

	item->size = size;
	item->status = STATUS_ALLOC;
	item->next = NULL;
	item->prev = NULL;

	bm_list_init_sbrk(item);
	return (item + 1);
}

struct block_meta *sbrk_second(size_t size)
{
	struct block_meta *item = sbrk(MMAP_THRESHOLD);

	DIE(item == (void *)-1, "Sbrk failed");

	item->size = size;
	item->status = STATUS_ALLOC;
	item->next = NULL;
	item->prev = NULL;

	bm_list_init_sbrk2(item);
	return(item + 1);
}


struct block_meta *mmap_last(size_t size)
{
	struct block_meta *item = mmap(0, size + SIZE_T_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	DIE(item == MAP_FAILED, "Mmap failed");

	item->size = size;
	item->status = STATUS_MAPPED;
	item->next = NULL;
	item->prev = NULL;

	bm_list_add_mmap(item);
	return (item + 1);
}


struct block_meta *sbrk_last_verif(struct block_meta *item, size_t size)
{
	if (item->size == size || item->size < size + SIZE_T_SIZE + ALIGNMENT) {
		item->status = STATUS_ALLOC;
		return (item + 1);
	}
	item->status = STATUS_ALLOC;
	bm_list_add_mid(item, size);
	return (item + 1);
}

struct block_meta *sbrk_last_final(size_t size)
{
	if (tail->status == STATUS_FREE) {
		struct block_meta *item = sbrk(size - tail->size);

		DIE(item == (void *)-1, "Sbrk failed");

		tail->size = size;
		tail->status = STATUS_ALLOC;

		return (tail + 1);
	}
	struct block_meta *item = sbrk(size + SIZE_T_SIZE);

	DIE(item == (void *)-1, "Sbrk failed");

	item->size = size;
	item->status = STATUS_ALLOC;
	item->next = NULL;
	item->prev = NULL;

	bm_list_add_end(item);
	return (item + 1);
}

struct block_meta *sbrk_last(size_t size)
{
	struct block_meta *item = bm_list_find_size(size);

	if (!item)
		return sbrk_last_final(size);
	else
		return sbrk_last_verif(item, size);
}





void *os_malloc(size_t size)
{
	/* TODO: Implement os_malloc */
	if (!size)
		return NULL;

	size_t total_size = SIZE_T_SIZE + ALIGN(size);

	if (total_size >= MMAP_THRESHOLD) {
		if (!head)
			return mmap_first(ALIGN(size));
		else
			return mmap_last(ALIGN(size));
	} else {
		if (!head)
			return sbrk_first(ALIGN(size));
		else if (!tail)
			return sbrk_second(ALIGN(size));
		else
			return sbrk_last(ALIGN(size));
	}
}




void free_first(struct block_meta *item)
{
	item->status = STATUS_FREE;
	item->next = item;
	item->prev = item;
}


void free_last_mmap(struct block_meta *item)
{
	item->prev->next = item->next;
	item->next->prev = item->prev;
	munmap(item, item->size + SIZE_T_SIZE);
}


void free_right(struct block_meta *item)
{
	item->size += item->next->size + SIZE_T_SIZE;
	item->status = STATUS_FREE;

	item->next->next->prev = item;
	item->next = item->next->next;
}

void free_left(struct block_meta *item)
{
	item->prev->size += SIZE_T_SIZE + item->size;

	item->next->prev = item->prev;
	item->prev->next = item->next;
}

void free_triple(struct block_meta *item)
{
	item->prev->size += SIZE_T_SIZE + item->size + SIZE_T_SIZE + item->next->size;

	item->next->next->prev = item->prev;
	item->prev->next = item->next->next;
}

void free_zero(struct block_meta *item)
{
	item->status = STATUS_FREE;
}

void free_last_sbrk(struct block_meta *item)
{
	if (item == head) {
		if (item->next->status == STATUS_FREE)
			free_right(item);
		else
			free_zero(item);
	} else if (item == tail) {
		if (item->prev->status == STATUS_FREE) {
			tail = item->prev;
			free_left(item);
		} else {
			free_zero(item);
		}
	} else {
		if (item->prev->status == STATUS_FREE && item->next->status == STATUS_FREE) {
			if (item->next == tail)
				tail = item->prev;
			free_triple(item);
		} else if (item->prev->status == STATUS_FREE) {
			free_left(item);
		} else if (item->next->status == STATUS_FREE) {
			free_right(item);
		} else {
			free_zero(item);
		}
	}
}



void os_free(void *ptr)
{
	/* TODO: Implement os_free */
	if (!ptr)
		return;

	struct block_meta *item = (struct block_meta *)((char *)ptr - SIZE_T_SIZE);

	if (item->status == STATUS_MAPPED) {
		if (head == head->next) {
			munmap(item, item->size + SIZE_T_SIZE);
			head = NULL;
		} else {
			free_last_mmap(item);
		}
	} else if (item->status == STATUS_ALLOC) {
		if (head == head->next)
			free_first(item);
		else
			free_last_sbrk(item);
	}
}




void *os_calloc(size_t nmemb, size_t size)
{
	/* TODO: Implement os_calloc */

	if (!(nmemb && size))
		return NULL;

	struct block_meta *item;

	size_t total_size = SIZE_T_SIZE + ALIGN(nmemb * size);

	if (total_size >= (size_t)getpagesize()) {
		if (!head) {
			item = mmap_first(ALIGN(nmemb * size));
			item = (struct block_meta *)((char *)item - SIZE_T_SIZE);
			memset((char *)item + SIZE_T_SIZE, 0, ALIGN(nmemb * size));
		} else {
			item =  mmap_last(ALIGN(nmemb * size));
			item = (struct block_meta *)((char *)item - SIZE_T_SIZE);
			memset((char *)item + SIZE_T_SIZE, 0, ALIGN(nmemb * size));
		}
	} else {
		if (!head) {
			item = sbrk_first(ALIGN(nmemb * size));
			item = (struct block_meta *)((char *)item - SIZE_T_SIZE);
			memset((char *)item + SIZE_T_SIZE, 0, ALIGN(nmemb * size));
		} else if (!tail) {
			item = sbrk_second(ALIGN(nmemb * size));
			item = (struct block_meta *)((char *)item - SIZE_T_SIZE);
			memset((char *)item + SIZE_T_SIZE, 0, ALIGN(nmemb * size));
		} else {
			item = sbrk_last(ALIGN(nmemb * size));
			item = (struct block_meta *)((char *)item - SIZE_T_SIZE);
			memset((char *)item + SIZE_T_SIZE, 0, ALIGN(nmemb * size));
		}
	}
	return (item + 1);
}


struct block_meta *realloc_simple(struct block_meta *destination, struct block_meta *item, size_t size)
{
	destination = os_malloc(size);
	if (size > item->size)
		size = item->size;
	item = (struct block_meta *)((char *)item + SIZE_T_SIZE);
	memmove(destination, item, size);
	os_free(item);
	return destination;
}

struct block_meta *realloc_split(struct block_meta *item, size_t size)
{
	struct block_meta *fre = (struct block_meta *)((char *)item + size + SIZE_T_SIZE);

	fre->size = item->size - size - SIZE_T_SIZE;
	fre->status = STATUS_FREE;
	item->size = size;

	fre->next = item->next;
	fre->prev = item;
	item->next->prev = fre;
	item->next = fre;

	if (item == tail)
		tail = fre;
	return (item + 1);
}

struct block_meta *realloc_no_split(struct block_meta *item)
{
	return (item + 1);
}

struct block_meta *condition_one(struct block_meta *item, size_t size, struct block_meta *destination)
{
	if (item->status == STATUS_ALLOC) {
		if (size > item->size) {
			if (item == tail) {
				sbrk(size - item->size);
				item->size = size;
				return (item + 1);

			} else if (item->next->status == STATUS_FREE) {
				if (item->next == tail)
					tail = item;

				item->size += SIZE_T_SIZE + item->next->size;

				item->next->next->prev = item;
				item->next = item->next->next;

				if (item->size >= size) {
					if (item->size - size < SIZE_T_SIZE + ALIGNMENT)
						return realloc_no_split(item);
					else
						return realloc_split(item, size);
					} else {
						return realloc_simple(destination, item, size);
					}
			} else {
				return realloc_simple(destination, item, size);
			}
		} else {
			if (item->size - size < SIZE_T_SIZE + ALIGNMENT)
				return (item + 1);
			else
				return realloc_split(item, size);
		}
	} else {
		return realloc_simple(destination, item, size);
	}
}


void *os_realloc(void *ptr, size_t size)
{
	/* TODO: Implement os_realloc */
	if (!ptr)
		return os_malloc(size);
	if (!size) {
		os_free(ptr);
		return NULL;
	}

	struct block_meta *item = (struct block_meta *)((char *)ptr - SIZE_T_SIZE);

	if (item->status == STATUS_FREE)
		return NULL;

	struct block_meta *destination = NULL;

	size = ALIGN(size);
	if (size == item->size)
		return ptr;

	if (size < MMAP_THRESHOLD)
		return condition_one(item, size, destination);
	else
		return realloc_simple(destination, item, size);

	return NULL;
}
