/* SPDX-License-Identifier: BSD-3-Clause */

#pragma once

#include <errno.h>
#include <stdio.h>
#include "printf.h"

#define MMAP_THRESHOLD      (128 * 1024)

#define DIE(assertion, call_description)									\
	do {													\
		if (assertion) {										\
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);					\
			perror(call_description);								\
			exit(errno);										\
		}												\
	} while (0)

/* Structure to hold memory block metadata */
typedef struct block_meta block_meta;
struct block_meta {
	size_t size;
	int status;
	block_meta *prev;
	block_meta *next;
};

extern struct block_meta *head;
extern struct block_meta *tail;

void bm_list_init_mmap(block_meta *item);
void bm_list_init_sbrk(block_meta *item);
void bm_list_init_sbrk2(block_meta *item);
void bm_list_add_mmap(block_meta *item, size_t size);
void bm_list_add_mid(block_meta *item, size_t size);
void bm_list_add_end(block_meta *item, size_t size);
void bm_list_add_sbrk(block_meta *item, size_t size);
block_meta *bm_list_find_size(size_t size);
int sbrk_verif_list();
block_meta *bm_list_find(block_meta *item);
static block_meta *bm_list_extract(block_meta *item);

/* Block metadata status values */
#define STATUS_FREE   0
#define STATUS_ALLOC  1
#define STATUS_MAPPED 2

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define SIZE_T_SIZE (ALIGN(sizeof(block_meta)))