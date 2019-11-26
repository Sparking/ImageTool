#pragma once

#include "bitmatrix.h"
#include "queue.h"
#include "image.h"

struct qr_finder_pattern_center {
    unsigned int row;
    unsigned int column;
};

struct qr_finder_pattern_info {
    struct qr_finder_pattern_center left_top;
    struct qr_finder_pattern_center left_bottom;
    struct qr_finder_pattern_center right_top;
};

/**
 * @brief qr_finder_pattern_find 查找出3个定位图形的信息
 * @param img 二值化后的灰度图像
 */
extern struct qr_finder_pattern_info *qr_finder_pattern_find(const struct bitmatrix *image_bitmatrix);
