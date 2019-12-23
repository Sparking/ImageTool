#include <string.h>
#include "rbtree.h"
#include "dotcode_detect_point.h"

struct rb_dotcode_point {
    struct dotcode_point pt;
    struct rb_node node;
};

static __attribute__((unused)) struct rb_dotcode_point *dotcode_rb_point_insert(
        struct rb_root *root, struct dotcode_point *pt)
{
    int cmp_ret;
    struct rb_node **new, *parent;
    struct rb_dotcode_point *rb_pt;

    if (root == NULL || pt == NULL)
        return NULL;

    parent = NULL;
    new = &root->rb_node;
    while (*new) {
        parent = *new;
        rb_pt = rb_entry(*new, struct rb_dotcode_point, node);
        cmp_ret = memcmp(&pt->center, &rb_pt->pt.center, sizeof(pt->center));
        if (cmp_ret > 0) {
            new = &(*new)->rb_right;
        } else if (cmp_ret < 0) {
            new = &(*new)->rb_left;
        } else {
            ++rb_pt->pt.weight;
            return rb_pt;
        }
    }

    rb_pt = (struct rb_dotcode_point *)mem_alloc(sizeof(*rb_pt));
    if (rb_pt == NULL)
        return NULL;

    memcpy(&rb_pt->pt, pt, sizeof(*pt));
    rb_pt->pt.weight = 0;
    rb_link_node(&rb_pt->node, parent, new);
    rb_insert_color(&rb_pt->node, root);

    return rb_pt;
}

static __attribute__((unused)) void dotcode_rb_point_clean(struct rb_root *root)
{
    struct rb_node *node;

    if (root == NULL || root->rb_node == NULL)
        return;

    while ((node = rb_last(root)) != NULL) {
        rb_erase(node, root);
        free(rb_entry(node, struct rb_dotcode_point, node));
    }
}

static __attribute__((unused)) struct rb_dotcode_point *dotcode_rb_point_find(
        struct rb_root *root, int x, int y)
{
    int cmp_ret;
    struct rb_node *node;
    struct rb_dotcode_point *rb_pt;
    const struct point pt = {x, y};

    node = root->rb_node;
    rb_pt = NULL;
    while (node) {
        rb_pt = rb_entry(node, struct rb_dotcode_point, node);
        cmp_ret = memcmp(&pt, &rb_pt->pt.center, sizeof(pt));
        if (cmp_ret == 0)
            break;

        if (cmp_ret > 0) {
            node = node->rb_right;
        } else {
            node = node->rb_left;
        }
    }

    return rb_pt;
}

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

static bool dotcode_detect_point_get_hori_width(const struct image *img,
        const struct point *pos, const int edge_type, struct point *newpos, unsigned int *width)
{
    unsigned int cnt;
    struct point edge_off;
    struct image_raise_fall_edge rfe_buff[20], rfe_tmp;

    edge_off.y = 0;
    edge_off.x = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    rfe_tmp = rfe_buff[0];
    edge_off.x = -1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    newpos->x = pos->x;
    newpos->y = pos->y - ((*width + 256) >> 9) + rfe_tmp.dpos;
    *width = (*width + 128) >> 8;

    return true;
}

static bool dotcode_detect_point_get_vertical_width(const struct image *img,
        const struct point *pos, const int edge_type, struct point *newpos, unsigned int *width)
{
    unsigned int cnt;
    struct point edge_off;
    struct image_raise_fall_edge rfe_buff[20], rfe_tmp;

    edge_off.x = 0;
    edge_off.y = -1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    rfe_tmp = rfe_buff[0];
    edge_off.y = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    newpos->x = pos->x;
    newpos->y = pos->y - ((*width + 256) >> 9) + rfe_tmp.dpos;
    *width = (*width + 128) >> 8;

    return true;
}

static bool dotcode_detect_point_get_45_width(const struct image *img,
        const struct point *pos, const int edge_type, unsigned int *width)
{
    unsigned int cnt;
    struct point edge_off;
    struct image_raise_fall_edge rfe_buff[20], rfe_tmp;

    edge_off.x = -1;
    edge_off.y = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    rfe_tmp = rfe_buff[0];
    edge_off.x = -1;
    edge_off.y = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    *width = (*width + 128) >> 8;

    return true;
}

