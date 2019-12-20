#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "rf_edges.h"
#include "qr_position.h"

#define debug_xx

static void qr_edges_dist_left_shift_one(unsigned int w[5])
{
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = w[4];
}

#if 0
static bool qr_check_finder_mode(const unsigned int w[5])
{
    bool ret;
    unsigned int sum_a, sum_b, sum_c[4];
    unsigned int a[4], b;

    sum_a = w[0] + w[1] + w[3] + w[4];
    sum_b = sum_a + w[2];
    sum_c[0] = w[0] + w[1];
    sum_c[1] = w[1] + w[2];
    sum_c[2] = w[2] + w[3];
    sum_c[3] = w[3] + w[4];

    a[0] = (w[0] << 6) / sum_a;
    a[1] = (w[1] << 6) / sum_a;
    a[2] = (w[3] << 6) / sum_a;
    a[3] = (w[4] << 6) / sum_a;
    b = ((w[2] * 7) << 4) / (sum_b * 3);
    ret = a[0] >= 10 && a[0] <= 20 && a[1] >= 10 && a[1] <= 20
        && a[2] >= 10 && a[2] <= 20 && a[3] >= 10 && a[3] <= 20
        && b >= 10 && b <= 20
        && sum_c[0] >= (sum_c[3] << 2) / 5 && sum_c[3] >= (sum_c[0] << 2) / 5
        && sum_c[1] >= (sum_c[0] * 9) / 5 && sum_c[1] <= (sum_c[0] * 12) / 5
        && unsigned_diff(sum_c[1], sum_c[2]) <= (sum_c[0] >> 1);

    return ret;
}
#else
static bool qr_check_finder_mode(const unsigned int w[5])
{
    unsigned int total_msize, msize, max_variance;

    total_msize = w[0] + w[1] + w[2] + w[3] + w[4];
    if (total_msize < 7)
        return false;

    total_msize *= 14;
    msize = total_msize / 7;
    max_variance = msize >> 1;

    return unsigned_diff(w[0] * 14, msize) < max_variance
        && unsigned_diff(w[1] * 14, msize) < max_variance
        && unsigned_diff(w[2] * 14, msize * 3) < 3 * max_variance
        && unsigned_diff(w[3] * 14, msize) < max_variance
        && unsigned_diff(w[4] * 14, msize) < max_variance;
}
#endif

int qr_position_makrings_find(const struct image *img,
        struct qr_position_makrings_info *pqpmi, const unsigned int sz)
{
    int count;
    unsigned int i, j, ne, n, netmp;
    struct point edge_start, edge_off;
    struct image_raise_fall_edge edges[500];
    struct image_raise_fall_edge edges_tmp[2][20];
    unsigned int edges_dist[5], edges_dist_temp[5];

#ifdef debug_xx
    struct image *newimg = image_convert_format(img, IMAGE_FORMAT_BGR);
    const unsigned char rec[3] = {0x98, 0x35, 0x95};
    const unsigned char fec[3] = {0x11, 0xbf, 0xF7};
    const unsigned char *ecptr;
#endif

    count = 0;
    for (j = 0; j < img->height; ++j) {
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
            if (qr_check_finder_mode(edges_dist)) {
                edge_start.x = edges[i + 2].dpos + ((edges_dist[2] + 256) >> 9);
                edge_start.y = j;
                edge_off.x = 0;
                edge_off.y = -1;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[0], 20);
                if (netmp < 3)
                    continue;

                edge_off.y = 1;
                edges_dist_temp[0] = edges_tmp[0][2].dpos - edges_tmp[0][1].dpos;
                edges_dist_temp[1] = edges_tmp[0][1].dpos - edges_tmp[0][0].dpos;
                netmp = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, 1000, edges_tmp[1], 20);
                if (netmp < 3)
                    continue;

                edges_dist_temp[2] = edges_tmp[1][0].dpos + edges_tmp[0][0].dpos;
                edges_dist_temp[3] = edges_tmp[1][1].dpos - edges_tmp[1][0].dpos;
                edges_dist_temp[4] = edges_tmp[1][2].dpos - edges_tmp[1][1].dpos;
                (void)(edges_dist_temp);
                if (!qr_check_finder_mode(edges_dist_temp))
                    continue;

#ifdef debug_xx
                int xx;

                for (xx = 0; xx < 3; ++xx) {
                    if (edges_tmp[0][xx].type == IMAGE_RFEDGE_TYPE_RAISE)
                        ecptr = rec;
                    else
                        ecptr = fec;
                    memcpy(newimg->data + (j - edges_tmp[0][xx].dpos) * newimg->row_size + edge_start.x * newimg->pixel_size,
                            ecptr, 3);
                }

                for (xx = 0; xx < 3; ++xx) {
                    if (edges_tmp[1][xx].type == IMAGE_RFEDGE_TYPE_RAISE)
                        ecptr = rec;
                    else
                        ecptr = fec;
                    memcpy(newimg->data + (j + edges_tmp[1][xx].dpos) * newimg->row_size + edge_start.x * newimg->pixel_size,
                            ecptr, 3);
                }

                for (xx = 0; xx < 6; ++xx) {
                    if (edges[i + xx].type == IMAGE_RFEDGE_TYPE_RAISE)
                        ecptr = rec;
                    else
                        ecptr = fec;
                    memcpy(newimg->data + j * newimg->row_size + edges[xx + i].dpos * newimg->pixel_size,
                            ecptr, 3);
                }
#endif
            }
        }
    }

#ifdef debug_xx
    image_save("ruerue.bmp", newimg, IMAGE_FILE_BITMAP);
    image_release(newimg);
#endif

    return count;
}
