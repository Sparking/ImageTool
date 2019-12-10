#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IMAGE_RFEDGE_TYPE_NONE = 0,   // 平坦区域
    IMAGE_RFEDGE_TYPE_RAISE = 1,   // 上升边界
    IMAGE_RFEDGE_TYPE_FALL = 2,   // 下降边界
} image_raise_fall_edge_type;

struct image_raise_fall_edge {
	int len;
	int begin_pos;
	image_raise_fall_edge_type type;
	unsigned char max_grad, min_grad;
	unsigned char amplitude;
};

extern unsigned int image_find_raise_fall_edges(const unsigned char *imgdata,
		const unsigned int len, struct image_raise_fall_edge *pedge, const unsigned int num);

#ifdef __cplusplus
}
#endif
