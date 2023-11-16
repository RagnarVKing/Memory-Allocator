// SPDX-License-Identifier: BSD-3-Clause

#include "block.h"


struct block_meta *head;

struct block_meta *tail;

void bm_list_init_mmap(struct block_meta *item)
{
	head = item;
	head->prev = head;
	head->next = head;
}

void bm_list_init_sbrk(struct block_meta *item)
{
	head = item;


	if ((item->size + SIZE_T_SIZE + SIZE_T_SIZE + ALIGNMENT) >= MMAP_THRESHOLD) {
		head->prev = head;
		head->next = head;
		item->size += MMAP_THRESHOLD - item->size - SIZE_T_SIZE;

		tail = head;
	} else {
		struct block_meta *fre = (struct block_meta *)((char *)item + item->size + SIZE_T_SIZE);

		fre->size = MMAP_THRESHOLD - item->size - SIZE_T_SIZE - SIZE_T_SIZE;
		fre->status = STATUS_FREE;

		fre->prev = head;
		fre->next = head;
		head->next = fre;
		head->prev = fre;

		tail = fre;
	}
}

void bm_list_init_sbrk2(struct block_meta *item)
{
	tail = item;

	if ((item->size + SIZE_T_SIZE + SIZE_T_SIZE + ALIGNMENT) >= MMAP_THRESHOLD) {
		tail->next = head;
		tail->prev = head->prev;
		head->prev->next = tail;
		head->prev = tail;
		item->size += MMAP_THRESHOLD - item->size - SIZE_T_SIZE - SIZE_T_SIZE;
		head = tail;
	} else {
		struct block_meta *fre = (struct block_meta *)((char *)item + item->size + SIZE_T_SIZE);

		fre->size = MMAP_THRESHOLD - item->size -  SIZE_T_SIZE - SIZE_T_SIZE;
		fre->status = STATUS_ALLOC;

		fre->next = head;
		fre->prev = tail;

		tail->next = fre;
		tail->prev = head->prev;

		head->prev->next = tail;
		head->prev = fre;

		head = tail;
		tail = fre;
	}
}

void bm_list_add_mmap(struct block_meta *item)
{
	if (!item)
		return;

	item->next = head;
	item->prev = head->prev;
	head->prev->next = item;
	head->prev = item;
}

void bm_list_add_mid(struct block_meta *item, size_t size)
{
	if (!item)
		return;

	struct block_meta *fre = (struct block_meta *)((char *)item + size + SIZE_T_SIZE);

	fre->size = item->size - size - SIZE_T_SIZE;
	fre->status = STATUS_FREE;

	fre->prev = item;
	fre->next = item->next;
	item->next->prev = fre;
	item->next = fre;

	if (item == tail)
		tail = fre;
	item->size = size;
}

void bm_list_add_end(struct block_meta *item)
{
	if (!item)
		return;

	item->next = tail->next;
	item->prev = tail;
	tail->next->prev = item;
	tail->next = item;
	tail = item;
}

struct block_meta *bm_list_find_size(size_t size)
{
	struct block_meta *iter;
	struct block_meta *item = NULL;
	size_t best_size = MMAP_THRESHOLD;

	if (head == head->next)
		if (head->size >= size && head->status == STATUS_FREE)
			return head;

	if (head->size >= size && head->status == STATUS_FREE) {
		item = head;
		best_size = head->size;
	}

	for (iter = head->next; iter != head; iter = iter->next)
		if (iter->size >= size && iter->status == STATUS_FREE)
			if (iter->size < best_size) {
				item = iter;
				best_size = iter->size;
			}

	return item;
}
