#include <stdlib.h>
#include <string.h>
#include "maths.h"
#include "rf_edges.h"
#include "port_memory.h"
#include <stdio.h>

unsigned int image_find_raise_fall_edges(const unsigned char *imgdata, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    unsigned int cnt;
    unsigned int pos;
    unsigned char cur_grad;
    unsigned char max_grad;
    unsigned char min_grad;
    unsigned char last_gray;
    struct image_raise_fall_edge *cur_edge;
    struct image_raise_fall_edge *max_edge;
    struct image_raise_fall_edge *buff, *buff_ptr[2];
    struct image_raise_fall_edge *buff_prev, *buff_end;
    image_raise_fall_edge_type new_edge_type;

    if (imgdata == NULL || pedge == NULL || len == 0 || num == 0)
        return 0;

    cnt = sizeof(struct image_raise_fall_edge) * num;
    buff = (struct image_raise_fall_edge *)mem_alloc(cnt);
    if (buff == NULL)
        return 0;

    memset(buff, 0, cnt);
    memset(pedge, 0, cnt);
    cnt = 0;
    max_grad = 0;
    min_grad = 255;
    cur_edge = pedge;
    cur_edge->len = 1;
    cur_edge->max_grad = 0;
    cur_edge->min_grad = 0;
    cur_edge->begin_pos = 0;
    cur_edge->type = IMAGE_RFEDGE_TYPE_NONE;
    max_edge = cur_edge;
    last_gray = *imgdata;
    for (pos = 1; (pos < len) && (cnt < num); ++pos) {
        cur_grad = unsigned_diff(last_gray, imgdata[pos]);
        if (max_grad < cur_grad)
            max_grad = cur_grad;
        if (min_grad > cur_grad)
            min_grad = cur_grad;

        if (imgdata[pos] == last_gray) {
            continue;
        } else if (imgdata[pos] < last_gray) {
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
            ++cnt;
            ++cur_edge;
            if (cnt >= num)
                break;

            cur_edge->type = new_edge_type;
            cur_edge->len = 1;
            cur_edge->begin_pos = pos;
            cur_edge->max_grad = cur_grad;
            cur_edge->min_grad = cur_grad;
        }

        if (cur_grad != 0 &&cur_edge->max_grad > max_edge->max_grad)
            max_edge = cur_edge;

        last_gray = imgdata[pos];
    }

    buff_prev = buff + (max_edge - pedge);
    buff_ptr[0] = buff_prev;
    memcpy(buff_ptr[0], max_edge, sizeof(struct image_raise_fall_edge));
    buff_ptr[1] = max_edge;
    buff_end = pedge + cnt;
    while (++buff_ptr[1] < buff_end) {
        if (buff_ptr[1]->type == buff_ptr[0]->type
            ||  (buff_ptr[1]->len < 3 && buff_ptr[1]->max_grad < (buff_ptr[0]->max_grad +  buff_ptr[0]->min_grad + 1) >> 2)) {
            buff_ptr[0]->len = buff_ptr[1]->begin_pos - buff_ptr[0]->begin_pos + buff_ptr[1]->len;
        } else {
            ++buff_ptr[0];
            memcpy(buff_ptr[0], buff_ptr[1], sizeof(struct image_raise_fall_edge));
        }
    }
    buff_end = buff_ptr[0];
    buff_ptr[0] = buff_prev;
    buff_ptr[1] = max_edge;
    while (--buff_ptr[1] >= pedge) {
        if (buff_ptr[1]->type == buff_ptr[0]->type
            ||  (buff_ptr[1]->len < 3 && buff_ptr[1]->max_grad < (buff_ptr[0]->max_grad +  buff_ptr[0]->min_grad + 1) >> 2)) {
            buff_ptr[0]->begin_pos = buff_ptr[1]->begin_pos;
            buff_ptr[0]->len = buff_ptr[0]->begin_pos - buff_ptr[1]->begin_pos + buff_ptr[0]->len;
        } else {
            --buff_ptr[0];
            memcpy(buff_ptr[0], buff_ptr[1], sizeof(struct image_raise_fall_edge));
        }
    }
    cnt = buff_end - buff_ptr[0];
    memset(pedge + cnt, 0, sizeof(struct image_raise_fall_edge) * (num - cnt));
    memcpy(pedge, buff_ptr[0], sizeof(struct image_raise_fall_edge) * cnt);
    mem_free(buff);

    return cnt;
}
