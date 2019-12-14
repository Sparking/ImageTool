#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <set>
#include "iniparser.h"
#include "port_memory.h"
#include "image.h"
#include "qr_decode.h"
#include "rf_edges.h"
#include "qr_position.h"

struct graph_config {
    char *file_path;
    char *qr_image_file;
    char *qr_info;
    unsigned int pm_line;

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

    value = ini_config_get(config, nullptr, "path", nullptr);
    if (value != nullptr)
        g_config.file_path = strdup(value);

    if ((section = ini_config_get_section(config, "QR Code")) != nullptr) {
        value = ini_config_get_key(section, "path", nullptr);
        if (value != nullptr)
            g_config.qr_image_file = strdup(value);

        value = ini_config_get_key(section, "info", nullptr);
        if (value != nullptr)
            g_config.qr_info = strdup(value);

        value = ini_config_get_key(section, "pm_line", nullptr);
        if (value != nullptr)
            g_config.pm_line = (unsigned int)atoi(value);
    }

    if ((section = ini_config_get_section(config, "Sobel")) != nullptr) {
        value = ini_config_get_key(section, "run", nullptr);
        if (value != nullptr && strcasecmp(value, "enable") == 0) {
            g_config.sobel_run = true;
        } else {
            g_config.sobel_run = false;
        }

        value = ini_config_get_key(section, "method", nullptr);
        if (value != nullptr) {
            if (strcasecmp(value, "hori") == 0) {
                g_config.sobel_method = 1;
            } else if (strcasecmp(value, "vert") == 0) {
                g_config.sobel_method = 2;
            } else if (strcasecmp(value, "all") == 0) {
                g_config.sobel_method = 0;
            } else {
                g_config.sobel_method = 0xFF;
            }
        } else {
            g_config.sobel_method = 0;
        }
    }

    ini_config_release(config);
    return 0;
}

