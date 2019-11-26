#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "qr_position.h"

struct qr_spect_finder_info {
    int16_t type_len; // 最高1位: 0, 增边界 1, 减边界; 其余位: 长度
    int16_t pos;      // 开始位置
};

int qr_hori_scan_spect_finder(const struct image *img,
        struct qr_spect_finder_info *pqsfi, const unsigned int sz)
{
    int count;
    unsigned int i, j, offset;

#ifdef CHECK_ENTRY_PARAM
    if (img == NULL || pqsfi == NULL || sz == 0)
        return 0;
#endif

    count = 0;
    for (j = 0, offset = 0; j < img->height; ++j) {
        for (i = 0; i < img->width; ++i, ++offset) {

        }
    }


    return count;
}

int qr_position_makrings_find(const struct image *img,
        struct qr_position_makrings_info *pqpmi, const unsigned int sz)
{
    int count;
    int spect_finder_count;
    struct qr_spect_finder_info qsfi[256];

    count = 0;
    spect_finder_count = qr_hori_scan_spect_finder(img, qsfi, 256);
    if (spect_finder_count == 0)
        return 0;

    return count;
}
