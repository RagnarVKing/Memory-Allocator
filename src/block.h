/* SPDX-License-Identifier: BSD-3-Clause */

#pragma once

#include "printf.h"
#include "block_meta.h"
#include "osmem.h"

#define MMAP_THRESHOLD      (128 * 1024)

extern struct block_meta *head;
extern struct block_meta *tail;

void bm_list_init_mmap(block_meta *item);
void bm_list_init_sbrk(block_meta *item);
void bm_list_init_sbrk2(block_meta *item);
void bm_list_add_mmap(block_meta *item);
void bm_list_add_mid(block_meta *item, size_t size);
void bm_list_add_end(block_meta *item);
block_meta *bm_list_find_size(size_t size);


#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define SIZE_T_SIZE (ALIGN(sizeof(block_meta)))
