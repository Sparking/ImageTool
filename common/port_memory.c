#include <stdlib.h>
#include "port_memory.h"

#if (!defined (_WIN32) && !defined (__linux__)) || (defined (ENABLE_MEMORY_POOL) && (ENABLE_MEMORY_POOL != 0U))

struct heap_block_link {
    struct heap_block_link *next_free_block;    /* 表中的下一块空闲块 */
    size_t block_size;                          /* 当前空闲块的大小 */
};

#define HEAP_MINIMUM_BLOCK_SIZE     0x000000002 /* 8 Bytes */
#define HEAP_BITS_PER_BYTE          8
#define HEAP_BYTE_ALIGNMENT         8           /* 8字节对齐 */
#define HEAP_BYTE_ALIGNMENT_MASK    0x000000007 /* 掩码 */
#define HEAP_STRUCTURE_SIZE                                                     \
        ((sizeof(struct heap_block_link) + ((size_t)(HEAP_BYTE_ALIGNMENT - 1))) \
            & ~(HEAP_BYTE_ALIGNMENT_MASK))

/**内存池的地址和大小**/
#if !defined (HEAP_AVAILABLE_SIZE)
#define HEAP_AVAILABLE_SIZE         0x000800000 /* 8M Bytes */
#endif

#if !defined (HEAP_ADDRESS)
static char port_memory_address[HEAP_AVAILABLE_SIZE];
#define HEAP_ADDRESS                ((void *)port_memory_address)
#endif

static struct heap_block_link heap_block_start, *heap_block_end = NULL;
static size_t heap_free_bytes_remaining = 0U;
static size_t heap_minimum_ever_free_bytes_remaining = 0U;
static size_t heap_block_allocated_bit = 0;

/* 堆初始化, 在第一次内存分配前需要先运行该函数 */
static void static_heap_init(void *pool, const size_t size)
{

    size_t address;
    size_t total_heap_size;
    unsigned char *puc_aligned_heap;
    struct heap_block_link *pfirst_free_block;

    total_heap_size = size;
    address = (size_t)pool;
    if ((address & HEAP_BYTE_ALIGNMENT_MASK) != 0) {
        address += HEAP_BYTE_ALIGNMENT - 1;
        address &= ~((size_t)HEAP_BYTE_ALIGNMENT_MASK);
        total_heap_size -= address - (size_t)pool;
    }

    puc_aligned_heap = (unsigned char *)address;
    heap_block_start.next_free_block = (void *)puc_aligned_heap;
    heap_block_start.block_size = (size_t)0;

    address = ((size_t)puc_aligned_heap) + total_heap_size;
    address -= HEAP_STRUCTURE_SIZE;
    address &= ~((size_t)HEAP_BYTE_ALIGNMENT_MASK);
    heap_block_end = (struct heap_block_link *)address;
    heap_block_end->block_size = 0;
    heap_block_end->next_free_block = NULL;

    pfirst_free_block = (void *)puc_aligned_heap;
    pfirst_free_block->block_size = address - (size_t)pfirst_free_block;
    pfirst_free_block->next_free_block = heap_block_end;

    heap_minimum_ever_free_bytes_remaining = pfirst_free_block->block_size;
    heap_free_bytes_remaining = pfirst_free_block->block_size;
    heap_block_allocated_bit = ((size_t)1) << ((sizeof(size_t) * HEAP_BITS_PER_BYTE) - 1);
}

/* 向空闲列表插入空闲块 */
static void static_insert_block_into_free_list(struct heap_block_link *block)
{
    struct heap_block_link *pit;
    unsigned char *puc;

    for (pit = &heap_block_start; pit->next_free_block < block; pit = pit->next_free_block) {
        continue;
    }

    puc = (unsigned char *)pit;
    if ((puc + pit->block_size) == (unsigned char *)block) {
        pit->block_size += block->block_size;
        block = pit;
    }

    puc = (unsigned char *)block;
    if ((puc + block->block_size) == (unsigned char *)pit->next_free_block) {
        if (pit->next_free_block != heap_block_end) {
            block->block_size += pit->next_free_block->block_size;
            block->next_free_block = pit->next_free_block->next_free_block;
        } else {
            block->next_free_block = heap_block_end;
        }
    } else {
        block->next_free_block = pit->next_free_block;
    }

    if (pit != block) {
        pit->next_free_block = block;
    }
}

void *mem_alloc(size_t size)
{
    struct heap_block_link *pblock, *pprev_block, *pnew_block_link;
    void *pret = NULL;

    /* */
    // suspend all tasks
    do {
        if (heap_block_end == NULL)
            static_heap_init(HEAP_ADDRESS, HEAP_AVAILABLE_SIZE);

        if ((size & heap_block_allocated_bit) == 0) {
            if (size > 0) {
                size += HEAP_STRUCTURE_SIZE;

                if ((size & HEAP_BYTE_ALIGNMENT_MASK) != 0x00)
                    size += (HEAP_BYTE_ALIGNMENT - (size & HEAP_BYTE_ALIGNMENT_MASK));
            }

            if ((size > 0) && (size <= heap_free_bytes_remaining)) {
                pprev_block = &heap_block_start;
                pblock = heap_block_start.next_free_block;
                while ((pblock->block_size < size) && (pblock->next_free_block != NULL)) {
                    pprev_block = pblock;
                    pblock = pblock->next_free_block;
                }

                if (pblock != heap_block_end) {
                    pret = (void *)(((unsigned char *)pprev_block->next_free_block) + HEAP_STRUCTURE_SIZE);
                    pprev_block->next_free_block = pblock->next_free_block;

                    if ((pblock->block_size - size) > HEAP_MINIMUM_BLOCK_SIZE) {
                        pnew_block_link = (void *)(((unsigned char *)pblock) + size);
                        pnew_block_link->block_size = pblock->block_size - size;
                        pblock->block_size = size;
                        static_insert_block_into_free_list(pnew_block_link);
                    }

                    heap_free_bytes_remaining -= pblock->block_size;
                    if (heap_free_bytes_remaining < heap_minimum_ever_free_bytes_remaining) {
                        heap_minimum_ever_free_bytes_remaining = heap_free_bytes_remaining;
                    }

                    pblock->block_size |= heap_block_allocated_bit;
                    pblock->next_free_block = NULL;
                }
            }
        }
    } while (0);
    // resume all tasks

    return pret;
}

void mem_free(void *pv)
{
    unsigned char *puc = (unsigned char *)pv;
    struct heap_block_link *plink;

    if (pv != NULL) {
        puc -= HEAP_STRUCTURE_SIZE;
        plink = (void *)puc;

        if ((plink->block_size & heap_block_allocated_bit) != 0) {
            if (plink->next_free_block == NULL) {
                plink->block_size &= ~heap_block_allocated_bit;
                // suspend all tasks
                heap_free_bytes_remaining += plink->block_size;
                static_insert_block_into_free_list(plink);
                // resume all tasks
            }
        }
    }
}

size_t mem_avaiable_size(void)
{
    return heap_free_bytes_remaining;
}

#endif
