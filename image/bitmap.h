#pragma once

#include <stdint.h>

/* BMP头部两个字节, 小端格式 */
#define BMP_FILE_TYPE   0x4D42  /* 'B' 'M' */

/* 结构体1字节对齐 */
#pragma  pack(push, 1)
/* bitmap-file header */
struct bmp_header {
    uint16_t bf_type;       /* BM */
    uint32_t bf_size;
    uint32_t bf_reserved;
    uint32_t bf_data_offset;/* data offset */
};

/* bitmap-information header */
struct bmp_info_header {
    uint32_t bi_size;
    int32_t  bi_width;
    int32_t  bi_height;     /* 可为正负数 */
    uint16_t bi_planes;
    uint16_t bi_bit_count;
    uint32_t bi_compression;
    uint32_t bi_size_image;
    uint32_t bi_X_pels_meter;
    uint32_t bi_Y_pels_meter;
    uint32_t bi_color_used;
    uint32_t bi_clr_important;
};

/* 调色板 */
struct bmp_color_table {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
};

struct bitmap_image {
    struct bmp_header bh;
    struct bmp_info_header bih;
    struct bmp_color_table *bct;
    unsigned char *data;

    int32_t actual_height;        /* 图像的实际高度 */
    uint8_t dummy_line_bytes;   /* 图形每行数据的补齐字节数 */
};
#pragma pack(pop)

/**
 * @brief 结构体内存释放
 */
extern void bitmap_image_release(struct bitmap_image *img);

/**
 * @brief bitmap_image_create 创建一张空白的bmp图像
 * @brief height 图像的高度
 * @brief width 图像的宽度
 * @brief bit_counts 图像的位数
 * @brief gray 默认的图像颜色
 */
extern struct bitmap_image *bitmap_image_create(const int32_t height, const int32_t width,
        const uint32_t bit_counts, const uint8_t gray);

/**
 * @brief bitmap_image_open create bmp data from file
 * @param filename BMP file
 */
extern struct bitmap_image *bitmap_image_open(const char *filename);

/**
 * @brief bitmap_image_save 保存图像为指定的文件
 * @param filename 要保存的文件路径
 * @param image 图像数据
 * @return 保存成功返回0, 失败返回-1
 */
extern int bitmap_image_save(const char *filename, const struct bitmap_image *image);
