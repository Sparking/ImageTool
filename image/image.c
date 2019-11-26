﻿#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port_memory.h"
#include "image.h"
#include "bitmap.h"
#include "png.h"
#include "jpeglib.h"
#include "maths.h"
#include "stack.h"
#include "queue.h"

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

struct image *image_create(const unsigned int height, const unsigned int width, const unsigned int format)
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
    //return (unsigned char)(0.114f * blue + 0.578f * green + 0.299f * red);
    return (unsigned char)((38 * (unsigned short)red + 75 * (unsigned short)green + 15 * (unsigned short)blue) >> 7);
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
                    img->data[offset[1]] = image_rgb2gray(src_img->data[offset[0]],
                            src_img->data[offset[0] + 1], src_img->data[offset[0] + 2]);
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
                    img->data[offset[1]] = image_rgb2gray(src_img->data[offset[0]] + 2,
                            src_img->data[offset[0]] + 1, src_img->data[offset[0]]);
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
    unsigned int x, y;
    unsigned int row_offset;
    float m1, m2, n1, n2, sum, t, t0;
    const unsigned char *color;
    unsigned char i;

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

int image_gray_binarize(struct image *img, const unsigned char threshold, const unsigned char gray, const unsigned bg_gray)
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
    unsigned int i, j, row_offset;
    struct bitmatrix *matrix;

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY)
        return NULL;

    matrix = bitmatrix_create(img->height, img->width, 0);
    if (matrix == NULL)
        return NULL;

    for (j = 0, row_offset = 0; j < img->height; ++j, row_offset += img->row_size)
        for (i = 0; i < img->width; ++i)
            bitmatrix_set(matrix, j, i, img->data[row_offset + i]);

    return matrix;
}

struct image *image_create_from_bitmatrix(const struct bitmatrix *matrix)
{
    struct image *img;

    if (matrix == NULL)
        return NULL;

    img = image_create(matrix->row, matrix->column, IMAGE_FORMAT_GRAY);
    if (img == NULL)
        return NULL;

    for (unsigned int i, j = 0, row_offset = 0; j < img->height; ++j, row_offset += img->row_size) {
        for (i = 0; i < img->width; ++i) {
            if (bitmatrix_get(matrix, j, i)) {
                img->data[row_offset + i] = 0xFF;
            } else {
                img->data[row_offset + i] = 0;
            }
        }
    }

    return img;
}

struct image *image_open_jpeg(const char *jpeg_file)
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

int image_saveas_jpeg(const char *jpeg_file, const struct image *img)
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

unsigned int image_get_row4tbytes(const unsigned int width)
{
    if ((width & 0x03) == 0)
        return width;
    return ((width >> 2) + 1) << 2;
}

struct image *image_open_png(const char *png_file)
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

int image_saveas_png(const char *file, const struct image *img)
{
    int ret;
    FILE *fp;
    png_infop info;
    png_structp png;
    struct image *dump_img;
    unsigned char *ptr1;
    const unsigned char *ptr2;
    const unsigned char **rows;
    unsigned int i, j, offset, format;

    if (file == NULL || img == NULL)
        return -1;

    rows = (const unsigned char **)mem_alloc(img->height * sizeof(*rows));
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
            rows[j] = img->data + offset;
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

int image_saveas_bitmap(const char *file, const struct image *img)
{
    int ret;
    unsigned int j, i;
    unsigned char *dst_ptr[2];
    const unsigned char *src_ptr;
    struct bitmap_image *bmp_img;

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

int image_save(const char *file, const struct image *img, const int flag)
{
    int ret;

    switch (flag) {
    case IMAGE_FILE_BITMAP:
        ret = image_saveas_bitmap(file, img);
        break;
    case IMAGE_FILE_JPEG:
        ret = image_saveas_jpeg(file, img);
        break;
    case IMAGE_FILE_PNG:
        ret = image_saveas_png(file, img);
        break;
    default:
        ret = -1;
        break;
    }

    return ret;
}

struct image *image_open_bmp(const char *file)
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

    if (png_sig_cmp(checkheader, 0, 8) == 0) {
        img = image_open_png(file);
    } else if (checkheader[0] == 'B' && checkheader[1] == 'M') {
        img = image_open_bmp(file);
    } else if (checkheader[0] == 0xFF && checkheader[1] == 0xD8) {
        img = image_open_jpeg(file);
    } else {
        img = NULL;
    }

    return img;
}

int image_gray_blance(struct image *img, const unsigned char grad)
{
    unsigned int x, y;
    unsigned int hd[256] = {0};
    float p, q[256];

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY)
        return -1;

    for (y = 0; y < img->height; ++y) {
        unsigned int row_offset = y * img->width;
        for (x = 0; x < img->width; ++x) {
            ++hd[img->data[row_offset + x]];
        }
    }

    q[0] = (float)hd[0] / (float)img->size;
    for (x = 1; x < 256; ++x) {
        p = (float)hd[x] / (float)img->size;
        q[x] = q[x - 1] + p;
    }

    for (y = 0; y < img->height; ++y) {
        unsigned int row_offset = y * img->width;
        for (x = 0; x < img->width; ++x) {
            img->data[row_offset + x] = (unsigned char)(q[img->data[row_offset + x]] * grad);
        }
    }

    return 0;
}

