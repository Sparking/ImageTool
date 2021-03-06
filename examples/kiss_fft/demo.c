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
    float j, max;
    int dims[2];
    unsigned int i;
    struct image *img, *gray;
    kiss_fft_scalar *fft_in;
    kiss_fft_cpx *fft_out;
    kiss_fftndr_cfg st;

    if (argc < 1)
        return -1;

    img = image_open(argv[1]);
    if (img == NULL)
        return -1;

    gray = image_convert_gray(img);
    image_release(img);
    if (gray == NULL)
        return -1;

    dims[0] = gray->width;
    dims[1] = gray->height;
    fft_in = (kiss_fft_scalar *)KISS_FFT_MALLOC((sizeof(*fft_in) + sizeof(*fft_out)) * gray->size);
    if (fft_in == NULL) {
        image_release(gray);
        return -1;
    }

    fft_out = (kiss_fft_cpx *)(fft_in + gray->size);
    st = kiss_fftndr_alloc(dims, 2, 0, 0, 0);
    if (st == NULL) {
        KISS_FFT_FREE(fft_in);
        image_release(gray);
        return -1;
    }

    for (i = 0; i < gray->size; ++i)
        fft_in[i] = (kiss_fft_scalar)gray->data[i];
    kiss_fftndr(st, fft_in, fft_out);
    kiss_fftndr_free(st);

    img = image_create(gray->height, gray->width, IMAGE_FORMAT_GRAY);
    if (img != NULL) {
        //st = kiss_fftndr_alloc(dims, 2, 1, 0, 0);
        //kiss_fftndri(st, fft_out, fft_in);
        max = 0;
        for (i = 0; i < img->size; ++i) {
            j = logf(sqrtf(fft_out[i].r * fft_out[i].r + fft_out[i].i * fft_out[i].i) + 1.0f);
            fft_out[i].r = j;
            if (j > max)
                max = j;
        }
        j = 255.0f / max;
        for (i = 0; i < img->size; ++i) {
            img->data[i] = fft_out[i].r * j;
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

    KISS_FFT_FREE(fft_in);
    image_release(gray);

    return 0;
}
