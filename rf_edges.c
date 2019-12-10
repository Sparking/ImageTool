#include <stdlib.h>
#include <string.h>
#include "maths.h"
#include "rf_edges.h"
#include "port_memory.h"
#include <stdio.h>
#include <assert.h>

#define REF_GRAD(edge, n)  (((edge)->max_grad + (edge)->min_grad + 1) >> (n))
#define REF_AMP(edge)   (((edge)->amplitude << 1) / 5)

unsigned int image_find_raise_fall_edges(const unsigned char *imgdata, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    unsigned int i, j, r, t;
    unsigned int cnt;
    unsigned char cur_grad;
    unsigned char ref_grad;
    unsigned char last_gray;
    unsigned char ref_amplitude;
    unsigned char new_edge_type;
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
    //cnt = 1;
    cur_edge = pedge;
    cur_edge->len = 1;
    cur_edge->begin_pos = 0;
    cur_edge->type = IMAGE_RFEDGE_TYPE_NONE;
    cur_edge->max_grad = 0;
    cur_edge->min_grad = 0;
    cur_edge->amplitude = 0;
    max_edge = cur_edge;
    last_gray = imgdata[0];
    for (i = 1; i < len; ++i) {
        cur_grad = unsigned_diff(last_gray, imgdata[i]);
        if (imgdata[i] == last_gray) {
            continue;
        } else if (imgdata[i] < last_gray) {
            new_edge_type = IMAGE_RFEDGE_TYPE_FALL;
        } else {
            new_edge_type = IMAGE_RFEDGE_TYPE_RAISE;
        }

        if (cur_edge->type == new_edge_type) {
            if (cur_edge->min_grad > cur_grad)
                cur_edge->min_grad = cur_grad;
            if (cur_edge->max_grad < cur_grad)
                cur_edge->max_grad = cur_grad;
            ++cur_edge->len;
        } else {
            cur_edge->amplitude =
                unsigned_diff(imgdata[cur_edge->begin_pos + cur_edge->len - 1],
                        imgdata[cur_edge->begin_pos]);
            ++cur_edge;
            //++cnt;
            if (cur_edge - pedge >= num)
                break;

            cur_edge->len = 2;
            cur_edge->begin_pos = i - 1;
            cur_edge->type = new_edge_type;
            cur_edge->max_grad = cur_grad;
            cur_edge->min_grad = cur_grad;
            cur_edge->amplitude = imgdata[cur_edge->begin_pos];
        }

        if (cur_grad != 0 &&cur_edge->max_grad > max_edge->max_grad)
            max_edge = cur_edge;

        last_gray = imgdata[i];
    }

    if (max_edge->type == IMAGE_RFEDGE_TYPE_NONE) {
        mem_free(buff);
        return 0;
    }

    cnt = cur_edge - pedge + 1;
    cur_edge = pedge + cnt - 1;
    cur_edge->amplitude =
        unsigned_diff(imgdata[cur_edge->begin_pos + cur_edge->len - 1],
        imgdata[cur_edge->begin_pos]);
    buff_prev = buff + (max_edge - pedge);
    buff_ptr[0] = buff_prev;
    buff_ptr[1] = max_edge;
    buff_end = pedge + cnt;
    ref_amplitude = REF_AMP(max_edge);
    ref_grad = REF_GRAD(max_edge, 1);
    memcpy(buff_ptr[0], max_edge, sizeof(struct image_raise_fall_edge));
    while (++buff_ptr[1] < buff_end) {
        if (buff_ptr[1]->max_grad < ref_grad && buff_ptr[1]->amplitude < ref_amplitude) {
        	continue;
        } else {
            ++buff_ptr[0];
            memcpy(buff_ptr[0], buff_ptr[1], sizeof(struct image_raise_fall_edge));
            ref_grad = REF_GRAD(buff_ptr[0], 1);
            ref_amplitude = REF_AMP(buff_ptr[0]);
        }
    }
    buff_end = buff_ptr[0] + 1;
    buff_ptr[0] = buff_prev;
    buff_ptr[1] = max_edge;
    ref_amplitude = REF_AMP(max_edge);
    ref_grad = REF_GRAD(max_edge, 1);
    while (--buff_ptr[1] >= pedge) {
        if (buff_ptr[1]->max_grad < ref_grad && buff_ptr[1]->amplitude < ref_amplitude) {
        	continue;
        } else {
            --buff_ptr[0];
            memcpy(buff_ptr[0], buff_ptr[1], sizeof(struct image_raise_fall_edge));
            ref_grad = REF_GRAD(buff_ptr[0], 1);
            ref_amplitude = REF_AMP(buff_ptr[0]);
        }
    }
    cnt = buff_end - buff_ptr[0];
    memset(pedge + cnt, 0, sizeof(struct image_raise_fall_edge) * (num - cnt));
    memcpy(pedge, buff_ptr[0], sizeof(struct image_raise_fall_edge) * cnt);
    mem_free(buff);
    for (j = 0; j < cnt; ++j) {
        t = 0;
        r = pedge[j].begin_pos + pedge[j].len;
        for (i = pedge[j].begin_pos + 1; i < r; ++i) {
            t += unsigned_diff(imgdata[i - 1], imgdata[i]) * i;
        }
        pedge[j].dpos = t / pedge[j].amplitude;
    }

    return cnt;
}
