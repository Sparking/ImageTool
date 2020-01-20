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
};

struct dotcode_dot {
    struct dotcode_point dot;
    struct list_head node45;    /**表头:struct dotcode_line_node -> pt(45°线)**/
    struct list_head node135;   /**表头:struct dotcode_line_node -> pt(135°线)**/
    struct list_head *head45;
    struct list_head *head135;
};

struct dotcode_line_node {
    int dir;
    int ndt;
    int index;
    int min_len;
    struct list_head *head;
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

extern unsigned int image_find_raise_fall_edges_by_offset_dotcode(
	const struct image *img, const struct point *pstart,
	const struct point *setup_off, const unsigned int len,
	struct image_raise_fall_edge *pedge, const unsigned int num);

extern int image_find_dot_by_grad(const struct image *img);

#ifdef __cplusplus
}
#endif
