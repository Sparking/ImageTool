#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <image.h>
#include <maths.h>
#include <port_memory.h>


struct dotcode_point {
    struct point center;
    int nw;
    int nh;
    int n45;
    int n135;
    bool isblack;
    unsigned char score;
};

#define IMAGE_RFEDGE_AMP_LIMIT_MIN      15
unsigned int image_find_raise_fall_edges_by_offset_dotcode(
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

    if (max_edge->amplitude <= IMAGE_RFEDGE_AMP_LIMIT_MIN)
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

int get_edgepos_16x(const int *grads, const int len, const int srchpos)
{
    int pos16x, i, j;

    if (grads[srchpos] == 0)
        return 0;

    pos16x = 0;
    if (grads[srchpos] > 0) {
        for (i = srchpos - 1; i >= 0; --i) {
            if (grads[i] <= 0)
                break;
        }

        for (j = 0, pos16x = 0; ++i < len;) {
            if (grads[i] <= 0)
                break;

            pos16x += grads[i] * i;
            j += grads[i];
        }
    } else {
        for (i = srchpos - 1; i >= 0; --i) {
            if (grads[i] >= 0)
                break;
        }

        for (j = 0, pos16x = 0; ++i < len;) {
            if (grads[i] >= 0)
                break;

            pos16x += grads[i] * i;
            j += grads[i];
        }
    }
    pos16x = (pos16x << 4) / j;

    return pos16x;
}

bool get_dots_edge(unsigned char *data, const int len, const int center, int *center_offset, int *w16x, bool isblack)
{
    int grads[100], head, tail;
    int i, j, npos, tmp;
    unsigned char gradabs[100], maxgrad;
    unsigned char maxgradpos[50];

    for (maxgrad = 0, i = 1, j = 0; i < len; ++i, ++j) {
        grads[j] = (int)data[i] - (int)data[j];
        if (data[j] >= data[i]) {
            gradabs[j] = data[j] - data[i];
        } else {
            gradabs[j] = data[i] - data[j];
        }
        if (maxgrad < gradabs[j])
            maxgrad = gradabs[j];
    }

    for (npos = 0, i = 1, tmp = len - 2, j = -1; i < tmp && npos < 50; ++i) {
        if ((grads[i] > 0
            && ((grads[i] > grads[i - 1] && grads[i] >= grads[i + 1]) ||
            (grads[i] >= grads[i - 1] && grads[i] > grads[i + 1])))
            || (grads[i] < 0
                && ((grads[i] < grads[i - 1] && grads[i] <= grads[i + 1]) ||
                (grads[i] <= grads[i - 1] && grads[i] < grads[i + 1])))) {
            if ((gradabs[i] << 2) < maxgrad)    /**剔除小干扰**/
                continue;

            if (i >= center && j == -1)
                j = npos;
            maxgradpos[npos++] = i;
        }
    }

    if (npos < 2)
        return false;

    if (j < 1) {
        i = maxgradpos[npos - 2];
        j = maxgradpos[npos - 1];
        if (grads[i] * grads[j] >= 0)
            return false;

        if ((isblack && grads[i] < 0)
            || (!isblack && grads[i] > 0))
            return false;

        head = get_edgepos_16x(grads, len - 1, j);
        if (((head + 8) >> 4) < center - 2)
            return false;

        tail = get_edgepos_16x(grads, len - 1, i);
        if (tail == 0)
            return false;

        *w16x = head - tail;
    } else {
        tmp = j;
        i = maxgradpos[j - 1];
        j = maxgradpos[j];
        if (grads[i] * grads[j] >= 0) {
            if (gradabs[i] > gradabs[j]) {
                j = tmp + 1;
                if (j >= npos)
                    return false;

                j = maxgradpos[j];
            } else {
                i = tmp - 2;
                if (i < 0)
                    return false;

                i = maxgradpos[i];
            }

            if (grads[i] * grads[j] >= 0)
                return false;
        }

        if ((isblack && grads[i] < 0) || (!isblack && grads[i] > 0))
            return false;

        head = get_edgepos_16x(grads, len - 1, i);
        tail = get_edgepos_16x(grads, len - 1, j);
        if (head == 0 || tail == 0)
            return false;

        *w16x = tail - head;
    }
    *center_offset = (i + j - (center << 1)) << 3;

    return true;
}

bool dotcode_gooddot_confirmx(const struct image *srcimg, struct point *pt16x, int *plen, const bool isblack)
{
    struct point pt;
    unsigned char imgdata[100];
    int i, j, len, off, pos;

    if (*plen >= 49 || *plen <= 3)
        return false;

    len = 0;
    pos = *plen + 1;
    pt.x = (pt16x->x + 8) >> 4;
    pt.y = (pt16x->y + 8) >> 4;
    i = pt.y * srcimg->width;
    off = pt.x + (*plen + 1);
    if (off >= (int)srcimg->width) {
        off = srcimg->width - 1;
    }
    j = i + off;

    off = pt.x - (*plen + 1);
    if (off < 0) {
        pos += off;
        off = 0;
    }
    i = i + off;

    while (i <= j) {
        imgdata[len++] = srcimg->data[i++];
    }

    if (!get_dots_edge(imgdata, len, pos, &i, &len, isblack)) {
        return false;
    }

    *plen = (len + 8) >> 4;
    pt16x->x = (pt.x << 4) + i;

    return true;
}

bool dotcode_gooddot_confirmy(const struct image *srcimg, struct point *pt16x, int *plen, const bool isblack)
{
    struct point pt;
    unsigned char imgdata[100];
    int i, j, len, off, pos;

    if (*plen >= 49 || *plen <= 3)
        return false;

    len = 0;
    pos = *plen + 1;
    pt.x = (pt16x->x + 8) >> 4;
    pt.y = (pt16x->y + 8) >> 4;
    off = pt.y + (*plen + 1);
    if (off >= (int)srcimg->height) {
        off = srcimg->height - 1;
    }
    off *= srcimg->width;
    j = pt.x + off;

    off = pt.y - (*plen + 1);
    if (off < 0) {
        pos += off;
        off = 0;
    }
    off *= srcimg->width;
    i = pt.x + off;

    while (i <= j) {
        imgdata[len++] = srcimg->data[i];
        i += srcimg->width;
    }

    if (!get_dots_edge(imgdata, len, pos, &i, &len, isblack)) {
        return false;
    }

    *plen = (len + 8) >> 4;
    pt16x->y = (pt.y << 4) + i;

    return true;
}

bool dotcode_gooddot_confirm45(const struct image *srcimg, const struct point *pt16x, int *plen, const bool isblack)
{
    int r, k;
    struct point pt;
    int i, j, len, off, pos;
    unsigned char imgdata[100];

    if (*plen >= 49 || *plen <= 3)
        return false;

    len = 0;
    pos = *plen + 1;
    pt.x = (pt16x->x + 8) >> 4;
    pt.y = (pt16x->y + 8) >> 4;
    r = pt.x - (*plen + 1);
    if (r < 0) {
        return false;
    }

    k = pt.x + (*plen + 1);
    if (k >= (int)srcimg->width) {
        k = srcimg->width - 1;
    }

    off = pt.y + (*plen + 1);
    if (off >= (int)srcimg->height) {
        off = srcimg->height - 1;
    }
    j = off * srcimg->width;

    off = pt.y - (*plen + 1);
    if (off < 0) {
        pos += off;
        off = 0;
    }
    i = off * srcimg->width;

    while (i <= j && r <= k) {
        imgdata[len++] = srcimg->data[i + r];
        i += srcimg->width;
        ++r;
    }

    if (!get_dots_edge(imgdata, len, pos, &i, &len, isblack)) {
        return false;
    }

    *plen = ((len * 3 >> 1) + 8) >> 4;

    return true;
}

bool dotcode_gooddot_confirm135(const struct image *srcimg, const struct point *pt16x, int *plen, const bool isblack)
{
    int r, k;
    struct point pt;
    int i, j, len, off, pos;
    unsigned char imgdata[100];

    if (*plen >= 49 || *plen <= 3)
        return false;

    len = 0;
    pos = *plen + 1;
    pt.x = (pt16x->x + 8) >> 4;
    pt.y = (pt16x->y + 8) >> 4;
    r = pt.x - (*plen + 1);
    if (r < 0) {
        return false;
    }

    k = pt.x + (*plen + 1);
    if (k >= (int)srcimg->width) {
        k = srcimg->width - 1;
    }

    off = pt.y + (*plen + 1);
    if (off >= (int)srcimg->height) {
        off = srcimg->height - 1;
    }
    j = off * srcimg->width;

    off = pt.y - (*plen + 1);
    if (off < 0) {
        pos += off;
        off = 0;
    }
    i = off * srcimg->width;

    while (i <= j && r <= k) {
        imgdata[len++] = srcimg->data[j + r];
        j -= srcimg->width;
        ++r;
    }

    if (!get_dots_edge(imgdata, len, pos, &i, &len, isblack)) {
        return false;
    }

    *plen = ((len * 3 >> 1) + 8) >> 4;

    return true;
}

static bool dotcode_judge_width(const unsigned int a, const unsigned int b)
{
    if (a <= 0 || b <= 0)
        return false;

    if (a == b)
        return true;

    if (b <= 4 && a <= 5)
        return true;

    if (a <= 4 && b <= 5)
        return true;

    return (a << 1) / unsigned_diff(a, b) >= 4;
}

int dotcode_edge_search_length(const int ref)
{
    int value;

    if (ref >= 50) {
        value = -1;
    } else {
        value = ref << 1;
        if (value >= 50)
            value = 49;
    }

    return value;
}

int image_find_dot_by_grad(const struct image *img)
{
    bool ret;
    unsigned int i, j, rfe_cnt;
    struct dotcode_point pt;
    struct point coordinate, dbgcoord;
    struct point hori_edge_off;
    struct point hori_edge_start;
    struct image_raise_fall_edge rfe_hori[500];

    if (img == NULL)
        return 0;

    hori_edge_off.x = 1;
    hori_edge_off.y = 0;
    hori_edge_start.x = 0;
    for (j = 0; j < img->height; ++j) {
        hori_edge_start.y = j;
        rfe_cnt = image_find_raise_fall_edges_by_offset_dotcode(img,
            &hori_edge_start, &hori_edge_off, img->width, rfe_hori, 500);
        if (rfe_cnt < 2)
            continue;

        for (i = 1; i < rfe_cnt; ++i) {
            coordinate.y = j << 4;
            pt.nw = rfe_hori[i].dpos_256x - rfe_hori[i - 1].dpos_256x;
            coordinate.x = (rfe_hori[i - 1].dpos_256x + ((pt.nw + 1) >> 1) + 16) >> 4;
            pt.nw = (pt.nw + 128) >> 8;
            pt.center = coordinate;

            (void)dbgcoord;
            dbgcoord.x = (coordinate.x + 8) >> 4;
            dbgcoord.y = (coordinate.y + 8) >> 4;
            if (j == 177 && dbgcoord.x >= 168 - 5 &&dbgcoord.x <= 168 + 5) {
                hori_edge_off.y = 0;
            }
            pt.nh = dotcode_edge_search_length(pt.nw);
            ret = dotcode_gooddot_confirmy(img, &coordinate, &pt.nh, rfe_hori[i - 1].type != IMAGE_RFEDGE_TYPE_FALL);
            if (!ret || !dotcode_judge_width(pt.nw, pt.nh))
                continue;

            dbgcoord.x = (coordinate.x + 8) >> 4;
            dbgcoord.y = (coordinate.y + 8) >> 4;
            pt.nw = dotcode_edge_search_length(pt.nh);
            ret = dotcode_gooddot_confirmx(img, &coordinate, &pt.nw, rfe_hori[i - 1].type != IMAGE_RFEDGE_TYPE_FALL);
            if (!ret || !dotcode_judge_width(pt.nh, pt.nw))
                continue;

            dbgcoord.x = (coordinate.x + 8) >> 4;
            dbgcoord.y = (coordinate.y + 8) >> 4;

            pt.n45 = dotcode_edge_search_length(imax(pt.nw, pt.nh));
            ret = dotcode_gooddot_confirm45(img, &coordinate, &pt.n45, rfe_hori[i - 1].type != IMAGE_RFEDGE_TYPE_FALL);
            if (!ret || (!dotcode_judge_width(imax(pt.nw, pt.nh), pt.n45)))
                continue;

            pt.n135 = dotcode_edge_search_length(imax(pt.nw, pt.nh));
            ret = dotcode_gooddot_confirm135(img, &coordinate, &pt.n135, rfe_hori[i - 1].type != IMAGE_RFEDGE_TYPE_FALL);
            if (!ret || !dotcode_judge_width(pt.n45, pt.n135))
                continue;

            coordinate.x = (coordinate.x + 8) >> 4;
            coordinate.y = (coordinate.y + 8) >> 4;
            pt.center = coordinate;
            ushow_pt(1, pt.center.x, pt.center.y, GREENCOLOR);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct image *img, *gray;

    if (argc < 2) {
        fprintf(stderr, "param not engouh\n");
        return -1;
    }

    img = image_open(argv[1]);
    if (img == NULL)
        return -1;

    gray = image_convert_gray(img);
    image_release(img);
    if (gray == NULL)
        return -1;

    img = gray;
    image_find_dot_by_grad(img);
    image_release(img);

    return 0;
}

