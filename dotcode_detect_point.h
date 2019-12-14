#pragma once

#include "maths.h"
#include "image.h"
#include "port_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dotcode_point {
    struct point center;    /**点的中心点坐标**/
    unsigned char nw;       /**点的水平宽度**/
    unsigned char nh;
    unsigned char n45;
    unsigned char n135;
    unsigned char isblack;
};

/*******************************************************************************

 ******************************************************************************/
extern unsigned int dotcode_detect_point(const struct image *img,
		struct dotcode_point *bpdtp, const unsigned int nbdtp,
		struct dotcode_point *wpdtp, const unsigned int wbdtp);

#ifdef __cplusplus
}
#endif
