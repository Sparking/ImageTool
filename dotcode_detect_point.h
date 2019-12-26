#pragma once

#include "list.h"
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
	struct list_head node45;    /**表头:struct dotcode_line_node -> pt(45°线)**/
	struct list_head node135;   /**表头:struct dotcode_line_node -> pt(135°线)**/
};

struct dotcode_line_node {
	int index;
	int min_len;
	struct list_head node;  /**表头:struct list_head(45°线/135°线)**/
	struct list_head pt;    /**数据:struct dotcode_point -> node45/node135**/
};

struct dotcode_line {
	struct list_head line45;
	struct list_head line135;
};

/*******************************************************************************
 ******************************************************************************/
extern unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *pdtp, const unsigned int ndtp);

#ifdef __cplusplus
}
#endif
