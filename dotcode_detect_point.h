#pragma once

#include "maths.h"
#include "image.h"
#include "port_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dotcode_point {
    struct point center;    /**点的中心点坐标**/
    unsigned int nw;        /**点的水平宽度**/
    unsigned int nh;
    unsigned int n45;
    unsigned int n135;
    unsigned int weight;
    unsigned char isblack;
	unsigned char score;
};

struct dotcode_point_line {
	struct dotcode_point pt[50];
	unsigned int npt;
	unsigned int min_len;
	unsigned char is45;
};

struct dotcode_point_lineset {
	struct dotcode_point_line line[8];
	unsigned int nline;
};

/*******************************************************************************
 ******************************************************************************/
extern unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *pdtp, const unsigned int ndtp);

#ifdef __cplusplus
}
#endif
