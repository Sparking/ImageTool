#pragma once

#include "maths.h"
#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

struct qr_position_makrings_info {
    struct point center;
    struct point conner_points[4];
};

extern int qr_position_makrings_find(const struct image *img,
        struct qr_position_makrings_info *pqpmi, const unsigned int sz);

#ifdef __cplusplus
}
#endif
