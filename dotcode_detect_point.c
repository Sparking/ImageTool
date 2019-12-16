#include "rf_edges.h"
#include "dotcode_detect_point.h"

unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *pdtp, const unsigned int ndtp)
{
    unsigned int i, j;
    unsigned int dtp_size;
    unsigned int rfe_tmp_cnt;
    unsigned int rfe_hori_cnt;
    struct point edge_off;
    struct point edge_start;
    struct point hori_edge_off;
    struct point hori_edge_start;
    struct image_raise_fall_edge rfe_buff[4];
    struct image_raise_fall_edge rfe_hori[500];
    struct image_raise_fall_edge rfe_tmp;

    if (img == NULL || pdtp == NULL || ndtp == 0)
        return 0;

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
            pdtp[dtp_size].nw = (rfe_hori[i].dpos_256x - rfe_hori[i - 1].dpos_256x + 128) >> 8;
            pdtp[dtp_size].center.x = rfe_hori[i].dpos - ((pdtp[dtp_size].nw + 1) >> 1);
            edge_start.x = pdtp[dtp_size].center.x;
            edge_start.y = j;
            edge_off.x = 0;
            edge_off.y = 1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 1);
            if (rfe_tmp_cnt != 1 || rfe_buff[0].type == rfe_hori[i].type)
                continue;
            rfe_tmp = rfe_buff[0];

            edge_off.y = -1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 1);
            if (rfe_tmp_cnt != 1 || rfe_buff[0].type == rfe_hori[i].type)
                continue;

            pdtp[dtp_size].nh = (rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x + 128) >> 8;
            if (pdtp[dtp_size].nh >= (pdtp[dtp_size].nw << 1) || pdtp[dtp_size].nw >= (pdtp[dtp_size].nh << 1))
                continue;
            pdtp[dtp_size].center.y = j - ((pdtp[dtp_size].nh + 1) >> 1) + rfe_tmp.dpos;

            edge_start.x = pdtp[dtp_size].center.x;
            edge_start.y = pdtp[dtp_size].center.y;
            edge_off.x = 1;
            edge_off.y = 1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 1);
            if (rfe_tmp_cnt != 1 || rfe_buff[0].type == rfe_hori[i].type)
                continue;
            rfe_tmp = rfe_buff[0];

            edge_off.x = -1;
            edge_off.y = -1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 1);
            if (rfe_tmp_cnt != 1 || rfe_buff[0].type == rfe_hori[i].type)
                continue;
            pdtp[dtp_size].n45 = (rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x + 128) >> 8;
            if (pdtp[dtp_size].n45 >= (pdtp[dtp_size].nw << 1) || pdtp[dtp_size].nw >= (pdtp[dtp_size].n45 << 1))
                continue;

            edge_start.x = pdtp[dtp_size].center.x;
            edge_start.y = pdtp[dtp_size].center.y;
            edge_off.x = -1;
            edge_off.y = 1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 1);
            if (rfe_tmp_cnt != 1 || rfe_buff[0].type == rfe_hori[i].type)
                continue;
            rfe_tmp = rfe_buff[0];

            edge_off.x = 1;
            edge_off.y = -1;
            rfe_tmp_cnt = image_find_raise_fall_edges_by_offset(img,
                edge_start, edge_off, 1000, rfe_buff, 1);
            if (rfe_tmp_cnt != 1 || rfe_buff[0].type == rfe_hori[i].type)
                continue;
            pdtp[dtp_size].n135 = (rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x + 128) >> 8;
            if (pdtp[dtp_size].n135 >= (pdtp[dtp_size].nw << 1) || pdtp[dtp_size].nw >= (pdtp[dtp_size].n135 << 1))
                continue;
            pdtp[dtp_size].isblack = (rfe_hori[i].type == IMAGE_RFEDGE_TYPE_FALL);
            ++dtp_size;
        }
    }

    return dtp_size;
}

