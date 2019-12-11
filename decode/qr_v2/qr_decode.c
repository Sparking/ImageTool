#include <stdlib.h>
#include "qr_decode.h"
#include "qr_position.h"

int qr_decode_entry(const struct image *img)
{
    int nqpm;
    struct qr_position_makrings_info qpm[30];

#ifdef CHECK_ENTRY_PARAM
    if (img == NULL)
        return -1;

    if (img->format != IMAGE_FORMAT_GRAY)
        return -1;
#endif

    nqpm = qr_position_makrings_find(img, qpm, 30);
    if (nqpm == 0)
        return -1;

    return 0;
}