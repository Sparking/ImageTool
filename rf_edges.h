#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IMAGE_RFEDGE_TYPE_NONE = 0,   // 平坦区域
    IMAGE_RFEDGE_TYPE_RAISE = 1,   // 上升边界
    IMAGE_RFEDGE_TYPE_FALL = 2,   // 下降边界
} image_raise_fall_edge_type;

#define IMAGE_RFEDGE_FLAG_SURE  0   // 确定型边界
#define IMAGE_RFEDGE_FLAG_MN    1   // 疑似平坦区域
#define IMAGE_RFEDGE_FLAG_MR    2   // 疑似上升边界
#define IMAGE_RFEDGE_FLAG_MF    3   // 疑似下降边界

#if 0
struct image_raise_fall_edge {
    unsigned char type : 4;         /* 0: 平坦边界, 1: 增边界, 2: 减边界 */
    unsigned char type_flag : 4;    /* 0: 确定型边界, 1: 疑似平坦边界, 2: 疑似增边界, 3: 疑似减边界 */
    unsigned char max_gray;
    unsigned char min_gray;

    /* 毛刺信息 */
    unsigned char burr_base;        /* 毛刺基准 */
    unsigned char burr_t;           /* 毛刺阈值 */
    unsigned char max_burr;         /* 最大的毛刺幅度 */
    unsigned char min_burr;         /* 最小的毛刺幅度 */
    unsigned char burr_cnt;         /* 毛刺数量 */
    unsigned short len;
    unsigned int begin_pos;
};
#else
struct image_raise_fall_edge {
	unsigned char burr_base;
	short len;
	image_raise_fall_edge_type type;
	int begin_pos;
	unsigned char max_gray, min_gray;
	unsigned char grad;
};
#endif

extern unsigned int image_find_raise_fall_edges(const unsigned char *imgdata, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num);

#ifdef __cplusplus
}
#endif
