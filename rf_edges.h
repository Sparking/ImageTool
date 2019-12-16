#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "image.h"

enum {
    IMAGE_RFEDGE_TYPE_NONE = 0,   /* 平坦区域 */
    IMAGE_RFEDGE_TYPE_RAISE = 1,  /* 上升边界 */
    IMAGE_RFEDGE_TYPE_FALL = 2,   /* 下降边界 */
};

struct image_raise_fall_edge {
    unsigned int dpos;
    unsigned int dpos_256x;
    unsigned int begin;
    unsigned int end;
    unsigned char type;
    unsigned char max_grad;
    unsigned char min_grad;
    unsigned char max_gray;
    unsigned char min_gray;
    unsigned char ref_gray;
    unsigned char amplitude;
};

extern unsigned int image_find_raise_fall_edges(const unsigned char *imgdata,
        const unsigned int len, struct image_raise_fall_edge *pedge, const unsigned int num);

extern unsigned int image_find_raise_fall_edges_by_offset(
        const struct image *img, const struct point pstart,
        const struct point setup_off, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num);

#ifdef __cplusplus
}
#endif
