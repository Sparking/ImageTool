#include <assert.h>
#include <limits.h>
#include <string.h>
#include "rbtree.h"
#include "dotcode_detect_point.h"

struct point g_dtcline_coeff[] = {
    { -5,0 },{ -5,-3 },{ -3,-5 },{ 0, -5 },{ 3,-5 },{ 5,-3 },{ 5,0 },{ 5,3 },{ 3,5 },{ 0,5 },{ -3,5 },{ -5,3 },{ -5,0 }
    /**-180     -150     -120       -90       -60      -30      0       30      60      90      120      150       180**/
};

struct rb_dotcode_point {
    struct dotcode_point pt;
    struct rb_node node;
};

static UNUSED struct rb_dotcode_point *dotcode_rb_point_insert(
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

static UNUSED void dotcode_rb_point_clean(struct rb_root *root)
{
    struct rb_node *node;

    if (root == NULL || root->rb_node == NULL)
        return;

    while ((node = rb_last(root)) != NULL) {
        rb_erase(node, root);
        free(rb_entry(node, struct rb_dotcode_point, node));
    }
}

static UNUSED struct rb_dotcode_point *dotcode_rb_point_find(
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
    edge_off.x = -1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;
    rfe_tmp = rfe_buff[0];

    edge_off.x = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    newpos->x = (((pos->x << 8) + rfe_buff[0].dpos_256x - ((*width + 1) >> 1) + 128) >> 8);
    newpos->y = pos->y;
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
    newpos->y = (((pos->y << 8) + rfe_buff[0].dpos_256x - ((*width + 1) >> 1) + 128) >> 8);
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

    edge_off.x = 1;
    edge_off.y = -1;
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

static bool dotcode_judge_width(const unsigned int a, const unsigned int b)
{
    unsigned int diff;

    if ((a <= 3 && b < 6) || (b <= 3 && a < 6)) {
        return true;
    }

    diff = unsigned_diff(a, b) << 2;
    return (a * 3 > diff && b * 3 > diff) && (a * 3 <= 5 * b && b * 3 <= 5 * a);
}

bool dotcode_checkdot_with_ref(const struct image *img, const int ref_rfet,
        struct dotcode_point *pt, struct point *coordinate, const struct dotcode_point *ref)
{
    if (img == NULL || pt == NULL || coordinate == NULL || ref == NULL) {
        return false;
    }

    memset(pt, 0, sizeof(*pt));
    if (dotcode_detect_point_get_hori_width(img, coordinate, ref_rfet, &pt->center, &pt->nw)) {
        if (dotcode_judge_width(pt->nw, ref->nw)) {
            memcpy(coordinate, &pt->center, sizeof(*coordinate));
            ++pt->score;
        }
    }

    if (dotcode_detect_point_get_vertical_width(img, coordinate, ref_rfet, &pt->center, &pt->nh)) {
        if (dotcode_judge_width(pt->nh, ref->nh)) {
            ++pt->score;
        }
    }

    if (dotcode_detect_point_get_45_width(img, &pt->center, ref_rfet, &pt->n45)) {
        if (dotcode_judge_width(pt->n45, ref->n45)) {
            ++pt->score;
        }
    }

    if (dotcode_detect_point_get_135_width(img, &pt->center, ref_rfet, &pt->n135)) {
        if (dotcode_judge_width(pt->n135, ref->n135)) {
            ++pt->score;
        }
    }
    pt->isblack = (ref_rfet == IMAGE_RFEDGE_TYPE_RAISE);

    return pt->score >= 4;
}

bool dotcode_checkdot(const struct image *img, const bool isblack,
        struct dotcode_point *pt, struct point *coordinate)
{
    int ref_rfet;

    if (img == NULL || pt == NULL || coordinate == NULL) {
        return false;
    }

    ref_rfet = isblack ? IMAGE_RFEDGE_TYPE_RAISE : IMAGE_RFEDGE_TYPE_FALL;
    if (!dotcode_detect_point_get_vertical_width(img, coordinate, ref_rfet, &pt->center, &pt->nh)) {
        return false;
    }

    coordinate->y = pt->center.y;
    if (!dotcode_detect_point_get_hori_width(img, coordinate, ref_rfet, &pt->center, &pt->nw)) {
        return false;
    }

    if (!dotcode_judge_width(pt->nh, pt->nw)) {
        return false;
    }

    if (!dotcode_detect_point_get_45_width(img, &pt->center, ref_rfet, &pt->n45)) {
        return false;
    }

    if (!dotcode_judge_width(pt->nw, pt->n45)) {
        return false;
    }

    if (!dotcode_detect_point_get_135_width(img, &pt->center, ref_rfet, &pt->n135)) {
        return false;
    }

    if (!dotcode_judge_width(pt->n45, pt->n135)) {
        return false;
    }

    pt->isblack = isblack;

    return true;
}

static void dotcode_dot_init(struct dotcode_dot *dot, const struct dotcode_point *pt,
        struct list_head *head45, struct list_head *head135)
{
    memcpy(&dot->dot, pt, sizeof(dot->dot));
    dot->node45.prev = NULL;
    dot->node45.next = NULL;
    dot->node135.prev = NULL;
    dot->node135.next = NULL;
    dot->head135 = head135;
    dot->head45 = head45;
    if (head45) {
        list_add_tail(&dot->node45, dot->head45);
    }

    if (head135) {
        list_add_tail(&dot->node135, dot->head135);
    }
}

static void dotcode_line_node_update_index(const struct dotcode_line_node *line)
{
    struct list_head *pn;
    struct dotcode_line_node *pln;

    for (pn = line->node.next; pn != line->head; pn = pn->next) {
        pln = list_entry(pn, struct dotcode_line_node, node);
        ++pln->index;
    }

    /*debug*/
    if (!list_empty(line->head)) {
        int last_index;

        pn = line->head->next;
        last_index = list_entry(pn, struct dotcode_line_node, node)->index;
        for (pn = pn->next; pn != line->head; pn = pn->next) {
            assert(list_entry(pn, struct dotcode_line_node, node)->index = last_index + 1);
            ++last_index;
        }
    }
}

static void dotcode_insert_line(struct dotcode_line_node *line)
{
    struct dotcode_line_node *pln;

    list_for_each_entry(pln, struct dotcode_line_node, line->head, node) {
        if (pln->index >= line->index) {
            break;
        }
    }

    if (list_empty(line->head)) {
        list_add_tail(&line->node, line->head);
    } else {
        list_add_tail(&line->node, &pln->node);
    }
    dotcode_line_node_update_index(line);
}

static void dotcode_line_node_init(struct dotcode_line_node *node,
    struct dotcode_line *line, const int dir, const int index)
{
    node->dir = dir;
    node->index = index;
    node->node.next = NULL;
    node->node.prev = NULL;
    node->min_len = INT_MAX;
    INIT_LIST_HEAD(node->pt);
    if (dir == 0) {
        node->head = &line->line45;
    } else {
        node->head = &line->line135;
    }

    dotcode_insert_line(node);
}

static void dotcode_line_init(struct dotcode_line *pdln)
{
    INIT_LIST_HEAD(pdln->line45);
    INIT_LIST_HEAD(pdln->line135);
}

static struct dotcode_dot *dotcode_create_dot(struct list_head *head45, struct list_head *head135,
        const struct dotcode_point *pt)
{
    struct dotcode_dot *dot;

    if (pt == NULL)
        return NULL;

    dot = (struct dotcode_dot *)malloc(sizeof(struct dotcode_dot));
    if (dot != NULL) {
        dotcode_dot_init(dot, pt, head45, head135);
    }

    return dot;
}

static struct dotcode_line_node *dotcode_create_line_node(struct dotcode_line *line,const int dir, const int index)
{
    struct dotcode_line_node *pln;

    if (line == NULL)
        return NULL;

    pln = (struct dotcode_line_node *)malloc(sizeof(struct dotcode_line_node));
    if (pln != NULL) {
        dotcode_line_node_init(pln, line, dir, index);
    }

    return pln;
}

static void dotcode_remove_line_node(struct dotcode_line_node *node)
{
    struct dotcode_dot *pdot, *tmpp;

    if (node == NULL)
        return;

    if (node->dir == 0) {
        list_for_each_entry_safe(pdot, tmpp, struct dotcode_dot, &node->pt, node45) {
            list_del(&pdot->node45);
            if (pdot->node135.next)
                list_del(&pdot->node135);
            free(pdot);
        }
    } else {
        list_for_each_entry_safe(pdot, tmpp, struct dotcode_dot, &node->pt, node135) {
            list_del(&pdot->node135);
            if (pdot->node45.next)
                list_del(&pdot->node45);
            free(pdot);
        }
    }
    list_del(&node->node);
    free(node);
}

static bool dotcode_gooddot_search_line45(const struct image *img, const struct dotcode_point *pdtp, struct dotcode_line *lines)
{
    int min_len;
    int scan_off;
    unsigned int nedge;
    unsigned int ndpt;
    struct point pt;
    struct point last_secpt;
    struct point scan_endpos;
    struct point scan_range[2];
    struct dotcode_point curpt;
    struct dotcode_point dpt[100];
    struct dotcode_dot *pdot;
    struct dotcode_line_node *line45, *minln, *tmpln;
    struct image_raise_fall_edge edges[50];

    const int dbg_ushow_scan_flag = 0;
    const int color[2][3] = { {REDCOLOR, YELLOWCOLOR, CYANCOLOR},{GREENCOLOR, PINKCOLOR, BLUECOLOR} };

    if (img == NULL || pdtp == NULL || lines == NULL)
        return 0;

    ndpt = 0;
    scan_off = (pdtp->nw + 1) >> 1;
    memset(&last_secpt, 0, sizeof(last_secpt));
    memcpy(&scan_range[0], &pdtp->center, sizeof(struct point));
    memcpy(&scan_range[1], &pdtp->center, sizeof(struct point));
    scan_range[1].x += pdtp->nw * (-3);
    scan_range[1].y += pdtp->nh * (5);
    scan_endpos.x = pdtp->center.x + pdtp->nw * 5;
    scan_endpos.y = pdtp->center.y + pdtp->nh * (-3) - scan_off;

    ushow_ptWidth(dbg_ushow_scan_flag, scan_range[0].x, scan_range[0].y, color[0][0], 3);
    ushow_ptWidth(dbg_ushow_scan_flag, scan_range[1].x, scan_range[1].y, color[0][1], 1);
    dotcode_line_init(lines);
    minln = NULL;
    do {
        if (fabs(scan_range[0].x - scan_endpos.x) >= fabs(scan_range[0].y - scan_endpos.y)) {
            scan_endpos.y += scan_off;
        } else {
            scan_endpos.x -= scan_off;
        }

        if (scan_endpos.x < 0 || scan_endpos.x >= (int)img->width || scan_endpos.y < 0 || scan_endpos.y >= (int)img->height)
            break;

        nedge = image_find_raise_fall_edges_pt2pt(img, &scan_range[0], &scan_endpos, edges, 50);
        if (nedge < 3)
            continue;

        get_pos_in_pt2pt(&scan_range[0], &scan_endpos, &pt,
            (edges[2].dpos_256x + edges[1].dpos_256x + 256) >> 9);
        if (!dotcode_checkdot_with_ref(img, edges[0].type, &curpt, &pt, pdtp))
            continue;

        /**直线去重**/
        if ((int)fabs(last_secpt.x - curpt.center.x) <= 5 && (int)fabs(last_secpt.y - curpt.center.y) <= 5)
            continue;

        ndpt = 2;
        memcpy(&last_secpt, &curpt.center, sizeof(last_secpt));
        memcpy(&dpt[0], pdtp, sizeof(dpt[0]));
        memcpy(&dpt[1], &curpt, sizeof(dpt[0]));
        min_len = (int)points_distance(&pdtp->center, &curpt.center);
        do {
            get_line_dirpos(&dpt[ndpt - 2].center, &dpt[ndpt - 1].center, &dpt[ndpt - 1].center,
                dpt[0].nw << 2, &pt);

            nedge = image_find_raise_fall_edges_pt2pt(img, &dpt[ndpt - 1].center, &pt, edges, 50);
            if (nedge < 3)
                break;

            get_pos_in_pt2pt(&dpt[ndpt - 1].center, &pt, &pt,
                (edges[2].dpos_256x + edges[1].dpos_256x + 256) >> 9);
            if (!dotcode_checkdot_with_ref(img, edges[0].type, &curpt, &pt, pdtp))
                break;

            if (!points_in_line(&dpt[ndpt - 2].center, &dpt[ndpt - 1].center, &curpt.center)) {
                break;
            }

            min_len = imin(min_len, (int)points_distance(&dpt[ndpt - 1].center, &curpt.center));
            memcpy(&dpt[ndpt++], &curpt, sizeof(dpt[0]));
            if (ndpt >= 50)
                break;
        } while (1);

        if (ndpt > 5) {
            line45 = dotcode_create_line_node(lines, 0, 0);
            if (line45 == NULL)
                break;

            line45->min_len = min_len;
            for (line45->ndt = 0; line45->ndt < (int)ndpt; ++line45->ndt) {
                pdot = dotcode_create_dot(&line45->pt, NULL, &dpt[line45->ndt]);
                assert(pdot != NULL);
            }
            if (minln == NULL) {
                minln = line45;
            } else if (minln->min_len > min_len) {
                minln = line45;
            }
        }
    } while (scan_endpos.x >= scan_range[1].x);

    if (list_empty(&lines->line45))
        return false;

    list_for_each_entry_safe(line45, tmpln, struct dotcode_line_node, &lines->line45, node) {
        if (line45 == minln)
            continue;
        dotcode_remove_line_node(line45);
    }

    return true;
}

static bool dotcode_gooddot_search_vertline(const struct image *img,
        const struct point *start, const struct point *end, const struct dotcode_dot *ref)
{
    int nedge;
    struct point pt, sr[2];
    struct dotcode_point dpt;
    struct image_raise_fall_edge edges[50];

    memcpy(&sr[0], start, sizeof(struct point));
    memcpy(&sr[1], end, sizeof(struct point));
    do {
        nedge = image_find_raise_fall_edges_pt2pt(img, &sr[0], &sr[1], edges, 50);
        if (nedge < 3)
            break;

        get_pos_in_pt2pt(&sr[0], &sr[1], &pt, (edges[2].dpos_256x + edges[1].dpos_256x + 256) >> 9);
        if (!dotcode_checkdot_with_ref(img, ref->dot.isblack ? IMAGE_RFEDGE_TYPE_RAISE : IMAGE_RFEDGE_TYPE_FALL, &dpt, &pt, &ref->dot))
            break;

        ushow_ptWidth(1, dpt.center.x, dpt.center.y, PINKCOLOR, 1);
        get_line_dirpos(&sr[0], &dpt.center, &dpt.center, dpt.nw << 2, &pt);
        memcpy(&sr[0], &dpt.center, sizeof(struct point));
        memcpy(&sr[1], &pt, sizeof(struct point));
    } while (1);

    return true;
}

static bool dotcode_extend_vertline(const struct image *img, struct dotcode_line_node *pln)
{
    struct point verpt;
    struct dotcode_dot *pdn, *lastdn, *firstdn;

    if (pln == NULL || list_empty(&pln->pt))
        return false;

    if (pln->dir == 0) {
        firstdn = list_entry(pln->pt.next, struct dotcode_dot, node45);
        lastdn = list_entry(pln->pt.prev, struct dotcode_dot, node45);
        if (pln->pt.next == pln->pt.prev)
            return false;

        list_for_each_entry(pdn, struct dotcode_dot, &pln->pt, node45) {
            ushow_ptWidth(1, pdn->dot.center.x, pdn->dot.center.y, GREENCOLOR, 1);
            get_linepos_veroffset(&firstdn->dot.center, &lastdn->dot.center, &pdn->dot.center, pdn->dot.nw << 2, &verpt);
            dotcode_gooddot_search_vertline(img, &pdn->dot.center, &verpt, pdn);
            get_linepos_veroffset(&firstdn->dot.center, &lastdn->dot.center, &pdn->dot.center, -(int)(pdn->dot.nw << 2), &verpt);
            dotcode_gooddot_search_vertline(img, &pdn->dot.center, &verpt, pdn);
        }
    } else {
        list_for_each_entry(pdn, struct dotcode_dot, &pln->pt, node135) {
            ushow_ptWidth(1, pdn->dot.center.x, pdn->dot.center.y, GREENCOLOR, 1);
        }
    }
    dotcode_remove_line_node(pln);

    return true;
}

unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *pdtp, const unsigned int ndtp)
{
    unsigned int i, j, off;
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
    struct dotcode_line lineset;

    if (img == NULL || pdtp == NULL || ndtp == 0)
        return 0;

    dtp_size = 0;
    dtp_root = RB_ROOT;
    hori_edge_off.x = 1;
    hori_edge_off.y = 0;
    hori_edge_start.x = 0;
    off = 2;
    for (j = 0; j < img->height; ++j) {
        hori_edge_start.y = j;
        rfe_cnt = image_find_raise_fall_edges_by_offset_dotcode(img,
                &hori_edge_start, &hori_edge_off, img->width, rfe_hori, 500);
        if (rfe_cnt < 2)
            continue;

        off = 2;
        for (i = 1; i < rfe_cnt; ++i) {
            pt.nw = rfe_hori[i].dpos_256x - rfe_hori[i - 1].dpos_256x;
            edge_start.x = (rfe_hori[i - 1].dpos_256x + ((pt.nw + 1) >> 1) + 128) >> 8;
            pt.nw = (pt.nw + 128) >> 8;
            edge_start.y = j;
            if (!dotcode_checkdot(img, rfe_hori[i].type == IMAGE_RFEDGE_TYPE_RAISE ? true : false, &pt, &edge_start))
                continue;

            if (pt.nw <= 6)
                continue;

            pt.score = 4;
            ushow_ptWidth(1, pt.center.x, pt.center.y, GREENCOLOR, 2);
            if (!dotcode_gooddot_search_line45(img, &pt, &lineset)) {
                continue;
            }

            if (!dotcode_extend_vertline(img, list_entry(lineset.line45.next, struct dotcode_line_node, node)))
                continue;

            break;
            if (off < (pt.nh >> 1))
                off = (pt.nh >> 1);

            rb_pt = dotcode_rb_point_insert(&dtp_root, &pt);
            if (rb_pt == NULL)
                continue;
        }

        if (i != rfe_cnt)
            break;
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

unsigned int dotcode_detect_point_test(const struct image *img,
    struct dotcode_point *pdtp, const unsigned int ndtp)
{
    int nedge;
    struct dotcode_point pt;
    struct point center = { 320, 329 };
    struct image_raise_fall_edge edges[1000];

    if (img == NULL || pdtp == NULL || ndtp == 0)
        return 0;

    if (!dotcode_checkdot(img, IMAGE_RFEDGE_TYPE_RAISE, &pt, &center))
        return 0;

    if (pt.nw <= 6)
        return 0;

    pt.score = 4;
    ushow_ptWidth(1, pt.center.x, pt.center.y, GREENCOLOR, 1);

    struct point x, end = { 0, img->height - 1 };
    center = pt.center;
    while (end.x < (int)img->width) {
        ++end.x;

        nedge = image_find_raise_fall_edges_pt2pt(img, &center, &end, edges, 1000);
        const int color[] = { REDCOLOR, GREENCOLOR };
        for (int i = 0; i < nedge; ++i) {
            get_pos_in_pt2pt(&center, &end, &x, edges[i].dpos);
            ushow_pt(1, x.x, x.y, color[i & 2]);
        }
    }

    end.x = img->width - 1;
    while (end.y > 0) {
        --end.y;

        nedge = image_find_raise_fall_edges_pt2pt(img, &center, &end, edges, 1000);
        const int color[] = { REDCOLOR, GREENCOLOR };
        for (int i = 0; i < nedge; ++i) {
            get_pos_in_pt2pt(&center, &end, &x, edges[i].dpos);
            ushow_pt(1, x.x, x.y, color[i & 2]);
        }
    }

    end.y = 0;
    while (end.x > 0) {
        --end.x;

        nedge = image_find_raise_fall_edges_pt2pt(img, &center, &end, edges, 1000);
        const int color[] = { REDCOLOR, GREENCOLOR };
        for (int i = 0; i < nedge; ++i) {
            get_pos_in_pt2pt(&center, &end, &x, edges[i].dpos);
            ushow_pt(1, x.x, x.y, color[i & 2]);
        }
    }

    end.x = 0;
    while (end.y++ < (int)img->height - 1) {

        nedge = image_find_raise_fall_edges_pt2pt(img, &center, &end, edges, 1000);
        const int color[] = { REDCOLOR, GREENCOLOR };
        for (int i = 0; i < nedge; ++i) {
            get_pos_in_pt2pt(&center, &end, &x, edges[i].dpos);
            ushow_pt(1, x.x, x.y, color[i & 2]);
        }
    }
    return 0;
}
