#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port_memory.h"
#include "image.h"
#include "maths.h"
#include "stack.h"
#include "queue.h"

#ifdef IMAGE_ENABLE_PNG
#include <png.h>
#endif
#ifdef IMAGE_ENABLE_JPEG
#include <jpeglib.h>
#endif

#ifdef IMAGE_ENABLE_BITMAP
#include <stdint.h>

/* BMP头部两个字节, 小端格式 */
#define BMP_FILE_TYPE   0x4D42  /* 'B' 'M' */

/* 结构体1字节对齐 */
#pragma  pack(push, 1)
/* bitmap-file header */
struct bitmap_header {
    uint16_t bf_type;       /* BM */
    uint32_t bf_size;
    uint32_t bf_reserved;
    uint32_t bf_data_offset;/* data offset */
};

/* bitmap-information header */
struct bitmap_info_header {
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
struct bitmap_color_table {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
};

struct bitmap_image {
    struct bitmap_header bh;
    struct bitmap_info_header bih;
    struct bitmap_color_table *bct;
    unsigned char *data;

    int32_t actual_height;        /* 图像的实际高度 */
    uint8_t dummy_line_bytes;   /* 图形每行数据的补齐字节数 */
};
#pragma pack(pop)

static uint8_t bitmap_line_dummy_bytes(const uint32_t length, const uint32_t bit_count)
{
    return ((4 - (length * (bit_count >> 3))) & 3) & 3;
}

static void bitmap_init_256_color_table(struct bitmap_color_table pbct[256])
{
    for (uint16_t i = 0; i < 256; ++i) {
        pbct[i].blue = pbct[i].green = pbct[i].red = (uint8_t)i;
        pbct[i].reserved = 0;
    }
}

/**
 * @brief bitmap_file_count_size_image 计算位图数据+调色板的实际大小
 * @param width 一行数据的长度
 * @param height 数据的行数
 * @param bit_count 每个数据的长度(bit)
 */
static uint32_t bitmap_file_count_size_image(const uint32_t width, const uint32_t height, const uint32_t bit_count)
{
    const uint32_t dummy_line_bytes = bitmap_line_dummy_bytes(width, bit_count);
    uint32_t bitmap_size_image;

    switch (bit_count) {
    case 1:
        bitmap_size_image = (sizeof(struct bitmap_color_table) << 1) + ((width >> 3) + dummy_line_bytes) * height;
        break;
    case 4:
        bitmap_size_image = (sizeof(struct bitmap_color_table) << 4) + ((width >> 1) + dummy_line_bytes) * height;
        break;
    case 8:
        bitmap_size_image = (sizeof(struct bitmap_color_table) << 8) + (width + dummy_line_bytes) * height;
        break;
    default:
        bitmap_size_image = (width * (bit_count >> 3) + dummy_line_bytes) * height;
        break;
    }

    return bitmap_size_image;
}

/**
 * @brief bitmap_file_count_size 计算位图文件的实际大小
 * @param width 一行数据的长度
 * @param height 数据的行数
 * @param bit_count 每个数据的长度(bit)
 */
static uint32_t bitmap_file_count_size(const uint32_t width, const uint32_t height, const uint32_t bit_count)
{
    uint32_t bitmap_file_size;

    bitmap_file_size = sizeof(struct bitmap_header) + sizeof(struct bitmap_info_header)
        + bitmap_file_count_size_image(width, height, bit_count);

    return bitmap_file_size;
}

static void bitmap_image_release(struct bitmap_image *img)
{
    if (img != NULL) {
        if (img->bct != NULL) {
            mem_free(img->bct);
            img->bct = NULL;
        }

        if (img->data != NULL) {
            mem_free(img->data);
            img->data = NULL;
        }
        mem_free(img);
    }
}

static struct bitmap_image *bitmap_image_create(const int32_t height, const int32_t width,
        const uint32_t bit_counts, const uint8_t gray)
{
    struct bitmap_image *image;
    const uint8_t data_size = bit_counts >> 3;

    image = (struct bitmap_image *)mem_alloc(sizeof(struct bitmap_image));
    if (image == NULL)
        return NULL;

    memset(image, 0, sizeof(struct bitmap_image));
    image->bh.bf_type = BMP_FILE_TYPE;
    image->bh.bf_size = bitmap_file_count_size(width, height, bit_counts);
    image->bh.bf_data_offset = sizeof(struct bitmap_header)
            + sizeof(struct bitmap_info_header);
    image->bih.bi_size = sizeof(struct bitmap_info_header);
    image->bih.bi_height = height;
    image->bih.bi_width = width;
    image->bih.bi_planes = 1;
    image->bih.bi_bit_count = bit_counts;
    image->bih.bi_size_image = bitmap_file_count_size_image(width, height, bit_counts);
    image->dummy_line_bytes = bitmap_line_dummy_bytes(width, bit_counts);
    image->actual_height = height < 0 ? -height : height;
    image->data = (unsigned char *)mem_alloc(image->actual_height * image->bih.bi_width * data_size);
    if (image->data == NULL) {
        mem_free(image);
        return NULL;
    }

    image->bct = NULL;
    if (image->bih.bi_bit_count == 8) {
        image->bct = (struct bitmap_color_table *)mem_alloc(sizeof(struct bitmap_color_table) << 8);
        if (image->bct == NULL) {
            mem_free(image->data);
            mem_free(image);
            return NULL;
        }

        bitmap_init_256_color_table(image->bct);
        image->bh.bf_data_offset += sizeof(struct bitmap_color_table) << 8;
    }
    memset(image->data, gray, image->actual_height * image->bih.bi_width * data_size);

    return image;
}

static struct bitmap_image *bitmap_image_open(const char *filename)
{
    int ret;
    FILE *fp;
    int32_t y;
    uint32_t x;
    uint32_t line_size;
    uint8_t data_size;
    unsigned char *pdata;
    struct bitmap_image *image;

    if (filename == NULL)
        return NULL;

    fp = fopen(filename, "rb");
    if (fp == NULL)
        return NULL;

    ret = -1;
    image = (struct bitmap_image *)mem_alloc(sizeof(struct bitmap_image));
    if (image == NULL)
        goto release_resource;

    memset(image, 0, sizeof(struct bitmap_image));
    if (fread(&image->bh, sizeof(image->bh), 1, fp) != 1)
        goto release_resource;

    if (image->bh.bf_type != BMP_FILE_TYPE)
        goto release_resource;

    if (fread(&image->bih, sizeof(image->bih), 1, fp) != 1)
        goto release_resource;

    data_size = image->bih.bi_bit_count >> 3;
    line_size = image->bih.bi_width * data_size;
    image->actual_height = image->bih.bi_height;
    if (image->actual_height < 0)
        image->actual_height = -image->actual_height;
    image->dummy_line_bytes = bitmap_line_dummy_bytes(image->bih.bi_width, image->bih.bi_bit_count);
    image->bct = NULL;
    image->data = (unsigned char *)mem_alloc(image->actual_height * line_size);
    if (image->data == NULL)
        goto release_resource;

    if (image->bih.bi_bit_count == 8) {
        image->bct = (struct bitmap_color_table *)mem_alloc(sizeof(struct bitmap_color_table) << 8);
        if (image->bct == NULL)
            goto release_resource;

        if (fread(image->bct, sizeof(struct bitmap_color_table) << 8, 1, fp) != 1)
            goto release_resource;
    }

    for (y = 0, pdata = image->data; y < image->actual_height; ++y, pdata += line_size) {
        if (fread(pdata, line_size, 1, fp) != 1)
            goto release_resource;

        if (data_size == 4) {
            for (x = 3; x < line_size; x += data_size)
                pdata[x] = 0xFF;
        }

        if (image->dummy_line_bytes)
            fseek(fp, image->dummy_line_bytes, SEEK_CUR);
    }

    ret = 0;
release_resource:
    if (ret == -1 && image != NULL) {
        if (image->data != NULL)
            mem_free(image->data);

        if (image->bct != NULL)
            mem_free(image->bct);

        mem_free(image);
        image = NULL;
    }
    fclose(fp);

