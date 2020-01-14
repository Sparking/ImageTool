#include <stdio.h>
#include <image.h>
#include <maths.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

int get_edgepos_16x(const int *grads, const int len, const int srchpos)
{
    int pos16x, i, j;

    pos16x = 0;
    if (grads[srchpos] > 0) {
        for (i = srchpos; i >= 0; --i) {
            if (grads[i] <= 0)
                break;
        }

        for (j =  grads[i], pos16x = grads[i] * i; i++ < len;) {
            if (grads[i] <= 0)
                break;

            pos16x += grads[i] * i;
            j += grads[i];
        }
    } else {
        for (i = srchpos; i >= 0; --i) {
            if (grads[i] >= 0)
                break;
        }

        for (j =  grads[i], pos16x = grads[i] * i; i++ < len;) {
            if (grads[i] >= 0)
                break;

            pos16x += grads[i] * i;
            j += grads[i];
        }
    }
    pos16x = (pos16x << 4) / j;

    return pos16x;
}

bool get_dots_edge(unsigned char *data, const int len, const int center, int *center_offset, int *w16x, int *isblack)
{
    int grads[100], head, tail;
    unsigned int i, j, npos, tmp;
    unsigned char gradabs[100], maxgrad;
    unsigned char maxgradpos[50];

    for (maxgrad = 0, i = 1; i < len; ++i) {
        grads[i] = (int)data[i] - (int)data[i - 1];
        gradabs[i] = (unsigned char)fabs((int)data[i] - (int)data[i - 1]);
        if (maxgrad < gradabs[i])
            maxgrad = gradabs[i];
    }

    for (npos = 0, i = 1, tmp = len - 2, j = -1; i < tmp && npos < 50; ++i) {
        if ((gradabs[i] > gradabs[i - 1] && gradabs[i] >= gradabs[i + 1]) ||
            (gradabs[i] >= gradabs[i - 1] && gradabs[i] > gradabs[i + 1])) {
            if (gradabs[i] * 5 < (maxgrad << 1))    /**剔除小干扰**/
                continue;

            maxgradpos[npos++] = i;
            if (i >= center && j == -1)
                j = npos - 1;
        }
    }
    if (npos < 2 || j < 1 || center == maxgradpos[j])
        return false;

    i = j - 1;
    head = get_edgepos_16x(grads, len - 1, maxgradpos[i]);
    tail = get_edgepos_16x(grads, len - 1, maxgradpos[j]);
    *w16x = tail - head;
    *center_offset = ((head + tail + 16) >> 5) - center;

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

