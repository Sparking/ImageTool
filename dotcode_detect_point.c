#include "rf_edges.h"
#include "dotcode_detect_point.h"

unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *bpdtp, const unsigned int nbdtp,
        struct dotcode_point *wpdtp, const unsigned int wbdtp)
{
    unsigned int cnt[2];
    unsigned int nedges;
    unsigned int i, j, off[3];
    unsigned int imgdata_nmax;
    unsigned char *imgdata;
    struct image_raise_fall_edge edges[500], *last_edge, *cur_edge;

#if defined(CHECK_ENTRY_PARAM)
    if (img == NULL)
        return 0;
#endif

    imgdata_nmax = ((img->width > img->height ? img->width : img->height) * 3) >> 1;
    imgdata = (unsigned char *)mem_alloc(sizeof(unsigned char) * imgdata_nmax);
    if (imgdata == NULL)
        return 0;

    cnt[0] = cnt[1] = 0;
    for (j = 0, off[0] = 0; j < img->height; j += 4, off[0] += 4 * img->width) {
        nedges = image_find_raise_fall_edges(img->data + j * img->row_size, img->width, edges, 500);
        if (nedges < 2)
            continue;

        last_edge = edges;
        cur_edge = edges + 1;
        for (i = 1; i < nedges; ++i, ++last_edge) {
            if (last_edge->type == IMAGE_RFEDGE_TYPE_RAISE) {
                wpdtp[cnt[0]].nw = ((cur_edge->dpos_256x - last_edge->dpos_256x) + 128) >> 8;
                wpdtp[cnt[0]].center.x = ((cur_edge->dpos_256x - last_edge->dpos_256x) + 256) >> 9;
                for (off[1] = j, off[2] = j + wpdtp[cnt[0]].nw; off[1] < img->height && off[1] < off[2]; ++off[1]) {
                    ;
                }
            } else {
                bpdtp[cnt[1]].nw = cur_edge->dpos_256x - last_edge->dpos_256x;
            }
        }
    }

    return cnt[0];
}