    return image;
}

static int bitmap_image_save(const char *filename, const struct bitmap_image *image)
{
    int32_t y;
    FILE *fp;
    uint32_t data_size;
    uint32_t line_size;
    unsigned char *pdata;
    char dummy_bytes = 0;

    if (filename == NULL || image == NULL)
        return -1;

    fp = fopen(filename, "wb");
    if (fp == NULL)
        return -1;

    data_size = image->bih.bi_bit_count >> 3;
    line_size = image->bih.bi_width * data_size;
    fwrite((void *)&image->bh, sizeof(image->bh), 1, fp);
    fwrite((void *)&image->bih, sizeof(image->bih), 1, fp);
    if (image->bih.bi_bit_count == 8)
        fwrite((void *)image->bct, sizeof(struct bitmap_color_table) << 8, 1, fp);

    for (y = 0, pdata = image->data; y < image->actual_height;
            ++y, pdata += line_size) {
        fwrite(pdata, line_size, 1, fp);
        if (image->dummy_line_bytes)
            fwrite(&dummy_bytes, 1, image->dummy_line_bytes, fp);
    }
    fflush(fp);
    fclose(fp);

    return 0;
}

#endif

static unsigned char image_format_pixel_size(const unsigned int format)
{
    unsigned char size;

    switch (format) {
    case IMAGE_FORMAT_GRAY:
        size = 1;
        break;
    case IMAGE_FORMAT_RGB:
    case IMAGE_FORMAT_BGR:
        size = 3;
        break;
    case IMAGE_FORMAT_RGBA:
    case IMAGE_FORMAT_BGRA:
        size = 4;
        break;
    default:    /* unknow format */
        return 0;
    }

    return size;
}

struct image *image_create(const unsigned int height, const unsigned int width,
        const unsigned int format)
{
    struct image *img;
    const unsigned int pixel_size = image_format_pixel_size(format);
    const unsigned int row_size = width * pixel_size;
    const unsigned int img_size = row_size * height;

    if (img_size == 0)
        return NULL;

    img = (struct image *)mem_alloc(sizeof(struct image) + img_size);
    if (img == NULL)
        return NULL;

    img->height = height;
    img->width = width;

    img->format = format;
    img->pixel_size = pixel_size;
    img->row_size = row_size;
    img->size = img_size;
    memset(img->data, 0, img_size);

    return img;
}

struct image *image_dump(const struct image *img)
{
    struct image *new_img;

    if (img == 0)
        return NULL;

    new_img = (struct image *)mem_alloc(sizeof(struct image) + img->size);
    if (new_img == NULL)
        return NULL;

    memcpy(new_img, img, sizeof(struct image) + img->size);

    return new_img;
}

void image_release(struct image *img)
{
    if (img)
        mem_free(img);
}

static unsigned char image_rgb2gray(const unsigned char red, const unsigned char green, const unsigned char blue)
{
    return (unsigned char)((38 * (unsigned int)red + 75 * (unsigned int)green + 15 * (unsigned int)blue) >> 7);
}

struct image *image_convert_format(const struct image *src_img, const unsigned int format)
{
    struct image *img;
    unsigned int i, j, offset[2];

    if (src_img == NULL)
        return NULL;

    if (src_img->format == format)
        return image_dump(src_img);

    img = image_create(src_img->height, src_img->width, format);
    if (img == NULL)
        return NULL;

    switch (src_img->format) {
    case IMAGE_FORMAT_GRAY:
        for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
            for (i = 0; i < img->width; ++i, ++offset[0], offset[1] += img->pixel_size) {
                img->data[offset[1] + img->pixel_size - 1] = 0xFF;
                memset(img->data + offset[1], src_img->data[offset[0]], 3);
            }
        }
        break;
    case IMAGE_FORMAT_RGB:
        switch (format) {
        case IMAGE_FORMAT_GRAY:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 3, ++offset[1]) {
                    img->data[offset[1]] = image_rgb2gray(src_img->data[offset[0]],
                            src_img->data[offset[0] + 1], src_img->data[offset[0] + 2]);
                }
            }
            break;
        case IMAGE_FORMAT_RGBA:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 3, offset[1] += 4) {
                    memcpy(img->data + offset[1], src_img->data + offset[0], 3);
                    img->data[offset[1] + 3] = 0xFF;
                }
            }
            break;
        case IMAGE_FORMAT_BGR:
        case IMAGE_FORMAT_BGRA:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 3, offset[1] += img->pixel_size) {
                    img->data[offset[1] + img->pixel_size - 1] = 0xFF;
                    img->data[offset[1]] = src_img->data[offset[0] + 2];
                    img->data[offset[1] + 1] = src_img->data[offset[0] + 1];
                    img->data[offset[1] + 2] = src_img->data[offset[0]];
                }
            }
            break;
        }
        break;
    case IMAGE_FORMAT_RGBA:
        switch (format) {
        case IMAGE_FORMAT_GRAY:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 4, ++offset[1]) {
                    img->data[offset[1]] = image_rgb2gray(src_img->data[offset[0]],
                            src_img->data[offset[0] + 1], src_img->data[offset[0] + 2]);
                }
            }
            break;
        case IMAGE_FORMAT_RGB:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 4, offset[1] += 3) {
                    memcpy(img->data + offset[1], src_img->data + offset[0], 3);
                }
            }
            break;
        case IMAGE_FORMAT_BGR:
        case IMAGE_FORMAT_BGRA:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 4, offset[1] += img->pixel_size) {
                    img->data[offset[1] + img->pixel_size - 1] = 0xFF;
                    img->data[offset[1]] = src_img->data[offset[0] + 2];
                    img->data[offset[1] + 1] = src_img->data[offset[0] + 1];
                    img->data[offset[1] + 2] = src_img->data[offset[0]];
                }
            }
            break;
        }
        break;
    case IMAGE_FORMAT_BGR:
        switch (format) {
        case IMAGE_FORMAT_GRAY:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 3, ++offset[1]) {
                    img->data[offset[1]] = image_rgb2gray(src_img->data[offset[0] + 2],
                            src_img->data[offset[0] + 1], src_img->data[offset[0]]);
                }
            }
            break;
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_RGBA:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 3, offset[1] += img->pixel_size) {
                    img->data[offset[1] + img->pixel_size - 1] = 0xFF;
                    img->data[offset[1]] = src_img->data[offset[0] + 2];
                    img->data[offset[1] + 1] = src_img->data[offset[0] + 1];
                    img->data[offset[1] + 2] = src_img->data[offset[0]];
                }
            }
            break;
        case IMAGE_FORMAT_BGRA:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 3, offset[1] += 4) {
                    memcpy(img->data + offset[1], src_img->data + offset[0], 3);
                    img->data[offset[1] + 3] = 0xFF;
                }
            }
            break;
        }
        break;
    case IMAGE_FORMAT_BGRA:
        switch (format) {
        case IMAGE_FORMAT_GRAY:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 4, ++offset[1]) {
                    img->data[offset[1]] = image_rgb2gray(src_img->data[offset[0] + 2],
                            src_img->data[offset[0] + 1], src_img->data[offset[0]]);
                }
            }
            break;
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_RGBA:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 4, offset[1] += img->pixel_size) {
                    img->data[offset[1] + img->pixel_size - 1] = 0xFF;
                    img->data[offset[1]] = src_img->data[offset[0] + 2];
                    img->data[offset[1] + 1] = src_img->data[offset[0] + 1];
                    img->data[offset[1] + 2] = src_img->data[offset[0]];
                }
            }
            break;
        case IMAGE_FORMAT_BGR:
            for (j = 0, offset[0] = 0, offset[1] = 0; j < img->height; ++j) {
                for (i = 0; i < img->width; ++i, offset[0] += 4, offset[1] += 3) {
                    memcpy(img->data + offset[1], src_img->data + offset[0], 3);
                }
            }
            break;
        }
        break;
    }

    return img;
}

struct image *image_convert_gray(const struct image *img)
{
    return image_convert_format(img, IMAGE_FORMAT_GRAY);
}

unsigned char image_find_binariztion_global_threshold(const struct image *img)
{
    unsigned char i;
    unsigned int x, y;
    unsigned int row_offset;
    float m1, m2, n1, n2, sum, t, t0;
    const unsigned char *color;

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY)
        return 0;

    sum = 0;
    for (y = 0; y < img->height; ++y) {
        row_offset = y * img->row_size;
        for (x = 0; x < img->width; ++x)
            sum += (float)img->data[row_offset + x];
    }

    t0 = sum / (float)img->size;
    for (i = 0; i < 100; ++i) {
        n1 = n2 = m1 = m2 = 0;
        for (y = 0; y < img->height; ++y) {
            row_offset = y * img->row_size;
            for (x = 0; x < img->width; ++x) {
                color = img->data + row_offset + x;
                if (*color > (unsigned char )t0) {
                    m1 += (float)*color;
                    ++n1;
                }
                else {
                    m2 += (float)*color;
                    ++n2;
                }
            }
        }

        t = (m1 / n1 + m2 / n2) / 2.0f;
        if (fabs(t - t0) < 5.0f)
            break;

        t0 = t;
    }

    return (unsigned char)(t + 0.5f);
}

int image_gray_binarize(struct image *img, const unsigned char threshold,
        const unsigned char gray, const unsigned bg_gray)
{
    unsigned int x, y;
    unsigned char *color;

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY)
        return -1;

    for (y = 0; y < img->height; ++y) {
        color = img->data + y * img->row_size;
        for (x = 0; x < img->width; ++x)
            color[x] = color[x] <= threshold ? gray : bg_gray;
    }

    return 0;
}

struct bitmatrix *image_create_bitmatrix(const struct image *img)
{
    unsigned int i, j, off;
    struct bitmatrix *matrix;

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY)
        return NULL;

    matrix = bitmatrix_create(img->height, img->width, 0);
    if (matrix == NULL)
        return NULL;

    for (j = 0, off = 0; j < img->height; ++j, off += img->row_size)
        for (i = 0; i < img->width; ++i)
            bitmatrix_set(matrix, j, i, img->data[off + i]);

    return matrix;
}

struct image *image_create_from_bitmatrix(const struct bitmatrix *matrix)
{
    struct image *img;
    unsigned int i, j, off;

    if (matrix == NULL)
        return NULL;

    img = image_create(matrix->row, matrix->column, IMAGE_FORMAT_GRAY);
    if (img == NULL)
        return NULL;

