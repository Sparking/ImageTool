#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if (!defined (_WIN32) && !defined (__linux__)) || (defined (ENABLE_MEMORY_POOL) && (ENABLE_MEMORY_POOL != 0U))

/* 从内存池中申请出一片内存 */
extern void *mem_alloc(size_t size);

/* 释放内存池中的内存 */
extern void mem_free(void *pv);

/* 获取内存池剩余空间 */
extern size_t mem_avaiable_size(void);

#else

#include <stdlib.h>

/* 从内存池中申请出一片内存 */
static inline void *mem_alloc(size_t size)
{
    return malloc(size);
}

/* 释放内存池中的内存 */
static inline void mem_free(void *pv)
{
    if (pv != NULL) {
        free(pv);
    }
}

/* 获取内存池剩余空间 */
static inline size_t mem_avaiable_size(void)
{
    return 0;
}

#endif

#ifdef __cplusplus
}
#endif