int config_set(const char *filename)
{
    INI_CONFIG *config;

    config = ini_config_create(filename);
    if (config == nullptr) {
        std::cerr << "failed to open config file: " << filename << std::endl;
        return -1;
    }

    ini_config_set(config, "QR Code", "info", g_config.qr_info);
    ini_config_save(config);
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

struct image *image_sobel_enhancing1(const struct image *img, const unsigned int t)
{
	struct image *dimg;
	unsigned int i, j, i_end, off;

	if (img == nullptr)
		return nullptr;

	dimg = image_create(img->height, img->width, IMAGE_FORMAT_GRAY);
	if (dimg == nullptr)
		return nullptr;

	memset(dimg->data, 0, dimg->size);
	i_end = img->width - 1;
	for (off = 0; off < img->size; off += img->row_size) {
		for (i = 1; i < i_end; ++i) {
			j = unsigned_diff(img->data[off + i + 1], img->data[off + i]) + unsigned_diff(img->data[off + i - 1], img->data[off + i]);
			if (j >= 255) {
				dimg->data[off + i] = 255;
			} else {
				dimg->data[off + i] = j;
			}
		}
	}

	return dimg;
}

int image_scale_line(const struct image *img)
{
    FILE *fp;
    unsigned int cnt;
    struct image *simg;
    unsigned int i, j, soff;
    struct image_raise_fall_edge *rfe;

    if (img == nullptr)
        return -1;

    simg = image_create(img->height + 256 + 55 + 1, img->width, img->format);
    if (simg == nullptr)
        return -1;

    rfe = (struct image_raise_fall_edge *)mem_alloc(sizeof(struct image_raise_fall_edge) * 500);
    memset(simg->data, 0xFF, simg->size);
    soff = img->row_size * g_config.pm_line;
    for (j = 0; j < img->size; j += img->row_size) {
        if (j < soff) {
            memcpy(simg->data + j, img->data + j, img->row_size);
        } else {
            memcpy(simg->data + j, img->data + soff, img->row_size);
        }
    }

    {
#if 0
        cnt = image_find_raise_fall_edges(img->data + soff, simg->width, rfe, 500);
#else
        point s = {0, (int)g_config.pm_line};
        point off = {1, 0};
        cnt = image_find_raise_fall_edges_by_offset(img, s, off, 1000, rfe, 500);
#endif
        printf("cnt = %d\n", cnt);
    }

    j = img->size + 55 * simg->row_size;
    for (i = 0; i < simg->row_size; ++i) {
        simg->data[j + (255 - img->data[soff + i]) * simg->row_size + i] = 0;
    }
    j = img->size + (55 + 256) * simg->row_size;
    for (i = 0; i < cnt; ++i) {
        switch (rfe[i].type) {
        case IMAGE_RFEDGE_TYPE_FALL:
            //break;
        case IMAGE_RFEDGE_TYPE_RAISE:
            memset(simg->data + j + rfe[i].begin, 0x00, rfe[i].end - rfe[i].begin + 1);
#if 1
            for (unsigned int off = img->size; off < simg->size; off += simg->row_size)
                memset(simg->data + off + rfe[i].dpos, 0x00, 1);
#else
            for (unsigned int off = img->size; off < simg->size; off += simg->row_size)
                memset(simg->data + off + rfe[i].begin, 0x00, 1);
#endif
            break;
        case IMAGE_RFEDGE_TYPE_NONE:
            break;
        }
    }

    mem_free(rfe);
    fp = fopen("data.txt", "w");
    if (fp != nullptr) {
        fprintf(fp, "orgin gray:{\n");
        for (i = 0; i < simg->row_size; ++i) {
            fprintf(fp, "%3u", (unsigned int)img->data[soff + i]);
            if (i != simg->row_size - 1) {
                fprintf(fp, ",");
                if (((i + 1) & 0x7) == 0)
                    fprintf(fp, "\n");
                else
                    fprintf(fp, " ");
            }
        }
        fprintf(fp, "};\n");
        fflush(fp);
    }
    wave_image("test-wave.bmp", simg, g_config.pm_line, simg->width);
    image_save("test.bmp", simg, IMAGE_FILE_BITMAP);

    if (g_config.sobel_run) {
        struct image *sobel_img;

        sobel_img = image_sobel_enhancing1(simg, g_config.sobel_method);
        if (sobel_img != nullptr) {

            memset(sobel_img->data + img->size, 0xFF, (55 + 256 + 1) * simg->row_size);
            j = img->size + 55 * simg->row_size;
            for (i = 0; i < simg->row_size; ++i) {
                sobel_img->data[j + (255 - sobel_img->data[soff + i]) * simg->row_size + i] = 0;
            }

    if (fp != nullptr) {
        fprintf(fp, "sobel value: {\n");
        for (i = 0; i < simg->row_size; ++i) {
            fprintf(fp, "%3u", (unsigned int)sobel_img->data[soff + i]);
            if (i != simg->row_size - 1) {
                fprintf(fp, ",");
                if (((i + 1) & 0x7) == 0)
                    fprintf(fp, "\n");
                else
                    fprintf(fp, " ");
            }
        }
        fprintf(fp, "};\n");
        fflush(fp);
    }
            wave_image("sobel-wave.bmp", sobel_img, g_config.pm_line, sobel_img->width);
            image_save("sobel_img.bmp", sobel_img, IMAGE_FILE_BITMAP);
            image_release(sobel_img);
        }
    }
    image_release(simg);

    if (fp != nullptr)
        fclose(fp);

    return 0;
}

int image_draw_rfedges(const struct image *simg)
{
    struct image *img;
    struct image *dbg_img;
    unsigned int i, j, off[2], cnt, cnt1;
    struct image_raise_fall_edge rfe[2][500];
    const unsigned char re[4] = {0x98, 0x35, 0x95, 0xFF};
    const unsigned char fe[4] = {0x11, 0xbf, 0xF7, 0xFF};

    img = image_convert_format(simg, IMAGE_FORMAT_BGR);
    if (img == nullptr)
        return -1;

    dbg_img = image_dump(img);
    if (img == nullptr)
        return -1;

    for (j = 0, off[0] = off[1] = 0; j < img->height;
            ++j, off[0] += simg->row_size, off[1] += img->row_size) {
       cnt = image_find_raise_fall_edges(simg->data + off[0], simg->row_size, rfe[0], 500);
        for (i = 0; i < cnt; ++i) {
            if (rfe[0][i].type == IMAGE_RFEDGE_TYPE_RAISE) {
                memcpy(img->data + off[1] + rfe[0][i].dpos * 3, re, img->pixel_size);
            } else {
                memcpy(img->data + off[1] + rfe[0][i].dpos * 3, fe, img->pixel_size);
            }
        }
        point s = {0, (int)j};
        point off1 = {1, 0};
        cnt1 = image_find_raise_fall_edges_by_offset(simg, s, off1, 1000, rfe[1], 500);
        for (i = 0; i < cnt1; ++i) {
            if (rfe[1][i].type == IMAGE_RFEDGE_TYPE_RAISE) {
                memcpy(dbg_img->data + off[1] + rfe[1][i].dpos * 3, re, dbg_img->pixel_size);
            } else {
                memcpy(dbg_img->data + off[1] + rfe[1][i].dpos * 3, fe, dbg_img->pixel_size);
            }
        }
    }

    image_save("rfe.bmp", img, IMAGE_FILE_BITMAP);
    image_release(img);
    image_save("rfe_dbg.bmp", dbg_img, IMAGE_FILE_BITMAP);
    image_release(dbg_img);

    return 0;
}

int main(const int argc, char *argv[])
{
    image *img, *gray;
    char *decode_info;

    if (config_get("config.ini") == -1)
        return -1;

    if (g_config.qr_image_file == nullptr) {
        config_release();
        return -1;
    }

    img = image_open(g_config.qr_image_file);
    if (img == nullptr) {
        std::cerr << "failed to open image file: " << g_config.qr_image_file << std::endl;
        return -1;
    }

    gray = image_convert_gray(img);
    image_release(img);
    if (gray == nullptr)
        return -1;
    image_draw_rfedges(gray);
    img = image_dump(gray);
    if (img != nullptr) {
        image_gray_binarize(img, image_find_binariztion_global_threshold(gray), 0, 0xFF);
        image_save("bin.bmp", img, IMAGE_FILE_BITMAP);
        image_release(img);
    }

    img = image_sobel_enhancing(gray);
    if (img != nullptr) {
        image_save("sobel.bmp", img, IMAGE_FILE_BITMAP);
        image_release(img);
    }

    img = gray;
    image_scale_line(img);
    decode_info = qr_decode_info(img);
    image_release(img);
    if (decode_info != nullptr) {
        printf("%s\n", decode_info);
        if (g_config.qr_info == nullptr) {
            g_config.qr_info = strdup(decode_info);
        } else if (strcmp(decode_info, g_config.qr_info) != 0) {
            free(g_config.qr_info);
            g_config.qr_info = strdup(decode_info);
        }
        mem_free(decode_info);
    } else if (g_config.qr_info != nullptr) {
        free(g_config.qr_info);
        g_config.qr_info = nullptr;
    }
    config_set("config.ini");
    config_release();

    return 0;
}