    for (j = 0, off = 0; j < img->height; ++j, off += img->row_size) {
        for (i = 0; i < img->width; ++i) {
            if (bitmatrix_get(matrix, j, i)) {
                img->data[off + i] = 0xFF;
            } else {
                img->data[off + i] = 0;
            }
        }
    }

    return img;
}

#ifdef IMAGE_ENABLE_JPEG

static struct image *image_open_jpeg(const char *jpeg_file)
{
    FILE *src_fp;
    unsigned char *jpeg_data;
    JSAMPARRAY raw_jpeg_data;
    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct cinfo;
    struct image *img;
    unsigned int height, width;

    if (jpeg_file == NULL)
        return NULL;

    src_fp = fopen(jpeg_file, "rb");
    if (src_fp == NULL)
        return NULL;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, src_fp);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    height = cinfo.output_height;
    width = cinfo.output_width;
    switch (cinfo.output_components) {
    case 1:
        img = image_create(height, width, IMAGE_FORMAT_GRAY);
        break;
    case 3:
        img = image_create(height, width, IMAGE_FORMAT_RGB);
        break;
    default:
        img = NULL;
        break;
    }

    if (img == NULL)
        goto release_resource;

    jpeg_data = img->data;
    raw_jpeg_data = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, img->row_size, 1);
    while (cinfo.output_scanline < height) {
        jpeg_read_scanlines(&cinfo, raw_jpeg_data, 1);
        memcpy(jpeg_data, *raw_jpeg_data, img->row_size);
        jpeg_data = jpeg_data + img->row_size;
    }

release_resource:
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(src_fp);

    return img;
}

static int image_saveas_jpeg(const char *jpeg_file, const struct image *img)
{
    JSAMPROW row_jpeg_data;
    struct jpeg_error_mgr jerr;
    struct jpeg_compress_struct cinfo;
    unsigned int row_size, offset[2];
    unsigned char *ptr1, *jpeg_data;
    const unsigned char *ptr;
    FILE *fp;
    int ret;

    ret = -1;
    if (jpeg_file == NULL || img == NULL)
        return ret;

    fp = fopen(jpeg_file, "wb");
    if (fp == NULL)
        return ret;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = img->width;
    cinfo.image_height = img->height;
    cinfo.input_components = image_format_pixel_size(img->format);
    if (cinfo.input_components == 4) { /* 丢弃透明通道 */
        cinfo.input_components = 3;
    } else if (cinfo.input_components != 3 && cinfo.input_components != 1) { /* 暂不支持16及其他格式的转换 */
        jpeg_data = NULL;
        goto release_resource;
    }

    row_size = cinfo.input_components * cinfo.image_width;
    if (img->format == IMAGE_FORMAT_GRAY) {
        cinfo.in_color_space = JCS_GRAYSCALE;
    } else {
        cinfo.in_color_space = JCS_RGB;
    }

    jpeg_data = (unsigned char *)mem_alloc(cinfo.image_height * row_size);
    if (jpeg_data == NULL)
        goto release_resource;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 50, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    ptr = img->data;
    ptr1 = jpeg_data;
    for (unsigned int j, i = 0; i < cinfo.image_height; ++i) {
        switch (img->format) {
        case IMAGE_FORMAT_GRAY:
        case IMAGE_FORMAT_RGB:
            memcpy(ptr1, ptr, row_size);
            break;
        case IMAGE_FORMAT_BGR:
        case IMAGE_FORMAT_BGRA:
            for (offset[0] = offset[1] = j = 0; j < cinfo.image_width;
                    offset[0] += img->pixel_size, offset[1] += 3, ++j) {
                ptr1[offset[1] + 0] = ptr[offset[0] + 2];
                ptr1[offset[1] + 1] = ptr[offset[0] + 1];
                ptr1[offset[1] + 2] = ptr[offset[0] + 0];
            }
            break;
        case IMAGE_FORMAT_RGBA:
            for (offset[0] = offset[1] = j = 0; j < cinfo.image_width;
                    offset[0] += 4, offset[1] += 3, ++j)
                memcpy(ptr1 + offset[1], ptr + offset[0], 3);
            break;
        }

        ptr += img->row_size;
        ptr1 += row_size;
    }

    row_jpeg_data = jpeg_data;
    while (cinfo.next_scanline < cinfo.image_height) {
        jpeg_write_scanlines(&cinfo, &row_jpeg_data, 1);
        row_jpeg_data = ((unsigned char *)row_jpeg_data) + row_size;
    }
    ret = 0;
release_resource:
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);
    if (jpeg_data != NULL) {
        mem_free(jpeg_data);
    }

    return ret;
}

#endif

#ifdef IMAGE_ENABLE_PNG

static struct image *image_open_png(const char *png_file)
{
    png_uint_32 width, height;
    png_structp png_ptr;
    png_infop info_ptr;
    int bit_depth, color_type;
    struct image *img;
    FILE *fp;

    if (png_file == NULL)
        return NULL;

    if ((fp = fopen(png_file, "rb")) == NULL)
        return NULL;

    img = NULL;
    info_ptr = NULL;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
        goto release_resource;

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
        goto release_resource;

    if (setjmp(png_jmpbuf(png_ptr)))
        goto release_resource;

    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
    switch (color_type) {
    case PNG_COLOR_TYPE_RGB:
        img = image_create(height, width, IMAGE_FORMAT_RGB);
        break;
    case PNG_COLOR_TYPE_RGBA:
        img = image_create(height, width, IMAGE_FORMAT_RGBA);
        break;
    case PNG_COLOR_TYPE_GRAY:
        img = image_create(height, width, IMAGE_FORMAT_GRAY);
        break;
    default:
        goto release_resource;
    }

    if (img == NULL)
        goto release_resource;

    for (png_uint_32 i = 0; i < height; ++i) {
        memcpy(img->data + i * img->row_size, row_pointers[i], img->row_size);
    }

release_resource:
    if (png_ptr != NULL) {
        if (info_ptr == NULL) {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
        } else {
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        }
    }
    fclose(fp);

    return img;
}

