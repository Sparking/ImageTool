#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "maths.h"
#include "rf_edges.h"
#include "port_memory.h"

#define REF_GRAD(edge, n)   (((edge)->max_grad + (edge)->min_grad + 1) >> (n))
#define REF_AMP(edge)       (((edge)->amplitude << 1) / 5)
#define REF_MIN_LIMIT       5

unsigned int image_find_raise_fall_edges1(const unsigned char *imgdata, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    unsigned int cnt;
    unsigned int i, j, t;
    unsigned char cur_grad;
    unsigned char ref_grad;
    unsigned char last_gray;
    unsigned char ref_amplitude;
    unsigned char min_grad_limit;
    unsigned char new_cur_type;
    struct image_raise_fall_edge *cur_edge;
    struct image_raise_fall_edge *max_edge;
    struct image_raise_fall_edge *buff, *buff_ptr[2];
    struct image_raise_fall_edge *buff_prev, *buff_end;

    if (imgdata == NULL || len <= 1 || pedge == NULL || num == 0)
        return 0;

    cnt = sizeof(struct image_raise_fall_edge) * num;
    buff = (struct image_raise_fall_edge *)mem_alloc(cnt);
    if (buff == NULL)
        return 0;

    memset(buff, 0, cnt);
    memset(pedge, 0, cnt);
    cur_edge = pedge;
    cur_edge->begin = 0;
    cur_edge->end = 1;
    cur_edge->type = IMAGE_RFEDGE_TYPE_NONE;
    cur_edge->max_grad = 0;
    max_edge = cur_edge;
    last_gray = imgdata[0];
    for (i = 1; i < len; ++i) {
        if (imgdata[i] == last_gray) {
            continue;
        } else if (imgdata[i] < last_gray) {
            new_cur_type = IMAGE_RFEDGE_TYPE_FALL;
            cur_grad = last_gray - imgdata[i];
        } else {
            new_cur_type = IMAGE_RFEDGE_TYPE_RAISE;
            cur_grad = imgdata[i] - last_gray;
        }

        if (cur_edge->type == new_cur_type) {
            cur_edge->end = i;
            if (cur_edge->min_grad > cur_grad)
                cur_edge->min_grad = cur_grad;
            if (cur_edge->max_grad < cur_grad)
                cur_edge->max_grad = cur_grad;
        } else {
            cur_edge->amplitude =
                unsigned_diff(imgdata[cur_edge->end], imgdata[cur_edge->begin]);
            ref_grad = 0;
            ++cur_edge;
            if (cur_edge - pedge >= num)
                break;

            cur_edge->begin = i - 1;
            cur_edge->end = i;
            cur_edge->type = new_cur_type;
            cur_edge->max_grad = cur_grad;
            cur_edge->min_grad = cur_grad;
        }

        if (cur_edge->max_grad > max_edge->max_grad)
            max_edge = cur_edge;

        last_gray = imgdata[i];
    }

    if (max_edge->type == IMAGE_RFEDGE_TYPE_NONE) {
        mem_free(buff);
        return 0;
    }

    cnt = cur_edge - pedge + 1;
    //return cnt;
    cur_edge = pedge + cnt - 1;
    cur_edge->amplitude =
        unsigned_diff(imgdata[cur_edge->end], imgdata[cur_edge->begin]);
    buff_prev = buff + (max_edge - pedge);
    buff_ptr[0] = buff_prev;
    buff_ptr[1] = max_edge;
    buff_end = pedge + cnt;
    ref_amplitude = REF_AMP(max_edge);
    ref_grad = REF_GRAD(max_edge, 1);
    min_grad_limit = (max_edge->max_grad << 1) / 10;
    if (min_grad_limit < REF_MIN_LIMIT)
        min_grad_limit = REF_MIN_LIMIT;
    memcpy(buff_ptr[0], max_edge, sizeof(struct image_raise_fall_edge));
    while (++buff_ptr[1] < buff_end) {
        if (buff_ptr[1]->type == IMAGE_RFEDGE_TYPE_NONE)
            continue;

        if (buff_ptr[1]->type == buff_ptr[0]->type) {
            if (buff_ptr[1]->amplitude > buff_ptr[0]->amplitude
                && buff_ptr[1]->max_grad > (buff_ptr[0]->amplitude * 3) / 5) {
                memcpy(buff_ptr[0], buff_ptr[1], sizeof(struct image_raise_fall_edge));
            }
        } else if (buff_ptr[1]->max_grad >= min_grad_limit
            && (buff_ptr[1]->max_grad >= ref_grad || buff_ptr[1]->amplitude >= ref_amplitude)) {
            ++buff_ptr[0];
            memcpy(buff_ptr[0], buff_ptr[1], sizeof(struct image_raise_fall_edge));
            ref_amplitude = REF_AMP(buff_ptr[0]);
            ref_grad = REF_GRAD(buff_ptr[0], 1);
        }
    }
    buff_end = buff_ptr[0] + 1;

    buff_ptr[0] = buff_prev;
    buff_ptr[1] = max_edge;
    ref_amplitude = REF_AMP(max_edge);
    ref_grad = REF_GRAD(max_edge, 1);
    while (--buff_ptr[1] >= pedge) {
        if (buff_ptr[1]->type == IMAGE_RFEDGE_TYPE_NONE)
            continue;

        if (buff_ptr[1]->type == buff_ptr[0]->type) {
            if (buff_ptr[1]->amplitude > buff_ptr[0]->amplitude
                && buff_ptr[1]->max_grad > (buff_ptr[0]->amplitude * 3) / 5) {
                memcpy(buff_ptr[0], buff_ptr[1], sizeof(struct image_raise_fall_edge));
            }
        } else if ((buff_ptr[1]->max_grad >= ref_grad || buff_ptr[1]->amplitude >= ref_amplitude)
            && buff_ptr[1]->max_grad >= min_grad_limit) {
            --buff_ptr[0];
            memcpy(buff_ptr[0], buff_ptr[1], sizeof(struct image_raise_fall_edge));
            ref_amplitude = REF_AMP(buff_ptr[0]);
            ref_grad = REF_GRAD(buff_ptr[0], 1);
        }
    }
    cnt = buff_end - buff_ptr[0];
    memset(pedge + cnt, 0, sizeof(struct image_raise_fall_edge) * (num - cnt));
    memcpy(pedge, buff_ptr[0], sizeof(struct image_raise_fall_edge) * cnt);
    mem_free(buff);
    cur_edge = pedge;
    for (j = 0; j < cnt; ++j, ++cur_edge) {
        t = 0;
        for (i = cur_edge->begin; i < cur_edge->end; ++i) {
            cur_grad = unsigned_diff(imgdata[i], imgdata[i + 1]);
            t += cur_grad * i;
        }
        cur_edge->dpos = (((t << 4) / cur_edge->amplitude + 8) >> 4);
    }

    return cnt;
}

