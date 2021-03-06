﻿#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "image.h"
#include "iniparser.h"
#include "qr_decode.h"
#include "generic_gf.h"
#include "rsdecode.h"
#include "port_memory.h"
#include "qr_position.h"
#include "dotcode_detect_point.h"

struct graph_config {
    char *file_path;
    unsigned int file_column;
    char *qr_image_file;
    char *qr_info;
    unsigned int qr_line;

    bool sobel_run;
    unsigned char sobel_method;
} g_config;

char *qr_decode_info(const struct image *img)
{
    qr_decode_entry(img);

    return nullptr;
}

int config_get(const char *filename)
{
    const char *value;
    INI_CONFIG *config;
    void *section;

    memset(&g_config, 0, sizeof(g_config));
    config = ini_config_create(filename);
    if (config == nullptr) {
        std::cerr << "failed to open config file: " << filename << std::endl;
        return -1;
    }

    if ((section = ini_config_get_section(config, nullptr)) != nullptr
            && (value = ini_config_get_key(section, "column", nullptr)) != nullptr) {
        g_config.file_path = strdup(value);
        g_config.file_column = atoi(ini_config_get_key(section, "column", "0"));
    } else {
        g_config.file_path = nullptr;
        g_config.file_column = 0;
    }

    if ((section = ini_config_get_section(config, "QR Code")) != nullptr) {
        value = ini_config_get_key(section, "path", nullptr);
        if (value != nullptr)
            g_config.qr_image_file = strdup(value);

        value = ini_config_get_key(section, "info", nullptr);
        if (value != nullptr)
            g_config.qr_info = strdup(value);

        value = ini_config_get_key(section, "line", nullptr);
        if (value != nullptr)
            g_config.qr_line = (unsigned int)atoi(value);
    }
    ini_config_release(config);

    return 0;
}

void config_release(void)
{
    if (g_config.file_path != nullptr)
        free(g_config.file_path);
    if (g_config.qr_image_file != nullptr)
        free(g_config.qr_image_file);
    if (g_config.qr_info != nullptr)
        free(g_config.qr_info);
    memset(&g_config, 0, sizeof(g_config));
}

void wave_image(const char *name, const struct image *srcimg,
        const unsigned int height, const unsigned int width)
{
    unsigned int i, j, off;
    unsigned char color[4] = {0x00, 0x00, 0x00, 0xFF};
    const unsigned int w = 4;
    const unsigned char *gray = srcimg->data + height * srcimg->width;

    struct image *img = image_create(256, width * w, IMAGE_FORMAT_BGR);
    if (img == nullptr)
        return;

    memset(img->data, 0xFF, img->size);
    for (i = 0; i < width; ++i) {
        off = (255 - gray[i]) * img->row_size + i * w * img->pixel_size;
        for (j = 0; j < w; ++j) {
            memcpy(img->data + off + j * img->pixel_size, color, img->pixel_size);
        }
    }

    image_save(name, img, IMAGE_FILE_BITMAP);
    image_release(img);
}

int image_scale_line(const struct image *img)
{

    unsigned int cnt;
    struct image *simg;
    unsigned int i, j, soff;
    struct image_raise_fall_edge *rfe;
    const struct point s_off = {1, 0};
    const struct point s_start = {0, (int)g_config.qr_line};

    if (img == nullptr)
        return -1;

    simg = image_create(img->height + 256 + 55 + 1, img->width, img->format);
    if (simg == nullptr)
        return -1;

    rfe = (struct image_raise_fall_edge *)mem_alloc(sizeof(struct image_raise_fall_edge) * 500);
    memset(simg->data, 0xFF, simg->size);
    soff = img->row_size * g_config.qr_line;
    for (j = 0; j < img->size; j += img->row_size) {
        if (j < soff) {
            memcpy(simg->data + j, img->data + j, img->row_size);
        } else {
            memcpy(simg->data + j, img->data + soff, img->row_size);
        }
    }

    cnt = image_find_raise_fall_edges_by_offset(img, &s_start, &s_off, 1000, rfe, 500);
    j = img->size + 55 * simg->row_size;
    for (i = 0; i < simg->row_size; ++i) {
        simg->data[j + (255 - img->data[soff + i]) * simg->row_size + i] = 0;
    }
    j = img->size + (55 + 256) * simg->row_size;
    for (i = 0; i < cnt; ++i) {
        switch (rfe[i].type) {
        case IMAGE_RFEDGE_TYPE_FALL:
        case IMAGE_RFEDGE_TYPE_RAISE:
            memset(simg->data + j + rfe[i].dpos, 0x00, rfe[i].end - rfe[i].dpos + 1);
            for (unsigned int off = img->size; off < simg->size; off += simg->row_size)
                memset(simg->data + off + rfe[i].dpos, 0x00, 1);
            break;
        case IMAGE_RFEDGE_TYPE_NONE:
            break;
        }
    }

    mem_free(rfe);
    wave_image("test-wave.bmp", simg, g_config.qr_line, simg->width);
    image_save("test.bmp", simg, IMAGE_FILE_BITMAP);
    image_release(simg);

    return 0;
}

