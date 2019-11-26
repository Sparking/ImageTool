#include <stdlib.h>
#include <string.h>
#include "maths.h"
#include "rf_edges.h"

unsigned int image_find_raise_fall_edges(const unsigned char *imgdata, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    unsigned int cnt;
    unsigned int pos;
    struct image_raise_fall_edge *cur_edge;
    unsigned char last_gray;
    unsigned char grad;

    if (imgdata == NULL || pedge == NULL || len == 0 || num == 0)
        return 0;

    last_gray = *imgdata;
    memset(pedge, 0, sizeof(*pedge) * num);
    cnt = 0;
    cur_edge = pedge;
    cur_edge->len = 1;
    cur_edge->begin_pos = 0;
    cur_edge->type = IMAGE_RFEDGE_TYPE_NONE;
    cur_edge->min_gray = imgdata[0];
    cur_edge->max_gray = imgdata[0];
    cur_edge->burr_base = imgdata[0];
    cur_edge->grad = 0;
    for (pos = 1; (pos < len) && (cnt < num); ++pos) {
        if (imgdata[pos] > last_gray) {
            grad = imgdata[pos] - last_gray;
            if ((grad >> 1) > cur_edge->grad) {
                ++cur_edge;
                ++cnt;
                cur_edge->len = 1;
                cur_edge->begin_pos = pos;
                cur_edge->type = IMAGE_RFEDGE_TYPE_RAISE;
                cur_edge->min_gray = imgdata[pos];
                cur_edge->min_gray = imgdata[pos];
                cur_edge->burr_base = imgdata[pos];
                cur_edge->grad = 0;
            } else if (cur_edge->grad < grad) {
                ++cur_edge->len;
                cur_edge->grad = grad;
                cur_edge->max_gray = imgdata[pos];
                cur_edge->burr_base = imgdata[pos];
            }
        } else if (imgdata[pos] < last_gray) {
            grad = last_gray - imgdata[pos];
            if ((grad >> 1) > cur_edge->grad) {
                ++cur_edge;
                ++cnt;
                cur_edge->len = 1;
                cur_edge->begin_pos = pos;
                cur_edge->type = IMAGE_RFEDGE_TYPE_FALL;
                cur_edge->min_gray = imgdata[pos];
                cur_edge->min_gray = imgdata[pos];
                cur_edge->burr_base = imgdata[pos];
                cur_edge->grad = 0;
            } else if (cur_edge->grad < grad) {
                ++cur_edge->len;
                cur_edge->grad = grad;
                cur_edge->max_gray = imgdata[pos];
                cur_edge->burr_base = imgdata[pos];
            }
        } else {
                    ++cur_edge;
                    ++cnt;
                    cur_edge->len = 1;
                    cur_edge->begin_pos = pos;
                    cur_edge->type = IMAGE_RFEDGE_TYPE_NONE;
                    cur_edge->min_gray = imgdata[pos];
                    cur_edge->min_gray = imgdata[pos];
                    cur_edge->burr_base = imgdata[pos];
                    cur_edge->grad = 0;
        }
        last_gray = imgdata[pos];
    }

    return cnt;
}