unsigned int image_find_raise_fall_edges(const unsigned char *imgdata, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    unsigned int cnt, i, j;
    unsigned char cur_grad;
    unsigned char last_gray;
    unsigned char cur_type;
    unsigned char last_type;
    struct image_raise_fall_edge *cur_edge;
    //struct image_raise_fall_edge *max_edge;

    if (imgdata == NULL || len <= 1 || pedge == NULL || num == 0)
        return 0;

    cnt = 0;
    //max_edge = NULL;
    cur_edge = pedge - 1;
    last_gray = imgdata[0];
    last_type = IMAGE_RFEDGE_TYPE_NONE;
    memset(pedge, 0, sizeof(*pedge) * num);
    for (i = 1; i < len; ++i) {
        if (last_gray == imgdata[i]) {
            last_type = IMAGE_RFEDGE_TYPE_NONE;
            continue;
        } else if (last_gray < imgdata[i]) {
            cur_grad = imgdata[i] - last_gray;
            cur_type = IMAGE_RFEDGE_TYPE_RAISE;
        } else {
            cur_grad = last_gray - imgdata[i];
            cur_type = IMAGE_RFEDGE_TYPE_FALL;
        }

        if (last_type != cur_type) {
            ++cur_edge;
            ++cnt;
            if (cnt >= num)
                break;

            cur_edge->begin = i - 1;
            cur_edge->end = i;
            cur_edge->type = cur_type;
            cur_edge->min_gray = imgdata[cur_edge->begin];
            cur_edge->max_grad = cur_grad;
            cur_edge->min_grad = cur_grad;

            cur_edge->dpos = cur_edge->begin;
            last_type = cur_type;
        } else {
            cur_edge->end = i;
            if (cur_grad > cur_edge->max_grad) {
                cur_edge->max_grad = cur_grad;
            } else if (cur_grad < cur_edge->min_grad) {
                cur_edge->min_grad = cur_grad;
            }
        }
        last_gray = imgdata[i];
    }

    cur_edge = pedge;
    for (j = 0; j < cnt; ++j, ++cur_edge) {
        cur_edge->dpos_256x = 0;
        cur_edge->amplitude = 0;
        for (i = cur_edge->begin; i < cur_edge->end; ++i) {
            assert(!(imgdata[i] < imgdata[i + 1] && cur_edge->type == IMAGE_RFEDGE_TYPE_FALL)
                && !(imgdata[i] > imgdata[i + 1] && cur_edge->type == IMAGE_RFEDGE_TYPE_RAISE));

            cur_grad = unsigned_diff(imgdata[i], imgdata[i + 1]);
            cur_edge->amplitude += cur_grad;
            cur_edge->dpos_256x += cur_grad * i;
        }
        cur_edge->dpos_256x = (cur_edge->dpos_256x << 8) / cur_edge->amplitude;
        cur_edge->dpos = (cur_edge->dpos_256x + (1 << 7)) >> 8;
    }

    return cnt;
}

