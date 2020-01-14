#include <stdio.h>
#include <image.h>
#include <maths.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

bool get_dots_edge(unsigned char *data, const int len, const int center, int *new_center, int *w16x, int *isblack)
{
    unsigned int i, j, npos;
    unsigned char gradabs[100], maxgrad;
    unsigned char maxgradpos[50];

    assert(center > 0 && len > center && len <= 100);
    maxgrad = 0;
    for (i = 1; i < len; ++i) {
        gradabs[i] = (unsigned char)unsigned_diff(data[i], data[i - 1]);
        if (maxgrad < gradabs[i])
            maxgrad = gradabs[i];
        printf("%3d ", data[i]);
    }
    putchar('\n');
    for (i = 1; i < len; ++i) {
        printf("%3d ", gradabs[i]);
    }
    putchar('\n');

    for (npos = 0, i = 1, j = len - 2; i < j && npos < 50; ++i) {
        if ((gradabs[i] > gradabs[i - 1] && gradabs[i] >= gradabs[i + 1]) ||
            (gradabs[i] >= gradabs[i - 1] && gradabs[i] > gradabs[i + 1])) {
            maxgradpos[npos++] = i;
            printf("%3d ", i);
        }
    }
    putchar('\n');

    for (i = 0; i < npos; ++i) {
        if (maxgradpos[i] >= center)
            break;
    }
    if (i == npos || i == 0)
        return false;


    j = i + 1;
    if (maxgradpos[j] - maxgradpos[i] == 1) {
        ++j;
    }
    printf("p: %d %d\n", data[maxgradpos[i]], data[maxgradpos[j]]);
    *w16x = maxgradpos[j] - maxgradpos[i] + 1;
    *isblack = (((int)data[maxgradpos[i]] + (int)data[maxgradpos[j]]) >> 1) < data[(*w16x + 1) / 2];
    printf("%d %d\n", maxgradpos[i], maxgradpos[j]);
    printf("%d\n", *w16x);
    *new_center = ((int)maxgradpos[i]) - center - 1 + (*w16x + 1) / 2;
    printf("isblack: %d, center off: %d\n", *isblack, *new_center);

    return true;
}

int image_find_dot_by_grad(const struct image *srcimg, const struct point *pt, const int len)
{
    int i, j, n;
    struct image *gray, *img;
    unsigned char imgdata[480];

    gray = image_convert_gray(srcimg);
    if (gray == NULL)
        return -1;

    img = image_convert_format(gray, IMAGE_FORMAT_BGRA);
    if (img == NULL) {
        image_release(gray);
        return -1;
    }

    n = 0;
    i = pt->y * gray->width + pt->x;
    j = i + len + 1;
    i = i - len - 1;
    while (i <= j)
        imgdata[n++] = gray->data[i++];

    if (get_dots_edge(imgdata, n, len + 1, &i, &n, &j)) {
        printf("center: %d\n", pt->x + i);
    } else {
        printf("not found center\n");
    }
    image_release(gray);
    image_release(img);

    return 0;
}

int main(int argc, char *argv[])
{
    struct point pt;
    struct image *img;

    if (argc < 5) {
        fprintf(stderr, "param not engouh\n");
        return -1;
    }

    img = image_open(argv[1]);
    if (img == NULL)
        return -1;

    pt.x = atoi(argv[2]);
    pt.y = atoi(argv[3]);
    image_find_dot_by_grad(img, &pt, atoi(argv[4]));
    image_release(img);

    return 0;
}

