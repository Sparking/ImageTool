#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "maths.h"
#include "rf_edges.h"
#include "port_memory.h"

#define IMAGE_RFEDGE_AMP_LIMIT_MIN      15

unsigned int image_find_raise_fall_edges(const unsigned char *imgdata, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    unsigned int cnt, i;
    unsigned char grad;
    unsigned char gray;
    unsigned char cur_type;
    unsigned char last_type;
    struct image_raise_fall_edge *cur_edge;
    struct image_raise_fall_edge *max_edge;
    struct image_raise_fall_edge *last_edge;
    struct image_raise_fall_edge *buff;
    struct image_raise_fall_edge *buff_end;
    struct image_raise_fall_edge *buff_prev;

    if (imgdata == NULL || len <= 1 || pedge == NULL || num == 0)
        return 0;

    cnt = 0;
    max_edge = NULL;
    cur_edge = pedge - 1;
    gray = imgdata[0];
    last_type = IMAGE_RFEDGE_TYPE_NONE;
    for (i = 1; i < len; ++i) {
        if (gray == imgdata[i]) {
            last_type = IMAGE_RFEDGE_TYPE_NONE;
            continue;
        } else if (gray < imgdata[i]) {
            grad = imgdata[i] - gray;
            cur_type = IMAGE_RFEDGE_TYPE_RAISE;
        } else {
            grad = gray - imgdata[i];
            cur_type = IMAGE_RFEDGE_TYPE_FALL;
        }

        if (last_type != cur_type) {
            if (max_edge == NULL) {
                max_edge = pedge;
            } else if (max_edge->max_grad <= cur_edge->max_grad
                    && max_edge->amplitude <= cur_edge->amplitude) {
                max_edge = cur_edge;
            }

            ++cur_edge;
            ++cnt;
            if (cnt >= num)
                break;

            cur_edge->begin = i - 1;
            cur_edge->end = i;
            cur_edge->type = cur_type;
            cur_edge->max_grad = grad;
            cur_edge->min_grad = grad;
            cur_edge->amplitude = grad;
            last_type = cur_type;
            cur_edge->max_gray = gray;
            cur_edge->min_gray = imgdata[i];
            cur_edge->dpos_256x = grad * i;
        } else {
            cur_edge->end = i;
            if (grad > cur_edge->max_grad) {
                cur_edge->max_grad = grad;
            } else if (grad < cur_edge->min_grad) {
                cur_edge->min_grad = grad;
            }
            cur_edge->min_gray = imgdata[i];
            cur_edge->amplitude += grad;
            cur_edge->dpos_256x += grad * i;
        }
        gray = imgdata[i];
    }

    if (cnt == 0)
        return 0;

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
        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            ++cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude + 2) >> 2;
            grad = (cur_edge->max_grad + cur_edge->min_grad + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
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
        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            --cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude + 2) >> 2;
            grad = (cur_edge->max_grad + cur_edge->min_grad + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
        }
    }
    cnt = buff_end - cur_edge;
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

unsigned int image_find_raise_fall_edges_by_offset(
        const struct image *img, const struct point pstart,
        const struct point setup_off, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    int imgdata_off;
    struct point off_end;
    unsigned int cnt, i;
    unsigned char grad;
    unsigned char gray;
    unsigned char cur_type;
    unsigned char last_type;
    const unsigned char *imgdata;
    struct image_raise_fall_edge *cur_edge;
    struct image_raise_fall_edge *max_edge;
    struct image_raise_fall_edge *last_edge;
    struct image_raise_fall_edge *buff;
    struct image_raise_fall_edge *buff_end;
    struct image_raise_fall_edge *buff_prev;

    if (img == NULL || len <= 1 || pedge == NULL || num == 0)
        return 0;

    cnt = 0;
    max_edge = NULL;
    cur_edge = pedge - 1;
    off_end.x = pstart.x + setup_off.x;
    off_end.y = pstart.y + setup_off.y;
    imgdata_off = setup_off.y * img->width + setup_off.x;
    imgdata = img->data + pstart.y * img->width + pstart.x;
    gray = *imgdata;
    imgdata += imgdata_off;
    last_type = IMAGE_RFEDGE_TYPE_NONE;
    for (i = 1; i < len && off_end.x >= 0 && off_end.x < img->width && off_end.y >= 0 && off_end.y < img->height;
            ++i, imgdata += imgdata_off, off_end.x += setup_off.x, off_end.y += setup_off.y) {
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

        if (last_type != cur_type) {
            if (max_edge == NULL) {
                max_edge = pedge;
            } else if (max_edge->max_grad <= cur_edge->max_grad
                    && max_edge->amplitude <= cur_edge->amplitude) {
                max_edge = cur_edge;
            }

            ++cur_edge;
            if ((cur_edge - pedge) >= num)
                break;

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
        }
        gray = *imgdata;
    }

    if (cur_edge < pedge)
        return 0;

    cnt = cur_edge - pedge;
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
        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            ++cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude + 2) >> 2;
            grad = (cur_edge->max_grad + cur_edge->min_grad + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
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
        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            --cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude + 2) >> 2;
            grad = (cur_edge->max_grad + cur_edge->min_grad + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
        }
    }
    cnt = buff_end - cur_edge + 1;
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
