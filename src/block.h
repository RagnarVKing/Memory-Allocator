/* SPDX-License-Identifier: BSD-3-Clause */

#pragma once

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "printf.h"
#include "block_meta.h"

#define MMAP_THRESHOLD      (128 * 1024)

extern struct block_meta *head;
extern struct block_meta *tail;

void bm_list_init_mmap(struct block_meta *item);
void bm_list_init_sbrk(struct block_meta *item);
void bm_list_init_sbrk2(struct block_meta *item);
void bm_list_add_mmap(struct block_meta *item);
void bm_list_add_mid(struct block_meta *item, size_t size);
void bm_list_add_end(struct block_meta *item);
struct block_meta *bm_list_find_size(size_t size);


#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define SIZE_T_SIZE (ALIGN(sizeof(struct block_meta)))
