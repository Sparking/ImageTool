#include <stdio.h>
#include <stdlib.h>
#include "image.h"
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

#if 0
    printf("%d\n", nqpm);
    const unsigned char xxc[3] = {0x00, 0x7E, 0xFF};
    (void)xxc;
    struct image *newimg = image_convert_format(img, IMAGE_FORMAT_BGR);
    for (int i = 0; i < nqpm; ++i) {
        printf("[%d,%d]\n", qpm[i].center.x, qpm[i].center.y);
        img_print_point(newimg, qpm[i].center.x, qpm[i].center.y, xxc, 3);
    }
    image_save("pm.bmp", newimg, IMAGE_FILE_BITMAP);
    image_release(newimg);
#endif

    return 0;
}