#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <image.h>
#include "kiss_fft.h"
#include "kiss_fftr.h"
#include "kiss_fftnd.h"
#include "kiss_fftndr.h"

int main(int argc, char *argv[])
{
    int i, j;
    int dims[2];
    struct image *img, *gray;
    kiss_fft_cpx *fft_in;
    kiss_fft_cpx *fft_out;
    kiss_fftnd_cfg st;

    if (argc < 1)
        return -1;

    img = image_open(argv[1]);
    if (img == NULL)
        return -1;

    gray = image_convert_gray(img);
    image_release(img);
    if (gray == NULL)
        return -1;

    dims[0] = gray->height;
    dims[1] = gray->width;
    fft_in = (kiss_fft_cpx *)malloc(sizeof(*fft_in) * gray->size * 2);
    if (fft_in == NULL) {
        image_release(gray);
        return -1;
    }

    fft_out = fft_in + gray->size;
    st = kiss_fftnd_alloc(dims, 2, 0, 0, 0);
    if (st == NULL) {
        free(fft_in);
        image_release(gray);
        return -1;
    }

    for (i = 0; i < gray->size; ++i) {
        fft_in[i].r = (kiss_fft_scalar)gray->data[i];
        fft_in[i].i = 0;
    }
    kiss_fftnd(st, fft_in, fft_out);

    img = image_create(gray->height, gray->width, IMAGE_FORMAT_GRAY);
    if (img != NULL) {
        for (i = 0; i < gray->size; ++i) {
            j = (int)(sqrtf(fft_out[i].r * fft_out[i].r /*+ fft_out[i].i * fft_out[i].i*/) / 255.0f + 0.5f);
            if (j >= 255)
                j = 255;
            img->data[i] = j;
        }
        /*
        int off = img->size >> 2;
        char *buff = (char *)malloc(off);

        memcpy(buff, img->data, off);
        memcpy(img->data, img->data + off * 3, off);
        memcpy(img->data + off * 3, buff, off);

        memcpy(buff, img->data + off, off);
        memcpy(img->data + off, img->data + (img->size >> 1), off);
        memcpy(img->data + (img->size >> 1), buff, off);
        */
        image_save("fft.bmp", img, IMAGE_FILE_BITMAP);
        image_release(img);
    }

    free(fft_in);
    image_release(gray);

    return 0;
}