struct image *image_sharpening(const struct image *img, const int *template1, const int *template2, const unsigned int dimension)
{
    unsigned int n = dimension >> 1;
    struct image *dump_img;

    if (img == NULL || img->format != IMAGE_FORMAT_GRAY || (template1 == NULL && template2 == NULL))
        return NULL;

    dump_img = image_create(img->height, img->width, IMAGE_FORMAT_GRAY);
    if (dump_img == NULL)
        return NULL;

    for (unsigned int x, y = n; y < img->height - n; ++y) {
        unsigned int _row_offset = y * img->row_size;
        for (x = n; x < img->width - n; ++x) {
            unsigned int s, t;
            unsigned int row_offset[2];
            int g[2] = {0, 0};

            row_offset[0] = 0;
            row_offset[1] = (y - n) * img->row_size + x - n;
            for (t = 0; t < dimension; ++t, row_offset[0] += dimension, row_offset[1] += img->row_size) {
                for (s = 0; s < dimension; ++s) {
                    if (template1) {
                        g[0] = g[0] + template1[row_offset[0] + s] * img->data[row_offset[1] + s];
                    }

                    if (template2) {
                        g[1] = g[1] + template2[row_offset[0] + s] * img->data[row_offset[1] + s];
                    }
                }
            }
            g[0] = ((unsigned int)(fabs(g[0]) + fabs(g[1]))) >> 1;  /* 线性融合, 参考opencv addWeighted */
            dump_img->data[_row_offset + x] = g[0] > 255.0f ? 255 : (unsigned char)g[0];
        }
    }

    return dump_img;
}

const static int sobel_fact[2][3][3] = {
    {{-1, -2, -1}, { 0, 0, 0}, { 1, 2, 1}},     /* 求Gy */
    {{-1,  0,  1}, {-2, 0, 2}, {-1, 0, 1}}};    /* 求Gx */

struct image *image_sobel_enhancing(const struct image *img, const int method)
{
    const int *hori, *vert;

    switch (method) {
    case 0:
        hori = sobel_fact[1][0];
        vert = sobel_fact[0][0];
        break;
    case 1:
        hori = sobel_fact[1][0];
        vert = NULL;
        break;
    case 2:
        hori = NULL;
        vert = sobel_fact[0][0];
        break;
    default:
        return NULL;
    }

    return image_sharpening(img, hori, vert, 3);
}

struct image *image_laplace_enhancing(const struct image *img, const unsigned int flag)
{
    const int laplace_fact[2][3][3] = {
        {{ 0, -1,  0}, {-1, 5, -1}, { 0, -1,  0}},
        {{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}}};

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
struct image *image_hough_transform(const struct image *img, const unsigned int threshold, const unsigned int gray)
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
    rho_max = (int)(sqrtf((float)(img->height * img->height + img->width * img->width)) + 0.5f);
    hough_param = (unsigned int *)mem_alloc(sizeof(unsigned int) * (rho_max * theta_n));
    if (hough_param == NULL) {
        image_release(hough_img);
        return NULL;
    }
    memset(hough_param, 0, sizeof(unsigned int) * (rho_max * theta_n));

    for (j = 0, offset = 0; j < img->height; ++j) {
        for (i = 0; i < img->width; ++i, ++offset) {
            if (img->data[offset] == gray) {
                for (theta_i = 0, current_theta = 0.0f; theta_i < theta_n; ++theta_i, current_theta += delta_theta) {
                    /* rho = x * cos(a) + y * sin(a) */
                    rho = (int)((float)j * sinf(current_theta) + (float)i * cosf(current_theta) + 0.5f);
                    if (rho >= 0 && rho < rho_max)
                        ++hough_param[rho * theta_n + theta_i];
                }
            }
        }
    }

