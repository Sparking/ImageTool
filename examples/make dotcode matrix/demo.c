#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "image.h"

typedef struct {
    int x;
    int y;
} POINT;

typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} RECT;

#define DTMH   300
#define DTMW   300
struct dotcode_matrix {
    int nw;
    int nh;
    RECT area;  /**矩阵的范围*/
    unsigned char m[DTMH * DTMW];
};

void point_rotate45(const POINT *pt, POINT *pt45)
{
    pt45->x = pt->x - pt->y;
    pt45->y = -(pt->x + pt->y);
}

void dotcode_matrix_init(struct dotcode_matrix *pdm)
{
    memset(pdm, 0, sizeof(*pdm));
    pdm->area.top = DTMH;
    pdm->area.left = DTMW;
}

bool dotcode_matrix_set(struct dotcode_matrix *pdm, const int x, const int y)
{
    if (x < 0 || x >= DTMW || y < 0 || y >= DTMH)
        return false;

    if (x < pdm->area.left)
        pdm->area.left = x;

    if (x > pdm->area.right)
        pdm->area.right = x;

    if (y < pdm->area.top)
        pdm->area.top = y;

    if (y > pdm->area.bottom)
        pdm->area.bottom = y;
    pdm->m[x + y * DTMW] = 0xFF;

    return true;
}

void dotcode_matrix_reshape(struct dotcode_matrix *pdm)
{
    int i, j;

    pdm->nw = pdm->area.right - pdm->area.left + 1;
    pdm->nh = pdm->area.bottom - pdm->area.top + 1;
    for (i = 0, j = pdm->area.top; i < pdm->nh; ++i, j += DTMW)
        memcpy(pdm->m + i * pdm->nw, pdm->m + j + pdm->area.left, pdm->nw);

    pdm->area.left = 0;
    pdm->area.right = pdm->nw - 1;
    pdm->area.top = 0;
    pdm->area.bottom = pdm->nh - 1;
}

int main(void)
{
    struct image *img;
    struct dotcode_matrix dm;

    dotcode_matrix_init(&dm);
    dotcode_matrix_set(&dm, 1, 3);
    dotcode_matrix_set(&dm, 9, 7);
    dotcode_matrix_set(&dm, 1, 0);
    dotcode_matrix_reshape(&dm);
    img = image_create(dm.nh, dm.nw, IMAGE_FORMAT_GRAY);
    memcpy(img->data, dm.m, img->size);
    image_save("test.bmp", img, IMAGE_FILE_BITMAP);
    image_release(img);

    return 0;
}
