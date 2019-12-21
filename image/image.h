#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "maths.h"
#include "bitmatrix.h"

#define IMAGE_ENABLE_BITMAP
#define IMAGE_ENABLE_JPEG
#define IMAGE_ENABLE_PNG

/* 图像的格式 */
enum {
    IMAGE_FORMAT_GRAY = 0,  /* gray, 8 bits */
    IMAGE_FORMAT_RGB,       /* Red, Green, Blue, 24 bits */
    IMAGE_FORMAT_BGR,       /* Blue, Green, Red, 24 bits */
    IMAGE_FORMAT_RGBA,      /* Red, Green, Blue, Alpha, 32 bits */
    IMAGE_FORMAT_BGRA,      /* Blue, Green, Red, Alpha, 32 bits */
};

/* 图像的文件格式 */
enum {
#ifdef IMAGE_ENABLE_BITMAP
    IMAGE_FILE_BITMAP,
#endif

#ifdef IMAGE_ENABLE_JPEG
    IMAGE_FILE_JPEG,
#endif

#ifdef IMAGE_ENABLE_PNG
    IMAGE_FILE_PNG,
#endif
};

/* 图像的原点为左上角 */
struct image {
    unsigned int height;    /* 图像的高度 */
    unsigned int width;     /* 图像的宽度 */

    unsigned int format;    /* 图像的格式 */
    unsigned int pixel_size;/* 图像像素的大小，和图像的格式有关 */
    unsigned int row_size;  /* 图像每一行数据的宽度, 单位字节 */
    unsigned int size;      /* 图像占用的内存大小 */

    unsigned char data[0];  /* 图像数据 */
};

/**
 * @brief image 打开图像文件
 */
extern struct image *image_open(const char *file);

/**
 * @brief image_create 创建空白的图像
 */
extern struct image *image_create(const unsigned int height, const unsigned int width,
        const unsigned int format);

/**
 * @brief image_create_from_bitmatrix 从bitmatrix矩阵中创建二值图像
 */
extern struct image *image_create_from_bitmatrix(const struct bitmatrix *matrix);

/**
 * @brief image_create_bitmatrix 从二值化图像中创建bitmatrix
 * @note 在bitmatrix中, 黑色设置为0, 白色设置为1
 */
extern struct bitmatrix *image_create_bitmatrix(const struct image *img);

/**
 * @brief image_save 保存图像到文件
 * @param flag 指定保存的文件格式, 可选参数有: IMAGE_FILE_JPEG, IMAGE_FILE_BITMAP
 * @return 成功返回0, 失败返回-1
 */
extern int image_save(const char *file, const struct image *img, const int flag);

/**
 * @brief image_dump 复制图像
 */
extern struct image *image_dump(const struct image *img);

/**
 * @brief image_release 释放图像所占用的内存空间
 */
extern void image_release(struct image *img);

/**
 * @brief image_convert_gray 将图像转为灰度格式
 */
extern struct image *image_convert_gray(const struct image *img);

/**
 * @brief image_convert_format 将图像转为指定格式
 */
extern struct image *image_convert_format(const struct image *src_img, const unsigned int format);

/**
 * @brief image_find_binariztion_global_threshold 找出灰度图的二值化阈值, 这里采用的是全局阈值基本阈值法
 * @note 图像的格式必须为灰度图格式
 */
extern unsigned char image_find_binariztion_global_threshold(const struct image *img);

/**
 * @brief image_gray_binarize 灰度图形二值化
 * @param img 灰度图
 * @param threshold 二值化阈值
 * @param gray 二值化后的物体颜色
 * @param bg_gray 二值化后的背景颜色
 */
extern int image_gray_binarize(struct image *img, const unsigned char threshold,
        const unsigned char gray, const unsigned bg_gray);

/**
 * @brief image_gray_blance 灰度图直方均衡化
 * @param grad 均衡化后的颜色阶数
 */
extern int image_gray_blance(struct image *image, const unsigned char grad);

