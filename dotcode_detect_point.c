#include <string.h>
#include "rbtree.h"
#include "rf_edges.h"
#include "dotcode_detect_point.h"

struct rb_dotcode_point {
    struct dotcode_point pt;
    struct rb_node node;
};

static struct rb_dotcode_point *dotcode_rb_point_insert(
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

static void dotcode_rb_point_clean(struct rb_root *root)
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

unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *pdtp, const unsigned int ndtp)
{
    unsigned int i, j;
    unsigned int dtp_size, nw;
    unsigned int rfe_tmp_cnt;
    unsigned int rfe_hori_cnt;
    struct point edge_off;
    struct point edge_start;
    struct point hori_edge_off;
    struct point hori_edge_start;
    struct dotcode_point pt;
    struct rb_root dtp_root;
    struct rb_node *dtp_node;
    struct rb_dotcode_point *rb_pt;
    struct image_raise_fall_edge rfe_buff[10];
    struct image_raise_fall_edge rfe_hori[500];
    struct image_raise_fall_edge rfe_tmp;

    if (img == NULL || pdtp == NULL || ndtp == 0)
        return 0;

    dtp_root = RB_ROOT;
    dtp_size = 0;
    hori_edge_start.x = 0;
    hori_edge_off.x = 1;
    hori_edge_off.y = 0;
    for (j = 0; j < img->height; j += 4) {
        hori_edge_start.y = j;
        rfe_hori_cnt = image_find_raise_fall_edges_by_offset(img,
                hori_edge_start, hori_edge_off, img->width, rfe_hori, 500);
        if (rfe_hori_cnt <= 1)
            continue;

        for (i = 1; i < rfe_hori_cnt; ++i) {
            /* 竖直向下扫描 */
            nw = (rfe_hori[i].dpos_256x - rfe_hori[i - 1].dpos_256x + 128) >> 8;
            edge_start.x = rfe_hori[i].dpos - ((nw + 1) >> 1);
            edge_start.y = j;
            edge_off.x = 0;
            edge_off.y = 1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 10);
            if (rfe_tmp_cnt == 0 || rfe_buff[0].type != rfe_hori[i].type)
                continue;
            rfe_tmp = rfe_buff[0];

            /* 竖直向上扫描 */
            edge_off.y = -1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 10);
            if (rfe_tmp_cnt == 0 || rfe_buff[0].type != rfe_hori[i].type)
                continue;

            pt.nh = (rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x + 128) >> 8;
            pt.center.y = j - ((pt.nh + 1) >> 1) + rfe_tmp.dpos;

            /* 切换到中间位置, 并重新水平扫描 */
            if (j !=  pt.center.y) {
                edge_start.y =pt.center.y;
                edge_off.x = 1;
                edge_off.y = 0;

                /* 水平向右 */
                rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                    edge_start, edge_off, 1000, rfe_buff, 10);
                if (rfe_tmp_cnt == 0 || rfe_buff[0].type != rfe_hori[i].type)
                    continue;
                rfe_tmp = rfe_buff[0];

                /* 水平向左 */
                edge_off.x = -1;
                rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                    edge_start, edge_off, 1000, rfe_buff, 10);
                if (rfe_tmp_cnt == 0 || rfe_buff[0].type != rfe_hori[i].type)
                    continue;
                pt.nw = (rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x + 128) >> 8;
                pt.center.x = edge_start.x + rfe_tmp.dpos - ((pt.nw + 1) >> 1);
            } else {
                pt.nw = nw;
                pt.center.x = edge_start.x;
            }

            edge_off.x = -1;
            edge_off.y = -1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 10);
            if (rfe_tmp_cnt == 0 || rfe_buff[0].type != rfe_hori[i].type)
                continue;
            pt.n45 = (rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x + 128) >> 8;
            if (pt.n45 >= (pt.nw << 1) || pt.nw >= (pt.n45 << 1))
                continue;

            edge_start.x = pt.center.x;
            edge_start.y = pt.center.y;
            edge_off.x = -1;
            edge_off.y = 1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 10);
            if (rfe_tmp_cnt == 0 || rfe_buff[0].type != rfe_hori[i].type)
                continue;
            rfe_tmp = rfe_buff[0];

            edge_off.x = 1;
            edge_off.y = -1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 10);
            if (rfe_tmp_cnt == 0 || rfe_buff[0].type != rfe_hori[i].type)
                continue;
            pt.n135 = (rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x + 128) >> 8;
            if (pt.n135 >= (pt.nw << 1) || pt.nw >= (pt.n135 << 1))
                continue;
            pt.isblack = (rfe_hori[i].type != IMAGE_RFEDGE_TYPE_FALL);

            rb_pt = dotcode_rb_point_insert(&dtp_root, &pt);
            if (rb_pt == NULL) {
                continue;
            }
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