static int image_saveas_png(const char *file, const struct image *img)
{
    int ret;
    FILE *fp;
    png_infop info;
    png_structp png;
    struct image *dump_img;
    unsigned char *ptr1;
    unsigned char **rows;
    const unsigned char *ptr2;
    unsigned int i, j, offset, format;

    if (file == NULL || img == NULL)
        return -1;

    rows = (unsigned char **)mem_alloc(img->height * sizeof(*rows));
    if (rows == NULL)
        return -1;

    fp = fopen(file, "wb");
    if (fp == NULL) {
        mem_free(rows);
        return -1;
    }

    info = NULL;
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL) {
        mem_free(rows);
        fclose(fp);
        png_destroy_write_struct(&png, &info);
        return -1;
    }

    info = png_create_info_struct(png);
    if (info == NULL) {
        png_destroy_write_struct(&png, &info);
        mem_free(rows);
        fclose(fp);
        return -1;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        mem_free(rows);
        fclose(fp);
        return -1;
    }

    format = img->format;
    if (format == IMAGE_FORMAT_BGR || format == IMAGE_FORMAT_BGRA) {
        if (format == IMAGE_FORMAT_BGR) {
            format = IMAGE_FORMAT_RGB;
        } else {
            format = IMAGE_FORMAT_RGBA;
        }
        dump_img = image_create(img->height, img->width, format);
        if (dump_img == NULL) {
            png_destroy_write_struct(&png, &info);
            mem_free(rows);
            fclose(fp);
            return -1;
        }

        ptr1 = dump_img->data;
        ptr2 = img->data;
        for (j = 0; j < dump_img->height; ++j) {
            for (i = 0; i < dump_img->row_size; i += dump_img->pixel_size) {
                ptr1[0] = ptr2[2];
                ptr1[1] = ptr2[1];
                ptr1[2] = ptr2[0];
                if (format == IMAGE_FORMAT_RGBA)
                    ptr1[3] = ptr2[3];
                ptr1 += dump_img->pixel_size;
                ptr2 += img->pixel_size;
            }
        }

        for (j = 0, offset = 0; j < dump_img->height; ++j, offset += dump_img->row_size)
            rows[j] = dump_img->data + offset;
    } else {
        dump_img = NULL;
        for (j = 0, offset = 0; j < img->height; ++j, offset += img->row_size)
            rows[j] = (unsigned char *)((img->data - (const unsigned char *)NULL) + offset);
    }
    ret = 0;
    png_init_io(png, fp);
    png_set_compression_level(png, 9);
    switch (format) {
    case IMAGE_FORMAT_GRAY:
        png_set_IHDR(png, info, img->width, img->height, 8, PNG_COLOR_TYPE_GRAY,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        break;
    case IMAGE_FORMAT_RGB:
        png_set_IHDR(png, info, img->width, img->height, 8, PNG_COLOR_TYPE_RGB,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        break;
    case IMAGE_FORMAT_RGBA:
        png_set_IHDR(png, info, img->width, img->height, 8, PNG_COLOR_TYPE_RGBA,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        break;
    default:
        ret = -1;
        break;
    }
    if (ret == 0) {
        png_set_rows(png, info, (void*)rows);
        png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
        png_write_end(png,info);
    }

    png_destroy_write_struct(&png, &info);
    fclose(fp);
    mem_free(rows);
    if (dump_img != NULL)
        image_release(dump_img);

    return ret;
}

#endif

#ifdef IMAGE_ENABLE_BITMAP

static struct image *image_open_bmp(const char *file)
{
    struct image *img;
    struct bitmap_image *bmp_img;
    int i, j, j_start, j_end, setup;

    bmp_img = bitmap_image_open(file);
    if (bmp_img == NULL)
        return NULL;

    switch (bmp_img->bih.bi_bit_count) {
    case 8:
        img = image_create(bmp_img->actual_height, bmp_img->bih.bi_width, IMAGE_FORMAT_GRAY);
        break;
    case 24:
        img = image_create(bmp_img->actual_height, bmp_img->bih.bi_width, IMAGE_FORMAT_BGR);
        break;
    case 32:
        img = image_create(bmp_img->actual_height, bmp_img->bih.bi_width, IMAGE_FORMAT_BGRA);
        break;
    default:
        img = NULL;
        break;
    }

    if (img == NULL) {
        goto release_resource;
    }

    if (bmp_img->bih.bi_height < 0) {
        j_start = 0;
        j_end = img->size;
        setup = img->row_size;
    } else {
        j_start = img->size - img->row_size;
        j_end = -((int)img->row_size);
        setup = j_end;
    }

    for (j = j_start, i = 0; j != j_end; j = j + setup, i += img->row_size)
        memcpy(img->data + i, bmp_img->data + j, img->row_size);

release_resource:
    bitmap_image_release(bmp_img);

    return img;
}

static int image_saveas_bitmap(const char *file, const struct image *img)
{
    int ret;
    unsigned int j, i;
    unsigned char *dst_ptr[2];
    struct bitmap_image *bmp_img;
    const unsigned char *src_ptr;

    if (file == NULL || img == NULL)
        return -1;

    bmp_img = bitmap_image_create(img->height, img->width, img->pixel_size << 3, 0);
    if (bmp_img == NULL)
        return -1;

    for (src_ptr = img->data, dst_ptr[0] = bmp_img->data + img->size - img->row_size, j = 0; j < img->height; ++j) {
        dst_ptr[1] = dst_ptr[0];
        for (i = 0; i < img->width; ++i) {
            memcpy(dst_ptr[1], src_ptr, img->pixel_size);
            if (img->format == IMAGE_FORMAT_RGB || img->format == IMAGE_FORMAT_RGBA) {
                dst_ptr[1][0] = src_ptr[2];
                dst_ptr[1][2] = src_ptr[0];
            }
            src_ptr += img->pixel_size;
            dst_ptr[1] += img->pixel_size;
        }
        dst_ptr[0] -= img->row_size;
    }
    ret = bitmap_image_save(file, bmp_img);
    bitmap_image_release(bmp_img);

    return ret;
}

#endif

int image_save(const char *file, const struct image *img, const int flag)
{
    int ret;

    switch (flag) {
#ifdef IMAGE_ENABLE_BITMAP
    case IMAGE_FILE_BITMAP:
        ret = image_saveas_bitmap(file, img);
        break;
#endif
#ifdef IMAGE_ENABLE_JPEG
    case IMAGE_FILE_JPEG:
        ret = image_saveas_jpeg(file, img);
        break;
#endif
#ifdef IMAGE_ENABLE_PNG
    case IMAGE_FILE_PNG:
        ret = image_saveas_png(file, img);
        break;
#endif
    default:
        ret = -1;
        break;
    }

    return ret;
}

struct image *image_open(const char *file)
{
    FILE *fp;
    struct image *img;
    unsigned char checkheader[8];

    fp = fopen(file, "rb");
    if (fp == NULL)
        return NULL;

    if (fread(checkheader, 1, 8, fp) != 8) {
        fclose(fp);
        return NULL;
    }
    fclose(fp);

#ifdef IMAGE_ENABLE_PNG
    if (png_sig_cmp(checkheader, 0, 8) == 0) {
        img = image_open_png(file);
    } else 
#endif
#ifdef IMAGE_ENABLE_BITMAP
    if (checkheader[0] == 'B' && checkheader[1] == 'M') {
        img = image_open_bmp(file);
    } else 
#endif
#ifdef IMAGE_ENABLE_JPEG
    if (checkheader[0] == 0xFF && checkheader[1] == 0xD8) {
        img = image_open_jpeg(file);
    } else
#endif
    {
        img = NULL;
    }

    return img;
}

int image_gray_blance(struct image *img, const unsigned char grad)
{
    unsigned int x, y;
    unsigned int off;
    unsigned int hd[256] = {0};
    float p, q[256];

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY)
        return -1;

    for (y = 0, off = 0; y < img->height; ++y, off += img->width) {
        for (x = 0; x < img->width; ++x) {
            ++hd[img->data[off + x]];
        }
    }

    q[0] = (float)hd[0] / (float)img->size;
    for (x = 1; x < 256; ++x) {
        p = (float)hd[x] / (float)img->size;
        q[x] = q[x - 1] + p;
    }

    for (y = 0, off = 0; y < img->height; ++y, off += img->width) {
        for (x = 0; x < img->width; ++x) {
            img->data[off + x] = (unsigned char)(q[img->data[off + x]] * grad);
        }
    }

    return 0;
}

struct image *image_sharpening(const struct image *img, const int *template1, const int *template2,
        const unsigned int dimension)
{
    struct vector g;
    unsigned int off[4];
    unsigned int x, y, s, t;
    unsigned int n = dimension >> 1;
    struct image *dump_img;

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY ||
            (template1 == NULL && template2 == NULL)) {
        return NULL;
    }

    dump_img = image_create(img->height, img->width, IMAGE_FORMAT_GRAY);
    if (dump_img == NULL)
        return NULL;

    off[2] = n * img->row_size;
    for (y = n; y < img->height - n; ++y, off[2] += img->width) {
        off[3] = (y - n) * img->row_size - n;
        for (x = n; x < img->width - n; ++x) {
            g.i = 0;
            g.j = 0;
            off[0] = 0;
            off[1] = off[3] + x;
            for (t = 0; t < dimension;
                    ++t, off[0] += dimension, off[1] += img->width) {
                for (s = 0; s < dimension; ++s) {
                    if (template1) {
                        g.i += template1[off[0] + s] * img->data[off[1] + s];
                    }

                    if (template2) {
                        g.j += template2[off[0] + s] * img->data[off[1] + s];
                    }
                }
            }
            if (g.i < 0) {
                g.i = -g.i;
            }

            if (g.j < 0) {
                g.j = -g.j;
            }
            g.i = (g.i + g.j) >> 1;
            dump_img->data[off[2] + x] =
                    (((unsigned int)g.i) > 255.0f) ? 255 : (unsigned char)g.i;
        }
    }

    return dump_img;
}

const static int sobel_fact[2][3][3] = {
    {{-1, -2, -1}, { 0, 0, 0}, { 1, 2, 1}},     /* 求Gy */
    {{-1,  0,  1}, {-2, 0, 2}, {-1, 0, 1}}};    /* 求Gx */
struct image *image_sobel_enhancing(const struct image *img)
{
    return image_sharpening(img, sobel_fact[0][0], sobel_fact[1][0], 3);
}

const static int laplace_fact[2][3][3] = {
    {{ 0, -1,  0}, {-1, 5, -1}, { 0, -1,  0}},
    {{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}}};
struct image *image_laplace_enhancing(const struct image *img, const unsigned int flag)
{
    return image_sharpening(img, flag ? laplace_fact[1][0] : laplace_fact[0][0], NULL, 3);
}

static int make_gaussian_filter_kernel(float *const kernel, const int size, const float sigma)
{
    int offset, center, i, k;
    float sum, d_sigma_2, x2;

    if ((size & 1) == 0)
        return -1;

    sum = 0.0f;
    center = size >> 1;
    d_sigma_2 = 2 * sigma * sigma;
    for (i = 0; i < size; i++) {
        offset = i * size;
        x2 = (float)((i - center) * (i - center));
        for (k = 0; k < size; k++) {
            kernel[offset + k] = expf(-(x2 + (k - center) * (k - center)) / d_sigma_2);
            sum += kernel[offset + k];
        }
    }

    for (i = 0; i < size; i++) {
        offset = i * size;
        for (k = 0; k < size; k++)
            kernel[offset + k] /= sum;
    }

    return 0;
}