/**
 * @brief image_sharpening 灰度图锐化处理
 * @param template1/template2 算子模板, 可以选择为NULL, 但不能同时为NULL
 * @param dimension 算子模板的维度
 */
extern struct image *image_sharpening(const struct image *img,
        const int *template1, const int *template2, const unsigned int dimension);

/**
 * @brief image_sobel_enhancing 灰度图锐化处理, sobel算子
 */
extern struct image *image_sobel_enhancing(const struct image *img);

/**
 * @brief image_laplace_enhancing 灰度图锐化处理, Laplace算子
 * @param flag 0: 选择模板1, 1: 选择模板2
 */
extern struct image *image_laplace_enhancing(const struct image *img, const unsigned int flag);

/**
 * @brief image_filter_median 中值滤波
 * @param m, n 指定滤波窗口, 设定值不超过15。 m指定窗口的高度(纵高), n指定窗口的宽度(横宽)
 */
extern int image_filter_median(struct image *img, const unsigned int m, const unsigned int n);

/**
 * @brief image_filter_gaussian 对图像进行高斯滤波
 */
extern int image_filter_gaussian(struct image *img, const unsigned int ksize, const float sigma);

/**
 * @brief image_canny_enhancing canny算法求出图像的边缘
 * @param gauss_ksize 高斯内核的大小
 * @param gauss_sigma 高斯核的参数
 * @param min_threshold 双阈值最小值
 * @param max_threshold 双阈值最大值
 * @return 成功返回0, 失败返回-1
 */
extern int image_canny_enhancing(struct image *img, const int gauss_ksize, const float gauss_sigma,
        const float min_threshold, const float max_threshold);

/**
 * @brief image_hough_transform hough变换画出图中的直线
 * @param img 原灰度图
 * @param threshold 阈值
 * @param gray hough变换的像素值。该变换只针对这种像素值的点做hough变换
 * @return 返回一张经过变换后的得到的直线图，黑底白线
 */
extern struct image *image_hough_transform(const struct image *img, const unsigned int threshold,
        const unsigned int gray);

/**********************************************************
* 函数: 插值函数系列
* 参数:
*   src_img: 插值使用的原图
*  new_pixel: 插值后的像素点的值，一般设置为大小为4的数组
*   x: 对应原图中的亚像素点的x坐标，列
*   y: 对应原图中的亚像素点的y坐标，行
**********************************************************/
/* 邻近插值 */
extern int image_nearest_interp(const struct image *src_img,
        unsigned char *new_pixel, const float x, const float y);

/* 双线性插值 */
extern int image_bilinear_interp(const struct image *src_img,
        unsigned char *new_pixel, const float x, const float y);

/* 双立方插值 */
extern int image_bicubic_interp(const struct image *src_img,
        unsigned char *new_pixel, const float x, const float y);

/**
 * @brief image_perspective_transform 透视变换
 * @param img 原灰度图
 * @param nh 目标图像的高度
 * @param nw 目标图像的宽度
 * @param src_point 在原灰度图中的点
 * @param dst_point 变换后的点
 * @return 返回一张经过透视变换后图
 */
extern struct image *image_perspective_transform(const struct image *img,
        const unsigned int nh, const unsigned int nw,
        const struct point src_point[4], const struct point dst_point[4]);

/**********************************************************
 * 参数: 这里图片的坐标原点设置为左上方的顶点
 *  rotation_center: 旋转中心点
 *  offset: 旋转后的图像偏移位置
 *      偏移量 > 0时, 图像向右/向下平移
 *      偏移量 < 0时, 图像向左/向上平移
 *  theta: 旋转的角度
 *      theta < 0时, 图像顺时针旋转
 *      theta > 0时, 图像逆时针旋转
 * interp: 插值函数
 **********************************************************/
extern struct image *image_rotation(const struct image *src_img, const struct point *rotation_center,
        const struct point *offset, const float theta,
        int (*interp)(const struct image *, unsigned char *, const float, const float));


extern void img_print_point(struct image *img, const unsigned int x, const unsigned int y,
    const unsigned char *color, const unsigned int size);

#ifdef __cplusplus
}
#endif