int image_scan_column(void)
{
    unsigned int cnt;
    unsigned int i, j, copy_size, off;
    struct image *simg, *img, *tmpimg;
    struct image_raise_fall_edge *rfe;
    const struct point setup = {0, 1};
    const struct point start_pt = {(int)g_config.file_column, 0};

    if ((simg = image_open(g_config.qr_image_file)) == nullptr)
        return -1;

    if ((img = image_convert_gray(simg)) == nullptr) {
        image_release(simg);
        return -1;
    }

    image_release(simg);
    tmpimg = image_convert_format(img, IMAGE_FORMAT_BGR);
    if (tmpimg == nullptr)
        return -1;

    simg = image_create(img->height, img->width + 55 + 256 + 2, IMAGE_FORMAT_BGR);
    if (simg == nullptr) {
        image_release(tmpimg);
        image_release(img);
        return -1;
    }
    memset(simg->data, 0xFF, simg->size);

    if (g_config.file_column >= simg->width || g_config.file_column <= 0)
        g_config.file_column = simg->width;

    copy_size = g_config.file_column * simg->pixel_size;
    for (i = 0, off = 0; i < tmpimg->height; ++i, off += simg->row_size) {
        memcpy(simg->data + off,
            tmpimg->data + i * tmpimg->row_size, copy_size);
        for (j = g_config.file_column; j < tmpimg->width; ++j) {
            memcpy(simg->data + off + j * simg->pixel_size,
                simg->data + off + (j - 1) * simg->pixel_size, simg->pixel_size);
        }

        /**波形图**/
        j = tmpimg->width + 55 + 256 - img->data[i * img->width + g_config.file_column];
        j *= simg->pixel_size;
        memset(simg->data + off + j, 0, simg->pixel_size);
    }
    image_release(tmpimg);

    rfe = (struct image_raise_fall_edge *)mem_alloc(sizeof(struct image_raise_fall_edge) * 500);
    cnt = image_find_raise_fall_edges_by_offset(img, &start_pt, &setup, 1000, rfe, 500);
    for (i = 0; i < cnt; ++i) {
        switch (rfe[i].type) {
        case IMAGE_RFEDGE_TYPE_FALL:
        case IMAGE_RFEDGE_TYPE_RAISE:
            memset(simg->data + rfe[i].dpos * simg->row_size + img->width * simg->pixel_size,
                0x00, (simg->width - img->width) * simg->pixel_size);
            for (j = rfe[i].dpos + 1; j < rfe[i].end; ++j)
                memset(simg->data - simg->pixel_size + j * simg->row_size, 0x00, simg->pixel_size);
            break;
        case IMAGE_RFEDGE_TYPE_NONE:
            break;
        }
    }
    mem_free(rfe);

    image_save("column.bmp", simg, IMAGE_FILE_BITMAP);
    image_release(simg);
    image_release(img);

    return 0;
}

int image_draw_rfedges(const struct image *simg)
{
    struct image *img;
    struct image *dbg_img;
    struct point start;
    struct point xoff = {0, 1};
    unsigned int i, j, cnt, cnt1;
    struct image_raise_fall_edge rfe[2][500];
    const unsigned char re[4] = {0x98, 0x35, 0x95, 0xFF};
    const unsigned char fe[4] = {0x11, 0xbf, 0xF7, 0xFF};

    img = image_convert_format(simg, IMAGE_FORMAT_BGR);
    if (img == nullptr)
        return -1;

    dbg_img = image_dump(img);
    if (img == nullptr)
        return -1;

    for (j = 0; j < img->width; ++j) {
        start.x = j;
        start.y = 0;
        cnt = image_find_raise_fall_edges_by_offset(simg, &start, &xoff, 1000, rfe[0], 500);
        for (i = 0; i < cnt; ++i) {
            if (rfe[0][i].type == IMAGE_RFEDGE_TYPE_RAISE) {
                memcpy(img->data + rfe[0][i].dpos * img->row_size + j * 3, re, img->pixel_size);
            } else {
                memcpy(img->data + rfe[0][i].dpos * img->row_size + j * 3, fe, img->pixel_size);
            }
        }
        point s = {(int)j, 0};
        point off1 = {s.x, (int)img->height - 1};
        cnt1 = image_find_raise_fall_edges_pt2pt(simg, &s, &off1, rfe[1], 500);
        for (i = 0; i < cnt1; ++i) {
            if (rfe[1][i].type == IMAGE_RFEDGE_TYPE_RAISE) {
                memcpy(dbg_img->data + rfe[1][i].dpos * dbg_img->row_size + j * 3, re, dbg_img->pixel_size);
            } else {
                memcpy(dbg_img->data + rfe[1][i].dpos * dbg_img->row_size + j * 3, fe, dbg_img->pixel_size);
            }
        }
    }

    image_save("rfe.bmp", img, IMAGE_FILE_BITMAP);
    image_save("rfe_dbg.bmp", dbg_img, IMAGE_FILE_BITMAP);
    image_release(img);
    image_release(dbg_img);

    return 0;
}