int image_filter_gaussian(struct image *img, const unsigned int ksize, const float sigma)
{
    unsigned int center;
    unsigned int row, column, i, j, i_offset, r;
    unsigned int row_start, column_start;
    unsigned int row_offset, column_offset;
    struct image *dump_img;
    float *kernel;
    float sum[4];

    if (img == NULL || ksize < 0 || ksize > 31 || (ksize & 1) == 0 || sigma <= 0.0f)
        return -1;

    dump_img = image_dump(img);
    if (dump_img == NULL)
        return -1;

    kernel = (float *)mem_alloc(sizeof(float) * ksize * ksize);
    if (kernel == NULL) {
        image_release(dump_img);
        return -1;
    }

    center = ksize >> 1;
    make_gaussian_filter_kernel(kernel, ksize, sigma);
    for (row = center; row < dump_img->height - center; ++row) {
        row_start = row - center;
        row_offset = row * dump_img->row_size;
        for (column = center; column < dump_img->width - center; ++column) {
            sum[0] = sum[1] = sum[2] = sum[3] = 0.0f;
            column_start = column - center;
            for (i = 0; i < ksize; ++i) {
                column_offset = (row_start + i) * dump_img->row_size + column_start *  dump_img->pixel_size;
                i_offset = i * ksize;
                for (j = 0; j < ksize; ++j) {
                    for (r = 0; r < dump_img->pixel_size; ++r) {
                        sum[r] += kernel[i_offset + j] * dump_img->data[column_offset + r];
                    }
                    column_offset += dump_img->pixel_size;
                }
            }
            column_offset = row_offset + column * dump_img->pixel_size;
            for (r = 0; r < dump_img->pixel_size; ++r) {
                img->data[column_offset + r] = sum[r] > 255.0f ? 255 : (unsigned char)sum[r];
            }
        }
    }
    image_release(dump_img);
    mem_free(kernel);

    return 0;
}

int image_canny_enhancing(struct image *img, const int gauss_ksize, const float gauss_sigma,
        const float min_trheshold, const float max_threshold)
{
    struct bitmatrix *mark_matrix;
    struct vector *grad_vector;
    float *grad_result;
    unsigned int x, y;

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY)
        return -1;

    mark_matrix = bitmatrix_create(img->height, img->width, 0);
    if (mark_matrix == NULL)
        return -1;

    grad_vector = (struct vector *)mem_alloc(sizeof(struct vector) * img->size
            + sizeof(float) * img->size);
    if (grad_vector == NULL) {
        bitmatrix_release(mark_matrix);
        return -1;
    }

    grad_result = (float *)(grad_vector + img->size);
    /* 1. 高斯滤波 */
    if (image_filter_gaussian(img, gauss_ksize, gauss_sigma) == -1) {
        bitmatrix_release(mark_matrix);
        mem_free(grad_vector);
        return -1;
    }

    /* 2. sobel计算边缘梯度 */
    memset(grad_vector, 0, sizeof(struct vector) * img->size + sizeof(float) * img->size);
    for (y = 1; y < img->height - 1; ++y) {
        unsigned int s, t;
        unsigned int offset = y * img->row_size + 1;
        for (x = 1; x < img->width - 1; ++x, ++offset) {
            unsigned int _offset = (y - 1) * img->row_size + (x - 1);
            for (t = 0; t < 3; ++t, _offset += img->row_size) {
                for (s = 0; s < 3; ++s) {
                    /* 分别计算出Gx和Gy */
                    grad_vector[offset].i += sobel_fact[1][t][s] * img->data[_offset + s]; // Gx
                    grad_vector[offset].j += sobel_fact[0][t][s] * img->data[_offset + s]; // Gy
                }
            }
            grad_result[offset] = vector_length(grad_vector + offset);
        }
    }

    /* 3. 非极大值抑制 */
    memset(img->data, 0, img->size);
    for (y = 1; y < img->height - 1; ++y) {
        unsigned int offset;

        offset = y * img->row_size + 1;
        for (x = 1; x < img->width - 1; ++x, ++offset) {
            int gx, gy;
            float weight, d_temp[2], g[4];

            if (grad_result[offset] == 0.0f)
                continue;

            gx = grad_vector[offset].i;
            gy = grad_vector[offset].j;
            if (fabs(gy) > fabs(gx)) {
                weight = (float)fabs(gx) / (float)fabs(gy);
                g[1] = grad_result[offset - img->row_size];
                g[3] = grad_result[offset + img->row_size];
                if (gx * gy > 0) {
                    g[0] = grad_result[offset - img->row_size - 1];
                    g[2] = grad_result[offset + img->row_size + 1];
                } else {
                    g[0] = grad_result[offset - img->row_size + 1];
                    g[2] = grad_result[offset + img->row_size - 1];
                }
            } else {
                weight = (float)fabs(gy) / (float)fabs(gx);
                g[1] = grad_result[offset + 1];
                g[3] = grad_result[offset - 1];
                if (gx * gy > 0) {
                    g[0] = grad_result[offset + 1 + img->row_size];
                    g[2] = grad_result[offset - 1 - img->row_size];
                } else {
                    g[0] = grad_result[offset + 1 - img->row_size];
                    g[2] = grad_result[offset - 1 + img->row_size];
                }
            }
            d_temp[0] = weight * g[0] + (1.0f - weight) * g[1];
            d_temp[1] = weight * g[2] + (1.0f - weight) * g[3];
            if (grad_result[offset] >= d_temp[0] && grad_result[offset] >= d_temp[1]) {
                if (grad_result[offset] >= max_threshold) {
                    img->data[offset] = 0xFF;
                } else if (grad_result[offset] >= min_trheshold) {
                    img->data[offset] = 0x7F;
                } else {
                    img->data[offset] = 0;
                }
            } else {
                img->data[offset] = 0;
            }
        }
    }

    /* 4. 滞后边界跟踪 */
    for (y = 1; y < img->height - 1; ++y) {
        unsigned int offset, _offset;

        offset = y * img->row_size + 1;
        for (x = 1; x < img->width - 1; ++x, ++offset) {
            if (img->data[offset] == 0x7F) {
                struct point p, _p;
                struct stack s;
                struct linkedlist_queue q;
                unsigned char connected;

                p.x = x;
                p.y = y;
                connected = 0;
                stack_init(&s, sizeof(p));
                linkedlist_queue_init(&q, sizeof(p));
                stack_push(&s, &p);
                linkedlist_queue_enque(&q, &p, 1);
                bitmatrix_set(mark_matrix, p.y, p.x, 1);
                while (!stack_empty(&s)) {
                    stack_pop(&s, &p);
                    for (_p.y = p.y - 1; _p.y <= p.y + 1; ++_p.y) {
                        _offset = _p.y * img->row_size + p.x - 1;
                        for (_p.x = p.x - 1; _p.x <= p.x + 1; ++_p.x, ++_offset) {
                            if (img->data[_offset] >= 0x7F && !bitmatrix_get(mark_matrix, _p.y, _p.x)) {
                                if (img->data[_offset] == 0xFF)
                                    connected = 0xFF;

                                stack_push(&s, &_p);
                                linkedlist_queue_enque(&q, &_p, 1);
                                bitmatrix_set(mark_matrix, _p.y, _p.x, 1);
                            }
                        }
                    }
                }

                stack_clear(&s);
                while (linkedlist_queue_size(&q) != 0) {
                    linkedlist_queue_deque(&q, &p, 1);
                    img->data[p.y * img->row_size + p.x] = connected;
                }
            }
        }
    }
    bitmatrix_release(mark_matrix);
    mem_free(grad_vector);

    return 0;
}

int image_filter_median(struct image *img, const unsigned int m, const unsigned int n)
{
    int ret;
    unsigned int i, j;
    struct image *dump_img;
    const unsigned char size = m * n;
    const unsigned char half_m = m >> 1;
    const unsigned char half_n = n >> 1;
    const unsigned char threshold = size >> 1;

    ret = -1;
    if (img == NULL || m >= img->height || n >= img->width || m > 15 || n > 15)
        return ret;

    if (size == 1)
        return 0;

    dump_img = (struct image *)image_dump(img);
    if (dump_img == NULL)
        return ret;

    for (j = 0; j < dump_img->height - m; ++j) {
        const unsigned int j_offset = j * dump_img->row_size;
        unsigned int offset, a, b;
        unsigned char counts[256] = {0};
        unsigned char counts_right = 0;
        unsigned char counts_mid = 0;
        unsigned char counts_left = 0;
        unsigned char mid_index;

        /* 1. 更新直方图 */;
        for (b = j, offset = j_offset; b < j + m; ++b, offset += img->row_size)
            for (a = 0; a < n; ++a)
                ++counts[dump_img->data[offset + a]];

        b = 0;
        mid_index = 0;
        for (unsigned int f = 0, index = 0; index < 256; ++index) {
            b += counts[index];
            if (b < threshold) {
                counts_left = b;
            } else {
                if (f == 0) {
                    f = 1;
                    mid_index = index;
                    counts_mid = counts[index];
                } else {
                    counts_right += counts[index];
                }
            }
        }

        offset = img->row_size * (j + half_m) + half_n;
        img->data[offset] = mid_index;

        /* 2. 滑动窗口统计 */
        for (i = 1; i < dump_img->width - n; ++i) {
            unsigned int b_off[2] = {j_offset + i - 1, j_offset + i + n - 1};

            /* 2.1 重新统计直方图 */
            for (b = 0; b < m; ++b, b_off[0] += dump_img->row_size, b_off[1] += dump_img->row_size) {
                const unsigned char img_data[2] = {dump_img->data[b_off[0]], dump_img->data[b_off[1]]};

                --counts[img_data[0]];
                ++counts[img_data[1]];
                if (img_data[0] > mid_index) {
                    --counts_right;
                } else if (img_data[0] < mid_index) {
                    --counts_left;
                } else {
                    --counts_mid;
                }

                if (img_data[1] > mid_index) {
                    ++counts_right;
                } else if (img_data[1] < mid_index) {
                    ++counts_left;
                } else {
                    ++counts_mid;
                }
            }

            /* 2.2 重新调整中值的游标 */
            if (counts_left >= threshold) {
                while (mid_index > 0) {
                    counts_right += counts[mid_index];
                    --mid_index;
                    counts_mid = counts[mid_index];
                    counts_left -= counts[mid_index];
                    if (counts_left < threshold)
                        break;
                }
            } else if (counts_mid + counts_left + counts_right >= threshold && counts_mid + counts_left < threshold) {
                while (mid_index < 255) {
                    counts_left += counts[mid_index];
                    ++mid_index;
                    counts_mid = counts[mid_index];
                    counts_right -= counts[mid_index];
                    if (counts_mid + counts_left >= threshold)
                        break;
                }
            }
            img->data[offset + i] = mid_index;
        }
    }

    ret = 0;
    image_release(dump_img);

    return ret;
}

