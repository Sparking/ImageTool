﻿#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "rf_edges.h"
#include "port_memory.h"
#include "qr_position.h"
#include "rbtree.h"

#define debug_xx

struct rb_qpm_info {
    struct qr_position_makrings_info pm;
    struct rb_node node;
};

static __attribute__((unused)) struct rb_qpm_info *rb_qpm_info_insert(
        struct rb_root *root, struct qr_position_makrings_info *pm)
{
    int cmp_ret;
    struct rb_node **new, *parent;
    struct rb_qpm_info *rb_pm;

    if (root == NULL || pm == NULL)
        return NULL;

    parent = NULL;
    new = &root->rb_node;
    while (*new) {
        parent = *new;
        rb_pm = rb_entry(*new, struct rb_qpm_info, node);
        cmp_ret = memcmp(&pm->center, &rb_pm->pm.center, sizeof(pm->center));
        if (cmp_ret > 0) {
            new = &(*new)->rb_right;
        } else if (cmp_ret < 0) {
            new = &(*new)->rb_left;
        } else {
            return rb_pm;
        }
    }

    rb_pm = (struct rb_qpm_info *)mem_alloc(sizeof(*rb_pm));
    if (rb_pm == NULL)
        return NULL;

    memcpy(&rb_pm->pm, pm, sizeof(*pm));
    rb_link_node(&rb_pm->node, parent, new);
    rb_insert_color(&rb_pm->node, root);

    return rb_pm;
}

static __attribute__((unused)) void rb_qpm_info_clean(struct rb_root *root)
{
    struct rb_node *node;

    if (root == NULL || root->rb_node == NULL)
        return;

    while ((node = rb_last(root)) != NULL) {
        rb_erase(node, root);
        free(rb_entry(node, struct rb_qpm_info, node));
    }
}

static void qr_edges_dist_left_shift_one(unsigned int w[5])
{
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = w[4];
}

static bool qr_check_finder_mode(const unsigned int w[5], unsigned int *sz)
{
    unsigned int total_msize, msize, max_variance;

    total_msize = w[0] + w[1] + w[2] + w[3] + w[4];
    if (total_msize < 7)
        return false;

    *sz = total_msize;
    total_msize *= 14;
    msize = total_msize / 7;
    max_variance = msize >> 1;

    return unsigned_diff(w[0] * 14, msize) < max_variance
        && unsigned_diff(w[1] * 14, msize) < max_variance
        && unsigned_diff(w[2] * 14, msize * 3) < 3 * max_variance
        && unsigned_diff(w[3] * 14, msize) < max_variance
        && unsigned_diff(w[4] * 14, msize) < max_variance;
}

int qr_position_makrings_find(const struct image *img,
        struct qr_position_makrings_info *pqpmi, const unsigned int sz)
{
    unsigned int cnt;
    unsigned int i, j, ne, n, netmp;
    struct point edge_start, edge_off;
    struct rb_root pm_root;
    struct rb_node *rb;
    struct rb_qpm_info *rb_pm;
    struct image_raise_fall_edge edges[500];
    struct image_raise_fall_edge edges_tmp[2][20];
    unsigned int edges_dist[5], edges_dist_temp[5];

#ifdef debug_xx
    int xx;
    struct image *newimg = image_convert_format(img, IMAGE_FORMAT_BGR);
    const unsigned char rec[3] = {0x98, 0x35, 0x95};
    const unsigned char fec[3] = {0x11, 0xbf, 0xF7};
    const unsigned char xxc[3] = {0x22, 0x62, 0x87};
    const unsigned char *ecptr;
#endif

    pm_root = RB_ROOT;
    for (j = 0; j < img->height; j += 2) {
        edge_start.x = 0;
        edge_start.y = j;
        edge_off.x = 1;
        edge_off.y = 0;
        ne = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, img->width, edges, 500);
        if (ne < 6)
            continue;

        edges_dist[1] = edges[1].dpos_256x - edges[0].dpos_256x;
        edges_dist[2] = edges[2].dpos_256x - edges[1].dpos_256x;
        edges_dist[3] = edges[3].dpos_256x - edges[2].dpos_256x;
        edges_dist[4] = edges[4].dpos_256x - edges[3].dpos_256x;
        n = ne - 5;
        for (i = 0; i < n; ++i) {
            qr_edges_dist_left_shift_one(edges_dist);
            edges_dist[4] = edges[i + 5].dpos_256x - edges[i + 4].dpos_256x;
            if (qr_check_finder_mode(edges_dist, &pqpmi[0].wx)) {
                /* 1.坐标移动到水平中心位置, 先验证y方向是否也满足1:1:3:1:1 */
                edge_start.x = edges[i + 2].dpos + ((edges_dist[2] + 256) >> 9);
                edge_start.y = j;
                edge_off.x = 0;
                edge_off.y = -1;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[0], 20);
                if (netmp < 3 || edges_tmp[0][0].type == edges[i].type)
                    continue;

                edge_off.y = 1;
                edges_dist_temp[0] = edges_tmp[0][2].dpos_256x - edges_tmp[0][1].dpos_256x;
                edges_dist_temp[1] = edges_tmp[0][1].dpos_256x - edges_tmp[0][0].dpos_256x;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[1], 20);
                if (netmp < 3 || edges_tmp[1][0].type == edges[i].type)
                    continue;

