#pragma once

#include "maths.h"
#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

struct qr_position_makrings_info {
    struct point center;
    struct point conner_points[4];
    unsigned int wx;
    unsigned int wy;
    unsigned int w45;
    unsigned int w135;
    float module_size;
};

extern int qr_position_makrings_find(const struct image *img,
        struct qr_position_makrings_info *pqpmi, const unsigned int sz);

#ifdef __cplusplus
}
#endif