/**
 * gray->进行hough变换的像素点的值
 */
struct image *image_hough_transform(const struct image *img,
        const unsigned int threshold, const unsigned int gray)
{
    int xy;
    struct image *hough_img;
    unsigned int *hough_param;
    unsigned int j, i, offset;
    float sinf_theta, cosf_theta;
    float delta_theta, current_theta;
    int rho, rho_max, theta_n, theta_i;

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY)
        return NULL;

    hough_img = image_create(img->height, img->width, img->format);
    if (hough_img == NULL)
        return NULL;

    theta_n = 360;
    delta_theta = M_PIf / (float)theta_n;
    rho_max = (int)(sqrtf((float)(img->height * img->height + img->width * img->width))
            + 0.5f);
    hough_param = (unsigned int *)mem_alloc(sizeof(unsigned int) * (rho_max * theta_n));
    if (hough_param == NULL) {
        image_release(hough_img);
        return NULL;
    }
    memset(hough_param, 0, sizeof(unsigned int) * (rho_max * theta_n));

    for (j = 0, offset = 0; j < img->height; ++j) {
        for (i = 0; i < img->width; ++i, ++offset) {
            if (img->data[offset] == gray) {
                for (theta_i = 0, current_theta = 0.0f; theta_i < theta_n;
                        ++theta_i, current_theta += delta_theta) {
                    /* rho = x * cos(a) + y * sin(a) */
                    rho = (int)((float)j * sinf(current_theta)
                            + (float)i * cosf(current_theta) + 0.5f);
                    if (rho >= 0 && rho < rho_max)
                        ++hough_param[rho * theta_n + theta_i];
                }
            }
        }
    }

    for (offset = 0, rho = 0; rho < rho_max; ++rho) {
        for (current_theta = 0.0f, theta_i = 0; theta_i < theta_n;
                ++offset, ++theta_i, current_theta += delta_theta) {
            sinf_theta = sinf(current_theta);
            cosf_theta = cosf(current_theta);
            if (hough_param[offset] >= threshold) {
                if (sinf_theta != 0.0f) {
                    for (i = 0; i < img->width; ++i) {
                        xy = (int)(((float)rho - (float)i * cosf_theta) / sinf_theta + 0.5f);
                        if (xy >= 0 && (unsigned int)xy < img->height) {
                            hough_img->data[xy * hough_img->row_size + i] = 0xFF;
                        }
                    }
                }
                if (cosf_theta != 0.0f) {
                    for (j = 0; j < img->height; ++j) {
                        xy = (int)(((float)rho - (float)j * sinf_theta) / cosf_theta + 0.5f);
                        if (xy >= 0 && (unsigned int) xy < img->width) {
                            hough_img->data[j * hough_img->row_size + xy] = 0xFF;
                        }
                    }
                }
            }
        }
    }

    mem_free(hough_param);
    return hough_img;
}

int image_nearest_interp(const struct image *src_img,
        unsigned char *new_pixel, const float x, const float y)
{
    int row, column;

    row = (int)(y + 0.5f);
    column = (int)(x + 0.5f);

    if (src_img == NULL || new_pixel == NULL
            || row < 0 || (unsigned int)row >= src_img->height
            || column < 0 || (unsigned int)column >= src_img->width)
        return -1;

    memcpy(new_pixel, &src_img->data[row * src_img->row_size + column * src_img->pixel_size],
            src_img->pixel_size);

    return 0;
}

int image_bilinear_interp(const struct image *src_img,
        unsigned char *new_pixel, const float x, const float y)
{
    unsigned int x_scale, y_scale, pixel_off;
    const unsigned char *pixel[4];

    if (src_img == NULL || new_pixel == NULL)
        return -1;

    if (x < 0 || ((unsigned int)x + 1) >= src_img->width
            || y < 0 || ((unsigned int)y + 1) >= src_img->height)
        return -1;

    pixel[0] = &src_img->data[((int)y) * src_img->row_size + ((int)x) * src_img->pixel_size];
    pixel[1] = pixel[0] + src_img->pixel_size;
    pixel[2] = pixel[0] + src_img->row_size;
    pixel[3] = pixel[2] + src_img->pixel_size;
    x_scale = ((unsigned int)(x * 256.0f)) & 255;
    y_scale = ((unsigned int)(y * 256.0f)) & 255;
    for (pixel_off = 0; pixel_off < src_img->pixel_size; ++pixel_off) {
        new_pixel[pixel_off] =
            ((256 - y_scale) * ((256 - x_scale) * pixel[0][pixel_off] + x_scale * pixel[1][pixel_off])
            + y_scale * ((256 - x_scale) * pixel[2][pixel_off] + x_scale * pixel[3][pixel_off])) >> 16;
    }

    return 0;
}

int image_bicubic_interp(const struct image *src_img,
        unsigned char *new_pixel, const float x, const float y)
{
    int wu[4], wv[4], g[4];
    unsigned int i, offset;
    int u_scale, v_scale, diff;
    const unsigned char *pixel[17];
    const int a_16x = (int)(-0.5f * 16.0f);

    if (src_img == NULL || new_pixel == NULL)
        return -1;

    if (x < 1 || ((unsigned int)x + 2) >= src_img->width || y < 1 || ((unsigned int)y + 2) >= src_img->height)
        return -1;

    pixel[16] = src_img->data + (((int)y) - 1) * src_img->row_size + (((int)x) - 1) * src_img->pixel_size;
    for (offset = 0; offset < 16;) {
        pixel[offset++] = pixel[16];
        pixel[16] += src_img->row_size;
        for (i = 1; i < 4; ++i, ++offset) {
            pixel[offset] = pixel[offset - 1] + src_img->pixel_size;
        }
    }

    u_scale = ((int)(x * 128.0f)) & 127;
    v_scale = ((int)(y * 128.0f)) & 127;
    diff = 128 + u_scale;
    wu[0] = (a_16x * (-8388608 + (131072 - (640 - diff) * diff) * diff)) >> 4;
    diff = u_scale;
    wu[1] = 2097152 - ((((a_16x + 48) << 7) - (a_16x + 32) * diff) * diff * diff >> 4);
    diff = 128 - u_scale;
    wu[2] = 2097152 - ((((a_16x + 48) << 7) - (a_16x + 32) * diff) * diff * diff >> 4);
    diff = 256 - u_scale;
    wu[3] = (a_16x * (-8388608 + (131072 - (640 - diff) * diff) * diff)) >> 4;
    diff = 128 + v_scale;
    wv[0] = (a_16x * (-8388608 + (131072 - (640 - diff) * diff) * diff)) >> 4;
    diff = v_scale;
    wv[1] = 2097152 - ((((a_16x + 48) << 7) - (a_16x + 32) * diff) * diff * diff >> 4);
    diff = 128 - v_scale;
    wv[2] = 2097152 - ((((a_16x + 48) << 7) - (a_16x + 32) * diff) * diff * diff >> 4);
    diff = 256 - v_scale;
    wv[3] = (a_16x * (-8388608 + (131072 - (640 - diff) * diff) * diff)) >> 4;
    for (offset = 0; offset < src_img->pixel_size; ++offset) {
        for (i = 0; i < 4; ++i) {
            g[i] = (wv[0] * pixel[i][offset] + wv[1] * pixel[i + 4][offset]
                + wv[2] * pixel[i + 8][offset] + wv[3] * pixel[i + 12][offset]) >> 21;
        }

        g[0] = (g[0] * wu[0] + g[1] * wu[1] + g[2] * wu[2] + g[3] * wu[3]) >> 21;
        if (g[0] >= 255) {
            g[0] = 255;
        } else if (g[0] <= 0) {
            g[0] = 0;
        }
        new_pixel[offset] = (unsigned char)g[0];
    }

    return 0;
}

