#include "rf_edges.h"
#include "dotcode_detect_point.h"

unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *bpdtp, const unsigned int nbdtp,
        struct dotcode_point *wpdtp, const unsigned int wbdtp)
{
    unsigned int cnt[2];
    unsigned int nedges, noedges;
    unsigned int i, j, off[3];
    unsigned int imgdata_nmax;
    unsigned char *imgdata;
    struct point edge_start, edge_off;
    struct image_raise_fall_edge edges[500], *last_edge, *cur_edge;
    struct image_raise_fall_edge oedges[2];

#if defined(CHECK_ENTRY_PARAM)
    if (img == NULL)
        return 0;
#endif

    imgdata_nmax = ((img->width > img->height ? img->width : img->height) * 3) >> 1;
    imgdata = (unsigned char *)mem_alloc(sizeof(unsigned char) * imgdata_nmax);
    if (imgdata == NULL)
        return 0;

    (void)off;
    cnt[0] = cnt[1] = 0;
    for (j = 0, off[0] = 0; j < img->height; j += 4, off[0] += 4 * img->width) {
        nedges = image_find_raise_fall_edges(img->data + off[0], img->width, edges, 500);
        if (nedges < 2)
            continue;

        last_edge = edges;
        cur_edge = edges + 1;
        for (i = 1; i < nedges; ++i, ++last_edge, ++cur_edge) {
            if (last_edge->type == IMAGE_RFEDGE_TYPE_RAISE) {
                wpdtp[cnt[0]].nw = ((cur_edge->dpos_256x - last_edge->dpos_256x) + 128) >> 8;
                wpdtp[cnt[0]].center.x = ((cur_edge->dpos_256x - last_edge->dpos_256x) + 256) >> 9;
                edge_start.x = wpdtp[cnt[0]].center.x;
                edge_start.y = j;
                edge_off.x = 0;
                edge_off.y = 1;
                noedges = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, wpdtp[cnt[0]].nw, oedges, 1);
                if (noedges != 1 || oedges[0].type != IMAGE_RFEDGE_TYPE_RAISE)
                    continue;
                edge_off.y = -1;
                noedges = image_find_raise_fall_edges_by_offset(img, edge_start, edge_off, wpdtp[cnt[0]].nw, oedges + 1, 1);
                if (noedges != 1 || oedges[0].type != IMAGE_RFEDGE_TYPE_RAISE)
                    continue;
                wpdtp[cnt[0]].nh = ((cur_edge->dpos_256x + last_edge->dpos_256x) + 128) >> 8;
                if (unsigned_diff(wpdtp[cnt[0]].nh, wpdtp[cnt[0]].nw) > ((wpdtp[cnt[0]].nh + wpdtp[cnt[0]].nw) >> 1))
                    continue;
                wpdtp[cnt[0]].center.y = (oedges[0].dpos + j) - (wpdtp[cnt[0]].nh >> 1);
                ++cnt[0];
            } else {
                bpdtp[cnt[1]].nw = cur_edge->dpos_256x - last_edge->dpos_256x;
            }
        }
    }

    return cnt[0];
}

