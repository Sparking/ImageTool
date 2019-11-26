#pragma once

#include "maths.h"
#include "port_memory.h"

#ifndef __cplusplus
extern "C" {
#endif

struct dotcode_point {
    struct point center;    /**点的中心点坐标**/
    unsigned char mw;       /**点的水平宽度**/
    unsigned char score;    /**好点统计得分**/
};

/*******************************************************************************
 * 参数 pseed: 种子点, 以该种子点为中心找出图中具有该点特征的其他点
 * 参数 inverse: 图形是否反相
 * 参数 pdtp: 存放dotcode点的数组首地址
 * 参数 np: 存放dotcode点的数组的大小
 * 返回值: 返回找到的点的个数, 如果没有找到将返回0
 ******************************************************************************/
extern unsigned int dotcode_detect_point(const struct image *img,
        const struct point *pseed, const unsigned int inverse,
        struct dotcode_point *pdtp, const unsigned int ndtp);

#ifndef __cplusplus
}
#endif