struct image *image_perspective_transform(const struct image *img,
        const unsigned int nh, const unsigned int nw,
        const struct point src_point[4], const struct point dst_point[4])
{
    float a[8], z;
    float coeff[8][9];
    struct image *rect_img;
    unsigned int i, j, offset;
    unsigned char pixel[4];

    if (img == NULL ||src_point == NULL || dst_point == NULL || nh * nw == 0)
        return NULL;

    rect_img = image_create(nh, nw, img->format);
    if (rect_img == NULL)
        return NULL;

    for (offset = 0; offset < 4; ++offset) {
        j = offset << 1;
        i = j + 1;
        coeff[j][0] = (float)dst_point[offset].x;
        coeff[j][1] = (float)dst_point[offset].y;
        coeff[j][2] = 1.0f;
        coeff[j][3] = 0.0f;
        coeff[j][4] = 0.0f;
        coeff[j][5] = 0.0f;
        coeff[j][6] = (float)(-src_point[offset].x * dst_point[offset].x);
        coeff[j][7] = (float)(-src_point[offset].x * dst_point[offset].y);
        coeff[j][8] = (float)src_point[offset].x;
        coeff[i][0] = 0.0f;
        coeff[i][1] = 0.0f;
        coeff[i][2] = 0.0f;
        coeff[i][3] = (float)dst_point[offset].x;
        coeff[i][4] = (float)dst_point[offset].y;
        coeff[i][5] = 1.0f;
        coeff[i][6] = (float)(-src_point[offset].y * dst_point[offset].x);
        coeff[i][7] = (float)(-src_point[offset].y * dst_point[offset].y);
        coeff[i][8] = (float)src_point[offset].y;
    }
    if (gaussian_elimination(coeff[0], a, 8) == -1)
        return NULL;

    for (j = 0; j < rect_img->height; ++j) {
        offset = j * rect_img->row_size;
        for (i = 0; i < rect_img->width; ++i) {
            z = a[6] * i + a[7] * j + 1.0f;
            if (z == 0.0f)
                continue;

            if (image_bilinear_interp(img, pixel,
                    (a[0] * i + a[1] * j + a[2]) / z,
                    (a[3] * i + a[4] * j + a[5]) / z) == -1)
                continue;

            memcpy(&rect_img->data[offset + i * rect_img->pixel_size],
                pixel, rect_img->pixel_size);
        }
    }

    return rect_img;
}

struct image *image_rotation(const struct image *src_img,
        const struct point *rotation_center, const struct point *offset, const float theta,
        int (*interp)(const struct image *, unsigned char *, const float, const float))
{
    int i, j;
    struct image *img;
    float sinf_theta, cosf_theta;
    float sinf_theta_dy, cosf_theta_dy;
    unsigned int data_off;
    unsigned char pixel[4];

    if (src_img == NULL || rotation_center == NULL || offset == NULL)
        return NULL;

    img = image_create(src_img->height, src_img->width, src_img->format);
    if (img == NULL)
        return NULL;

    sinf_theta = sinf(theta);
    cosf_theta = cosf(theta);
    memset(pixel, 0xFF, sizeof(pixel));
    memset(img->data, 0xFF, img->size);
    for (data_off = 0, j = 0; j < (int)img->height; ++j) {
        sinf_theta_dy = (j - rotation_center->y) * sinf_theta + rotation_center->x - offset->x;
        cosf_theta_dy = (j - rotation_center->y) * cosf_theta + rotation_center->y - offset->y;
        for (i = 0; i < (int)img->width; ++i, data_off += img->pixel_size) {
            if (interp(src_img, pixel,
                    (i - rotation_center->x) * cosf_theta + sinf_theta_dy,
                    -(i - rotation_center->x) * sinf_theta + cosf_theta_dy) == -1)
                continue;

            memcpy(&img->data[data_off], pixel, img->pixel_size);
        }
    }

    return img;
}

void img_print_point(struct image *img, const unsigned int x, const unsigned int y, const unsigned char *color, const unsigned int size)
{
    unsigned int i, j;
    unsigned int off_i[2], off_j[2];

    if (img == NULL || x < 0 || y < 0 || x >= img->width || y >= img->width || size == 0 || color == NULL)
        return;

    if (x <= size) {
        off_i[0] = 0;
    } else {
        off_i[0] = x - size;
    }

    if (x + size > img->width) {
        off_i[1] = img->width;
    } else {
        off_i[1] = x + size;
    }

    if (y <= size) {
        off_j[0] = 0;
    } else {
        off_j[0] = y - size;
    }

    if (y + size > img->height) {
        off_j[1] = img->height;
    } else {
        off_j[1] = y + size;
    }

    for (j = off_j[0]; j < off_j[1]; ++j) {
        for (i = off_i[0]; i < off_i[1]; ++i) {
            memcpy(img->data + j * img->row_size + i * img->pixel_size, color, img->pixel_size);
        }
    }
}

#define IMAGE_RFEDGE_AMP_LIMIT_MIN      25

unsigned int image_find_raise_fall_edges_by_offset(
        const struct image *img, const struct point *pstart,
        const struct point *setup_off, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    int imgdata_off;
    struct point off_end;
    unsigned int cnt, i;
    unsigned char grad;
    unsigned char gray;
    unsigned char cur_type;
    unsigned char last_type;
    unsigned char max_grad;
    unsigned char max_amplitude;
    const unsigned char *imgdata;
    struct image_raise_fall_edge *cur_edge;
    struct image_raise_fall_edge *max_edge;
    struct image_raise_fall_edge *last_edge;
    struct image_raise_fall_edge *buff;
    struct image_raise_fall_edge *buff_end;
    struct image_raise_fall_edge *buff_prev;

    if (img == NULL || pstart == NULL || setup_off == NULL || len <= 1 || pedge == NULL || num == 0)
        return 0;

    if (pstart->x < 0 || pstart->x >= (int)img->width || pstart->y < 0 || pstart->y >= (int)img->height)
        return 0;

    cnt = 0;
    max_grad = 0;
    max_edge = NULL;
    max_amplitude = 0;
    cur_edge = pedge - 1;
    off_end.x = pstart->x + setup_off->x;
    off_end.y = pstart->y + setup_off->y;
    imgdata_off = setup_off->y * img->width + setup_off->x;
    imgdata = img->data + pstart->y * img->width + pstart->x;
    gray = *imgdata;
    imgdata += imgdata_off;
    last_type = IMAGE_RFEDGE_TYPE_NONE;
    buff_end = pedge + num - 1;
    for (i = 1; i < len && off_end.x >= 0 && off_end.x < (int)img->width
                && off_end.y >= 0 && off_end.y < (int)img->height;
            ++i, imgdata += imgdata_off, off_end.x += setup_off->x, off_end.y += setup_off->y) {
        if (gray == *imgdata) {
            last_type = IMAGE_RFEDGE_TYPE_NONE;
            continue;
        } else if (gray < *imgdata) {
            grad = *imgdata - gray;
            cur_type = IMAGE_RFEDGE_TYPE_RAISE;
        } else {
            grad = gray - *imgdata;
            cur_type = IMAGE_RFEDGE_TYPE_FALL;
        }

        if (grad > max_grad)
            max_grad = grad;

        if (last_type != cur_type) {
            if (max_edge == NULL) {
                max_edge = pedge;
            } else if (max_edge->max_grad <= cur_edge->max_grad
                    && max_edge->amplitude <= cur_edge->amplitude) {
                max_edge = cur_edge;
            }

            if (cur_edge >= buff_end)
                break;

            ++cur_edge;
            cur_edge->begin = i - 1;
            cur_edge->end = i;
            cur_edge->type = cur_type;
            cur_edge->max_grad = grad;
            cur_edge->min_grad = grad;
            cur_edge->amplitude = grad;
            last_type = cur_type;
            cur_edge->max_gray = gray;
            cur_edge->min_gray = *imgdata;
            cur_edge->dpos_256x = grad * i;
        } else {
            cur_edge->end = i;
            if (grad > cur_edge->max_grad) {
                cur_edge->max_grad = grad;
            } else if (grad < cur_edge->min_grad) {
                cur_edge->min_grad = grad;
            }
            cur_edge->min_gray = *imgdata;
            cur_edge->amplitude += grad;
            cur_edge->dpos_256x += grad * i;
            if (cur_edge->amplitude > max_amplitude)
                max_amplitude = cur_edge->amplitude;
        }
        gray = *imgdata;
    }

    if (cur_edge < pedge)
        return 0;

    if (max_edge->amplitude <= 8)
        return 0;

    cnt = (unsigned int)(cur_edge - pedge + 1);
    buff = (struct image_raise_fall_edge *)mem_alloc(sizeof(struct image_raise_fall_edge) * cnt);
    if (buff == NULL)
        return 0;

    buff_end = pedge + cnt;
    buff_prev = buff + (max_edge - pedge);
    cur_edge = buff_prev;
    last_edge = max_edge;
    gray = (max_edge->amplitude + 2) >> 2;
    grad = (max_edge->max_grad + 1) >> 1;
    if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
        gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
    while (++last_edge < buff_end) {
        if (last_edge->amplitude < 5 && last_edge->end - last_edge->begin < 3)
            continue;

        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            ++cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude) >> 2;
            grad = (cur_edge->max_grad  + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
            if (gray > cur_edge->amplitude >> 1)
                gray = cur_edge->amplitude >> 1;
        }
    }
    buff_end = cur_edge;
    cur_edge = buff_prev;
    last_edge = max_edge;
    gray = (max_edge->amplitude + 2) >> 2;
    grad = (max_edge->max_grad + 1) >> 1;
    if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
        gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
    while (--last_edge >= pedge) {
        if (last_edge->amplitude < 5 && last_edge->end - last_edge->begin < 3)
            continue;

        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            --cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude) >> 2;
            grad = (cur_edge->max_grad + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
            if (gray > cur_edge->amplitude >> 1)
                gray = cur_edge->amplitude >> 1;
        }
    }
    cnt = (int)(buff_end - cur_edge + 1);
    memcpy(pedge, cur_edge, sizeof(struct image_raise_fall_edge) * cnt);
    mem_free(buff);

    cur_edge = pedge;
    last_edge = pedge + cnt;
    while (cur_edge < last_edge) {
        if (cur_edge->type == IMAGE_RFEDGE_TYPE_RAISE) {
            gray = cur_edge->max_gray;
            cur_edge->max_gray = cur_edge->min_gray;
            cur_edge->min_gray = gray;
        }
        cur_edge->dpos_256x = (cur_edge->dpos_256x << 8) / cur_edge->amplitude;
        cur_edge->dpos = (cur_edge->dpos_256x + (1 << 7)) >> 8;
        ++cur_edge;
    }

    return cnt;
}