static bool dotcode_detect_point_get_135_width(const struct image *img,
        const struct point *pos, const int edge_type, unsigned int *width)
{
    unsigned int cnt;
    struct point edge_off;
    struct image_raise_fall_edge rfe_buff[20], rfe_tmp;

    edge_off.x = 1;
    edge_off.y = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 10);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    rfe_tmp = rfe_buff[0];
    edge_off.x = -1;
    edge_off.y = -1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 10);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    *width = (*width + 128) >> 8;

    return true;
}

unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *pdtp, const unsigned int ndtp)
{
    unsigned int i, j;
    unsigned int diff;
    unsigned int rfe_cnt;
    unsigned int dtp_size;
    struct point edge_start;
    struct point hori_edge_off;
    struct point hori_edge_start;
    struct dotcode_point pt;
    struct rb_root dtp_root;
    struct rb_node *dtp_node;
    struct rb_dotcode_point *rb_pt;
    struct image_raise_fall_edge rfe_hori[500];

    if (img == NULL || pdtp == NULL || ndtp == 0)
        return 0;

    dtp_size = 0;
    dtp_root = RB_ROOT;
    hori_edge_off.x = 1;
    hori_edge_off.y = 0;
    hori_edge_start.x = 0;
    for (j = 0; j < img->height; j += 4) {
        hori_edge_start.y = j;
        rfe_cnt = image_find_raise_fall_edges_by_offset_dotcode(img,
                &hori_edge_start, &hori_edge_off, img->width, rfe_hori, 500);
        if (rfe_cnt < 2)
            continue;

        for (i = 1; i < rfe_cnt; ++i) {
            edge_start.y = j;
            pt.nw = (rfe_hori[i].dpos_256x - rfe_hori[i - 1].dpos_256x + 128) >> 8;
            edge_start.x = rfe_hori[i].dpos - ((pt.nw + 1) >> 1);

            if (!dotcode_detect_point_get_vertical_width(img, &edge_start,
                    rfe_hori[i].type, &pt.center, &pt.nh))
                continue;

            diff = (unsigned_diff(pt.nw, pt.nh) + 1) >> 1;
            if (pt.nw <= diff || pt.nh <= diff)
                continue;

            if (j !=  pt.center.y) {
                edge_start.y = pt.center.y;
                if (!dotcode_detect_point_get_hori_width(img, &edge_start,
                        rfe_hori[i].type, &pt.center, &pt.nw))
                    continue;
            }

            diff = (unsigned_diff(pt.nw, pt.nh) + 1) >> 1;
            if (pt.nw <= diff || pt.nh <= diff)
                continue;

            if (!dotcode_detect_point_get_45_width(img, &pt.center, rfe_hori[i].type,
                    &pt.n45))
                continue;

            diff = (unsigned_diff(pt.nw, pt.n45) + 1) >> 1;
            if (pt.nw <= diff || pt.n45 <= diff)
                continue;

            if (!dotcode_detect_point_get_135_width(img, &pt.center, rfe_hori[i].type,
                    &pt.n135))
                continue;

            diff = (unsigned_diff(pt.n135, pt.n45) + 1) >> 1;
            if (pt.n135 <= diff || pt.n45 <= diff)
                continue;

            pt.isblack = (rfe_hori[i].type == IMAGE_RFEDGE_TYPE_RAISE);
            rb_pt = dotcode_rb_point_insert(&dtp_root, &pt);
            if (rb_pt == NULL)
                continue;
        }
    }

    dtp_size = 0;
    for (dtp_node = rb_first(&dtp_root); dtp_node != NULL; dtp_node = rb_next(dtp_node)) {
        rb_pt = rb_entry(dtp_node, struct rb_dotcode_point, node);
        if (dtp_size < ndtp) {
            memcpy(pdtp + dtp_size, &rb_pt->pt, sizeof(*pdtp));
            ++dtp_size;
        }
    }
    dotcode_rb_point_clean(&dtp_root);

    return dtp_size;
}