                edges_dist_temp[2] = edges_tmp[1][0].dpos_256x + edges_tmp[0][0].dpos_256x;
                edges_dist_temp[3] = edges_tmp[1][1].dpos_256x - edges_tmp[1][0].dpos_256x;
                edges_dist_temp[4] = edges_tmp[1][2].dpos_256x - edges_tmp[1][1].dpos_256x;
                (void)(edges_dist_temp);
                if (!qr_check_finder_mode(edges_dist_temp, &pqpmi[0].wy))
                    continue;

                /* 2.同时坐标移动到竖直中心位置, 重新验证验证x方向是否满足1:1:3:1:1 */
                edge_start.y  = j + edges_tmp[1][0].dpos - ((edges_tmp[1][0].dpos_256x + edges_tmp[0][0].dpos_256x + 256) >> 9);
                edge_off.y = 0;
                edge_off.x = -1;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[0], 20);
                if (netmp < 3 || edges_tmp[0][0].type == edges[i].type)
                    continue;

                edge_off.y = 0;
                edge_off.x = 1;
                edges_dist_temp[0] = edges_tmp[0][2].dpos_256x - edges_tmp[0][1].dpos_256x;
                edges_dist_temp[1] = edges_tmp[0][1].dpos_256x - edges_tmp[0][0].dpos_256x;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[1], 20);
                if (netmp < 3 || edges_tmp[1][0].type == edges[i].type)
                    continue;

                edges_dist_temp[2] = edges_tmp[1][0].dpos_256x + edges_tmp[0][0].dpos_256x;
                edges_dist_temp[3] = edges_tmp[1][1].dpos_256x - edges_tmp[1][0].dpos_256x;
                edges_dist_temp[4] = edges_tmp[1][2].dpos_256x - edges_tmp[1][1].dpos_256x;
#ifdef debug_xx
                memcpy(newimg->data + edge_start.y * newimg->row_size + edge_start.x * newimg->pixel_size,
                            xxc, 3);
#endif
                (void)(edges_dist_temp);
                if (!qr_check_finder_mode(edges_dist_temp, &pqpmi[0].wx))
                    continue;

                /* 3.中心坐标保持不变, 扫描45方向上的模块比例 */
                edge_off.y = -1;
                edge_off.x = -1;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[0], 20);
                if (netmp < 3 || edges_tmp[0][0].type == edges[i].type)
                    continue;

                edge_off.y = 1;
                edge_off.x = 1;
                edges_dist_temp[0] = edges_tmp[0][2].dpos_256x - edges_tmp[0][1].dpos_256x;
                edges_dist_temp[1] = edges_tmp[0][1].dpos_256x - edges_tmp[0][0].dpos_256x;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[1], 20);
                if (netmp < 3 || edges_tmp[1][0].type == edges[i].type)
                    continue;

                edges_dist_temp[2] = edges_tmp[1][0].dpos_256x + edges_tmp[0][0].dpos_256x;
                edges_dist_temp[3] = edges_tmp[1][1].dpos_256x - edges_tmp[1][0].dpos_256x;
                edges_dist_temp[4] = edges_tmp[1][2].dpos_256x - edges_tmp[1][1].dpos_256x;
                (void)(edges_dist_temp);
                if (!qr_check_finder_mode(edges_dist_temp, &pqpmi[0].w45))
                    continue;

                /* 4.中心坐标保持不变, 扫描135方向上的模块比例 */
                edge_off.y = 1;
                edge_off.x = -1;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[0], 20);
                if (netmp < 3 || edges_tmp[0][0].type == edges[i].type)
                    continue;

                edge_off.y = 1;
                edge_off.x = -1;
                edges_dist_temp[0] = edges_tmp[0][2].dpos_256x - edges_tmp[0][1].dpos_256x;
                edges_dist_temp[1] = edges_tmp[0][1].dpos_256x - edges_tmp[0][0].dpos_256x;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[1], 20);
                if (netmp < 3 || edges_tmp[1][0].type == edges[i].type)
                    continue;

                edges_dist_temp[2] = edges_tmp[1][0].dpos_256x + edges_tmp[0][0].dpos_256x;
                edges_dist_temp[3] = edges_tmp[1][1].dpos_256x - edges_tmp[1][0].dpos_256x;
                edges_dist_temp[4] = edges_tmp[1][2].dpos_256x - edges_tmp[1][1].dpos_256x;
                (void)(edges_dist_temp);
                if (!qr_check_finder_mode(edges_dist_temp, &pqpmi[0].w135))
                    continue;

#ifdef debug_xx
                for (xx = 0; xx < 6; ++xx) {
                    if (edges[i + xx].type == IMAGE_RFEDGE_TYPE_RAISE)
                        ecptr = rec;
                    else
                        ecptr = fec;
                    memcpy(newimg->data + j * newimg->row_size + edges[xx + i].dpos * newimg->pixel_size,
                            ecptr, 3);
                }
#endif
                pqpmi[0].center = edge_start;
                if (rb_qpm_info_insert(&pm_root, &pqpmi[0]) == NULL) {
                    rb_qpm_info_clean(&pm_root);
#ifdef debug_xx
                    goto outof_xx;
#endif
                    return 0;
                }
            }
        }
    }

#ifdef debug_xx
outof_xx:
    image_save("ruerue.bmp", newimg, IMAGE_FILE_BITMAP);
    image_release(newimg);
#endif
    cnt = 0;
    for (rb = rb_first(&pm_root); rb != NULL; rb = rb_next(rb)) {
        rb_pm = rb_entry(rb, struct rb_qpm_info, node);
        if (cnt < sz) {
            memcpy(pqpmi + cnt, &rb_pm->pm, sizeof(*pqpmi));
            ++cnt;
        } else {
            break;
        }
    }
    rb_qpm_info_clean(&pm_root);

    return cnt;
}
