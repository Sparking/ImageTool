#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "maths.h"

int image_draw_line(struct image *img, struct point *start, struct point *end)
{
    int i;
    int x, y, x1, x2, y1, y2;
    int a, b;
    int dx, dy;
    int d, d1, d2;

    x1 = start->x, x2 = end->x;
    y1 = start->y, y2 = end->y;
    y1 = y1 < 0 ? 0 : y1;
    y1 = y1 > img->height - 1 ? img->height - 1 : y1;
    y2 = y2 < 0 ? 0 : y2;
    y2 = y2 > img->height - 1 ? img->height - 1 : y2;
    x1 = x1 < 0 ? 0 : x1;
    x1 = x1 > img->width - 1 ? img->width - 1 : x1;
    x2 = x2 < 0 ? 0: x2;
    x2 = x2 > img->width - 1 ? img->width - 1 : x2;
    a = y1 - y2;
    b = x2 - x1;
    x = x1;
    y = y1;
    dx = b < 0 ? (b = -b, -1) : 1;
    dy = a > 0 ? (a = -a, -1) : 1;
    i = 0;
    if (-a < b) {
        d = (a << 1) + b;
        d1 = a << 1;
        d2 = (a + b) << 1;
        while (x != x2) {
            if (d < 0) {
                y += dy;
                d += d2;
            } else {
                d += d1;
            }
            x += dx;
            img->data[y * img->width + x] = 255;
            i++;
        }
    } else {
        d = (b << 1) + a;
        d1 = b << 1;
        d2 = (b << 1) + (a << 2);
        while (y != y2) {
            if (d >= 0) {
                x += dx;
                d += d2;
            } else {
                d += d1;
            }

            y += dy;
            img->data[y * img->width + x] = 255;
            i++;
        }
    }
    return 0;
}

int main(void)
{
    struct point a, b;
    struct image *img;

    printf("enter data:");
    scanf("%d %d %d %d", &a.x, &a.y, &b.x, &b.y);
    img = image_create(a.y > b.y ? a.y: b.y, a.x > b.x ? a.x : b.x, IMAGE_FORMAT_GRAY);

    image_draw_line(img, &a, &b);

    image_save("xxx.bmp", img, IMAGE_FILE_BITMAP);
    image_release(img);

    return 0;
}