    for (offset = 0, rho = 0; rho < rho_max; ++rho) {
        for (current_theta = 0.0f, theta_i = 0; theta_i < theta_n; ++offset, ++theta_i, current_theta += delta_theta) {
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

int image_nearest_interp(const struct image *src_img, unsigned char *new_pixel, const float x, const float y)
{
    int row, column;

    row = (int)(y + 0.5f);
    column = (int)(x + 0.5f);

    if (src_img == NULL || new_pixel == NULL || row < 0 || (unsigned int)row >= src_img->height
            || column < 0 || (unsigned int)column >= src_img->width)
        return -1;

    memcpy(new_pixel, &src_img->data[row * src_img->row_size + column * src_img->pixel_size],
            src_img->pixel_size);

    return 0;
}

int image_bilinear_interp(const struct image *src_img, unsigned char *new_pixel, const float x, const float y)
{
    unsigned int x_scale, y_scale, pixel_off;
    const unsigned char *pixel[4];

    if (src_img == NULL || new_pixel == NULL)
        return -1;

    if (x < 0 || ((unsigned int)x + 1) >= src_img->width || y < 0 || ((unsigned int)y + 1) >= src_img->height)
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

int image_bicubic_interp(const struct image *src_img, unsigned char *new_pixel, const float x, const float y)
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

int image_lanczos_interp(const struct image *src_img, unsigned char *new_pixel, const float x, const float y)
{
    float coeff[64];
    float coeff_x[8],coeff_y[8];
    int i,j, offset;
    float u, v;
    float g;
    const int a = 3;
    const float PI = acosf(-1.0);
    const unsigned char *pixel[65];
    memset(coeff,0,sizeof(coeff));
    memset(coeff_x,0,sizeof(coeff_x));
    memset(coeff_y,0,sizeof(coeff_y));
    if (src_img == NULL || new_pixel == NULL)
        return -1;

    if ((int)x < a-1 || (unsigned int)x >= src_img->width - a || (int)y < a-1 || (unsigned int)y >= src_img->height - a)
        return -1;

    pixel[64] = src_img->data + (((int)y) - a+1) * src_img->row_size + (((int)x) -a+1) * src_img->pixel_size;
    for(offset=0;offset<4*a*a;){
        pixel[offset++]=pixel[64];
        pixel[64]+=src_img->row_size;
        for(i=1;i<2*a;++i,++offset){
            pixel[offset]=pixel[offset-1]+src_img->pixel_size;
        }
    }
    u=x-(int)x;
    v=y-(int)y;
    for(i=0;i<2*a;++i){
        if(u+a-i-1==0)
            coeff_x[i]=1;
        else
            coeff_x[i]=(a*sinf(PI*(u+a-i-1))*sinf(PI*(u+a-i-1)/a))/(PI*PI*(u+a-i-1)*(u+a-i-1));
        
        if(v+a-i-1==0)
            coeff_y[i]=1;
        else
            coeff_y[i]=(a*sinf(PI*(v+a-i-1))*sinf(PI*(v+a-i-1)/a))/(PI*PI*(v+a-i-1)*(v+a-i-1));
    }
    offset=0;
    for(j=0;j<2*a;++j){
        for(i=0;i<2*a;++i)
            coeff[offset++]=coeff_x[i]*coeff_y[j];
    }

     for (offset = 0; offset < (int)src_img->pixel_size; ++offset) {
        g=0;
        for (i = 0; i < 4*a*a; ++i) {
            g += coeff[i]*pixel[i][offset];
        }
         if (g >= 255) {
            g = 255;
        } else if (g <= 0) {
            g = 0;
        }
        new_pixel[offset] = (unsigned char)g;
    }

    return 0;
}

struct image *image_perspective_transform(const struct image *img, const unsigned int nh, const unsigned int nw,
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

            if (image_bilinear_interp(img, pixel, (a[0] * i + a[1] * j + a[2]) / z, (a[3] * i + a[4] * j + a[5]) / z) == -1)
                continue;

            memcpy(&rect_img->data[offset + i * rect_img->pixel_size],
                pixel, rect_img->pixel_size);
        }
    }

    return rect_img;
}

struct image *image_rotation(const struct image *src_img, const struct point *rotation_center,
		const struct point *offset, const float theta,
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