unsigned char imgdata[320];

static int compare(const void *a, const void *b)
{
    if (*(unsigned char *)b == *(unsigned char *)a)
        return 0;

    return *(unsigned char *)b > *(unsigned char *)a ? 1 : -1;
}

int image_correct_dot_pos(unsigned char *imgdata, const int len, const int pos, int *new_center)
{
    int i, nuppos;
    unsigned char grads[100];
    unsigned int uppos[100];

    /**转为梯度**/
    for (i = 1; i < len; ++i) {
        imgdata[i - 1] = unsigned_diff(imgdata[i], imgdata[i - 1]);
        printf("%3d ", imgdata[i]);
    }
    putchar('\n');
    memcpy(grads, imgdata, len - 1);
    qsort(grads, len - 1, sizeof(unsigned char), compare);
    for (i = 0; i < len - 1; ++i)
        printf("%3d ", imgdata[i]);
    putchar('\n');
    for (i = 0; i < len - 1; ++i)
        printf("%3d ", grads[i]);
    putchar('\n');

    if (imgdata[0] >= 10) {
    }
    for (nuppos = 0, i = 1; i < len - 2; ++i) {
        if (imgdata[i] >= 10) {
            if ((imgdata[i] > imgdata[i - 1] && imgdata[i] >= imgdata[i + 1])
                || (imgdata[i] >= imgdata[i - 1] && imgdata[i] > imgdata[i + 1])) {
                uppos[nuppos++] = i;
            }
        }
    }

    for (i = 0; i < nuppos; ++i) {
        printf("%3d ", (char)imgdata[uppos[i]]);
    }
    printf("\n");

    return 0;
}

int image_find_dot_by_grad(const struct image *srcimg, const struct point *pt, const int len)
{
    int i, j, n;
    struct image *gray, *img;

    gray = image_convert_gray(srcimg);
    if (gray == nullptr)
        return -1;

    img = image_convert_format(gray, IMAGE_FORMAT_BGRA);
    if (img == nullptr) {
        image_release(gray);
        return -1;
    }

    n = 0;
    i = (pt->y + 1) * gray->width + pt->x - len - 1;
    j = (pt->y + 1) * gray->width + pt->x + len + 1;
    while (i <= j)
        imgdata[n++] = gray->data[i++];

    if (image_correct_dot_pos(imgdata, n, len + 1, &i)) {
        printf("%d, %d\n", pt->x + len + 1, pt->x + i);
    } else {
        printf("not found center\n");
    }
    image_release(gray);
    image_release(img);

    return 0;
}

int main(const int argc, char *argv[])
{
#if 0
    image *img, *gray;

    if (config_get("config.ini") == -1)
        return -1;

    if (g_config.qr_image_file == nullptr) {
        config_release();
        return -1;
    }

    image_scan_column();
    img = image_open(g_config.qr_image_file);
    if (img == nullptr) {
        std::cerr << "failed to open image file: " << g_config.qr_image_file << std::endl;
        return -1;
    }

    gray = image_convert_gray(img);
    image_draw_rfedges(gray);
    image_release(img);
    if (gray == nullptr)
        return -1;

    //qr_decode_info(gray);
    image_scale_line(gray);
#if 1
    struct dotcode_point w[10000];
    unsigned int nxx = dotcode_detect_point(gray, w, 10000);
    //printf("%d\n", nxx);
    for (unsigned int x = 0; x < nxx; ++x) {
        //printf("%d, %d (%d)\n", w[x].center.x, w[x].center.y, w[x].weight);
        if (w[x].isblack)
            *(gray->data + w[x].center.y * gray->width + w[x].center.x) = 0xFF;
        else
            *(gray->data + w[x].center.y * gray->width + w[x].center.x) = 0;
    }
    image_save("white-dots.bmp", gray, IMAGE_FILE_BITMAP);
#endif
    image_release(gray);
    config_release();
//#else
    struct image *img;
    struct point pt = {311, 170};

    if (argc < 2)
        return -1;

    if ((img = image_open(argv[1])) == NULL)
        return -1;

    image_find_dot_by_grad(img, &pt, 11);
    image_release(img);
#endif
    struct generic_gf *gf_dotcode;
    unsigned int eccdata[59] = {1,12,37,62,87,113,98,113,91,93,109,4,113,5,113,112,15,18,6,6,27,15,20,41,23,45,52,53,35,57,64,65,47,69,74,79,59,81,52,22,71,103,10,82,93,73,17,102,4,19,84,95,90,8,73,33,84,69,59};

    gf_dotcode = generic_gf_create(113, 113, 0);
    if (rsdecode(gf_dotcode, eccdata, 59, 21) == 0) {
        printf("success\n");
    } else {
        printf("failed\n");
    }
    generic_gf_release(gf_dotcode);

    return 0;
}