unsigned int image_find_raise_fall_edges_pt2pt(
        const struct image *img, const struct point *start, const struct point *end,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    int i, j, step;
    struct point delta;
    struct point delta_abs;
    unsigned int cnt;
    unsigned int pos;
    unsigned char grad;
    unsigned char gray;
    unsigned char max_grad;
    unsigned char cur_type;
    unsigned char last_type;
    unsigned char max_amplitude;
    const unsigned char *imgdata;
    struct image_raise_fall_edge *cur_edge;
    struct image_raise_fall_edge *max_edge;
    struct image_raise_fall_edge *last_edge;
    struct image_raise_fall_edge *buff;
    struct image_raise_fall_edge *buff_end;
    struct image_raise_fall_edge *buff_prev;
    UNUSED int flag = 0;

    if (img == NULL || start == NULL || end == NULL || pedge == NULL || num == 0)
        return 0;

    if (start->x < 0 || start->x >= (int)img->width || start->y < 0 || start->y >= (int)img->height)
        return 0;

    if (end->x < 0 || end->x >= (int)img->width || end->y < 0 || end->y >= (int)img->height)
        return 0;

    if (start->x == end->x && start->y == end->y)
        return 0;

    cnt = 0;
    max_grad = 0;
    max_edge = NULL;
    max_amplitude = 0;
    cur_edge = pedge - 1;
    buff_end = pedge + num - 1;
    last_type = IMAGE_RFEDGE_TYPE_NONE;
    imgdata = img->data + start->y * img->width + start->x;
    gray = *imgdata;
    delta.x = end->x - start->x;
    delta.y = end->y - start->y;
    delta_abs.x = (int)fabs(delta.x);
    delta_abs.y = (int)fabs(delta.y);
    if (delta_abs.x >= delta_abs.y) {
        step = 1;
        if (delta.x < 0)
            step = -1;

        for (pos = 1, i = step; pos <= (unsigned int)delta_abs.x; i += step, ++pos) {
            j = (int)((((i * delta.y << 8) / delta.x + 128) >> 8) + start->y);
            imgdata = &img->data[img->width * j + start->x + i];
            ushow_pt(flag, start->x + i, j, YELLOWCOLOR);
            if (gray == *imgdata) {
                last_type = IMAGE_RFEDGE_TYPE_NONE;
                continue;
            } else if (gray < *imgdata) {
                grad = *imgdata - gray;
                cur_type = IMAGE_RFEDGE_TYPE_RAISE;
            } else {
                grad = gray - *imgdata;
                cur_type = IMAGE_RFEDGE_TYPE_FALL;
            }

            if (grad > max_grad)
                max_grad = grad;

            if (last_type != cur_type) {
                if (max_edge == NULL) {
                    max_edge = pedge;
                } else if (max_edge->max_grad <= cur_edge->max_grad
                        && max_edge->amplitude <= cur_edge->amplitude) {
                    max_edge = cur_edge;
                }

                if (cur_edge >= buff_end)
                    break;

                ++cur_edge;
                cur_edge->begin = pos - 1;
                cur_edge->end = pos;
                cur_edge->type = cur_type;
                cur_edge->max_grad = grad;
                cur_edge->min_grad = grad;
                cur_edge->amplitude = grad;
                last_type = cur_type;
                cur_edge->max_gray = gray;
                cur_edge->min_gray = *imgdata;
                cur_edge->dpos_256x = grad * pos;
            } else {
                cur_edge->end = pos;
                if (grad > cur_edge->max_grad) {
                    cur_edge->max_grad = grad;
                } else if (grad < cur_edge->min_grad) {
                    cur_edge->min_grad = grad;
                }
                cur_edge->min_gray = *imgdata;
                cur_edge->amplitude += grad;
                cur_edge->dpos_256x += grad * pos;
                if (cur_edge->amplitude > max_amplitude)
                    max_amplitude = cur_edge->amplitude;
            }
            gray = *imgdata;
        }
    } else {
        step = 1;
        if (delta.y < 0)
            step = -1;

        for (pos = 1, j = step; pos <= (unsigned int)delta_abs.y; j += step, ++pos) {
            i = (int)((((j * delta.x << 8) / delta.y + 128) >> 8) + start->x);
            imgdata = &img->data[img->width * (j + start->y) + i];
            ushow_pt(flag, i, j + start->y, YELLOWCOLOR);

            if (gray == *imgdata) {
                last_type = IMAGE_RFEDGE_TYPE_NONE;
                continue;
            } else if (gray < *imgdata) {
                grad = *imgdata - gray;
                cur_type = IMAGE_RFEDGE_TYPE_RAISE;
            } else {
                grad = gray - *imgdata;
                cur_type = IMAGE_RFEDGE_TYPE_FALL;
            }

            if (grad > max_grad)
                max_grad = grad;

            if (last_type != cur_type) {
                if (max_edge == NULL) {
                    max_edge = pedge;
                } else if (max_edge->max_grad <= cur_edge->max_grad
                        && max_edge->amplitude <= cur_edge->amplitude) {
                    max_edge = cur_edge;
                }

                if (cur_edge >= buff_end)
                    break;

                ++cur_edge;
                cur_edge->begin = pos - 1;
                cur_edge->end = pos;
                cur_edge->type = cur_type;
                cur_edge->max_grad = grad;
                cur_edge->min_grad = grad;
                cur_edge->amplitude = grad;
                last_type = cur_type;
                cur_edge->max_gray = gray;
                cur_edge->min_gray = *imgdata;
                cur_edge->dpos_256x = grad * pos;
            } else {
                cur_edge->end = pos;
                if (grad > cur_edge->max_grad) {
                    cur_edge->max_grad = grad;
                } else if (grad < cur_edge->min_grad) {
                    cur_edge->min_grad = grad;
                }
                cur_edge->min_gray = *imgdata;
                cur_edge->amplitude += grad;
                cur_edge->dpos_256x += grad * pos;
                if (cur_edge->amplitude > max_amplitude)
                    max_amplitude = cur_edge->amplitude;
            }
            gray = *imgdata;
        }
    }

    if (cur_edge < pedge)
        return 0;

    if (max_edge->amplitude <= 8)
        return 0;

    cnt = (unsigned int)(cur_edge - pedge + 1);
    buff = (struct image_raise_fall_edge *)mem_alloc(sizeof(struct image_raise_fall_edge) * cnt);
    if (buff == NULL)
        return 0;

    buff_end = pedge + cnt;
    buff_prev = buff + (max_edge - pedge);
    cur_edge = buff_prev;
    last_edge = max_edge;
    gray = (max_edge->amplitude + 2) >> 2;
    grad = (max_edge->max_grad + 1) >> 1;
    if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
        gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
    while (++last_edge < buff_end) {
        if (last_edge->amplitude < 5 && last_edge->end - last_edge->begin < 3)
            continue;

        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            ++cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude) >> 2;
            grad = (cur_edge->max_grad  + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
            if (gray > cur_edge->amplitude >> 1)
                gray = cur_edge->amplitude >> 1;
        }
    }
    buff_end = cur_edge;
    cur_edge = buff_prev;
    last_edge = max_edge;
    gray = (max_edge->amplitude + 2) >> 2;
    grad = (max_edge->max_grad + 1) >> 1;
    if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
        gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
    while (--last_edge >= pedge) {
        if (last_edge->amplitude < 5 && last_edge->end - last_edge->begin < 3)
            continue;

        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            --cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude) >> 2;
            grad = (cur_edge->max_grad + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
            if (gray > cur_edge->amplitude >> 1)
                gray = cur_edge->amplitude >> 1;
        }
    }
    cnt = (unsigned int)(buff_end - cur_edge + 1);
    memcpy(pedge, cur_edge, sizeof(struct image_raise_fall_edge) * cnt);
    mem_free(buff);

    cur_edge = pedge;
    last_edge = pedge + cnt;
    while (cur_edge < last_edge) {
        if (cur_edge->type == IMAGE_RFEDGE_TYPE_RAISE) {
            gray = cur_edge->max_gray;
            cur_edge->max_gray = cur_edge->min_gray;
            cur_edge->min_gray = gray;
        }
        cur_edge->dpos_256x = (cur_edge->dpos_256x << 8) / cur_edge->amplitude;
        cur_edge->dpos = (cur_edge->dpos_256x + (1 << 7)) >> 8;
        ++cur_edge;
    }

    return cnt;
}
