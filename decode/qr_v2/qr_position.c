#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "rf_edges.h"
#include "qr_position.h"
#include <stdio.h>

#define debug_xx
int flag = 0;

static void qr_edges_dist_left_shift_one(unsigned int w[5])
{
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = w[4];
}

static bool qr_check_finder_mode(const unsigned int w[5])
{
    unsigned int a = (((w[0] + w[1] + 1) >> 1) + ((w[3] + w[4] + 1) >> 1) + 1) >> 1;
    unsigned int b, c;
    bool ret;

    c = a * 3;
    b = a * 7 / 6;
    a = a * 5 / 6;
    ret = ((w[0] >= a && w[0] <= b)
        && (w[1] >= a && w[1] <= b)
        && (w[3] >= a && w[3] <= b)
        && (w[4] >= a && w[4] <= b)
        && (w[2] >= (c * 11 / 15) && w[2] <= (c * 17 / 15) && w[2] > (a << 1)));

    if (flag)
        printf("[%d, %d, %d, %d]%d,%d,%d,%d,%d\n", ret, a, b, c, w[0], w[1], w[2], w[3], w[4]);

    return ret;
}

int qr_position_makrings_find(const struct image *img,
        struct qr_position_makrings_info *pqpmi, const unsigned int sz)
{
    int count;
    const unsigned char *imgdata;
    unsigned int i, j, ne, n;
    struct image_raise_fall_edge edges[500];
    unsigned int edges_dist[5];
    //unsigned char buff[640];

#ifdef debug_xx
    struct image *newimg = image_convert_format(img, IMAGE_FORMAT_BGR);
    const unsigned char rec[3] = {0x98, 0x35, 0x95};
    const unsigned char fec[3] = {0x11, 0xbf, 0xF7};
    const unsigned char *ecptr;
#endif

    count = 0;
    imgdata = img->data;
    for (j = 0; j < img->height; ++j, imgdata += img->width) {
        ne = image_find_raise_fall_edges(imgdata, img->width, edges, 500);
        if (ne < 6)
            continue;

        edges_dist[2] = edges[2].dpos - edges[1].dpos;
        edges_dist[3] = edges[3].dpos - edges[2].dpos;
        edges_dist[4] = edges[4].dpos - edges[3].dpos;
        edges_dist[5] = edges[5].dpos - edges[4].dpos;
        n = ne - 6;
        for (i = 0; i <= n; ++i) {
            qr_edges_dist_left_shift_one(edges_dist);
            edges_dist[4] = edges[i + 5].dpos - edges[i + 4].dpos;
            if (j == 370 && edges[i].dpos >= 0 && edges[i].dpos <= 46) {
                flag = 1;
                printf("%d,", edges[i].dpos);
            }
            if (qr_check_finder_mode(edges_dist)) {
#ifdef debug_xx
                for (int xx = 0; xx < 6; ++xx) {
                    if (edges[i + xx].type == IMAGE_RFEDGE_TYPE_RAISE)
                        ecptr = rec;
                    else
                        ecptr = fec;
                    memcpy(newimg->data + j * newimg->row_size + edges[xx + i].dpos * newimg->pixel_size,
                            ecptr, 3);
                }
#endif
            }
            flag = 0;
        }
    }

#ifdef debug_xx
    image_save("ruerue.bmp", newimg, IMAGE_FILE_BITMAP);
    image_release(newimg);
#endif

    return count;
}
