#pragma once

#include "image.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "bitmatrix.h"

struct qr_decode_info {
    unsigned char version;
    // 格式信息
    unsigned char error_correct_level;  /* 纠错等级 */
    unsigned char mask;                 /* 掩膜版本 */

    unsigned char *data;
    unsigned int data_size;
};

/**
 * @brief QR二维码解码初始化
 * @note 解码之前需要进行一次初始化
 * @return 0: 初始化成功, 1: 初始化失败
 */
extern int qr_decode_init(void);

/**
 * @brief 释放QR二维码解码资源
 */
extern void qr_decode_deinit(void);

/**
 * @brief qr_detect 扫描二维码, 返回码字矩阵
 * @param img 二值化图像
 * @return 返回一个二维码对应的矩阵
 */
extern struct bitmatrix *qr_detect(const struct image *img);

/**
 * @brief qr_decode 解析二维码矩阵, 返回解码信息
 */
extern struct qr_decode_info *qr_decode(struct bitmatrix *sample_image);

#ifdef __cplusplus
}
#endif

