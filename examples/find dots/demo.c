#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <image.h>
#include <maths.h>
#include <port_memory.h>
#include <dotcode_detect_point.h>

int main(int argc, char *argv[])
{
    struct image *img, *gray;

    if (argc < 2) {
        fprintf(stderr, "param not engouh\n");
        return -1;
    }

    img = image_open(argv[1]);
    if (img == NULL)
        return -1;

    gray = image_convert_gray(img);
    image_release(img);
    if (gray == NULL)
        return -1;

    img = gray;
    image_find_dot_by_grad(img);
    image_release(img);

    return 0;
}

