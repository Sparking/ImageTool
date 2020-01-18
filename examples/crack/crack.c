#include <math.h>
#include <stdlib.h>

unsigned int DCDotPatterns[113] =
{
  341, 171, 173, 181, 213, 342, 346, 362, 426, 174,
  182, 186, 214, 218, 234, 299, 301, 309, 331, 333,
  339, 345, 357, 361, 405, 421, 425, 87, 91, 93,
  107, 109, 117, 151, 155, 157, 167, 179, 185, 203,
  205, 211, 217, 229, 233, 302, 310, 314, 334, 348,
  358, 364, 370, 372, 406, 410, 422, 428, 434, 436,
  458, 466, 468, 94, 110, 118, 122, 158, 188, 206,
  220, 230, 236, 242, 244, 279, 283, 285, 295, 307,
  313, 327, 355, 369, 395, 397, 403, 409, 419, 433,
  453, 457, 465, 47, 55, 59, 61, 79, 103, 115, 121,
  143, 199, 227, 241, 286, 316, 376, 398, 412, 440,
  454, 460
}; // idb
int DCPARAM_max_block_search = 1; // weak
int DCPARAM_max_fft_per_block = 4; // weak
int DCPARAM_max_dots_per_block = 8; // weak
int DCPARAM_dot_confirm_angles = 4; // weak
int DCPARAM_dot_confirm_step; // weak
float DCPARAM_max_dot_size; // weak
float DCPARAM_min_dot_size; // weak
int DCPARAM_confirm_max_deviation_2; // weak
float DCPARAM_confirm_max_deviation_1; // weak
float DCPARAM_dots_angle90_tollerance; // weak
float DCPARAM_center_dot_radius_tollerance; // weak
int DOTCODE_best_EC = -1; // weak
float *dword_11DCE0 = NULL;

int getThreshold(const unsigned char *imgdata, const int width, const int height,
        const int x, const int y, const int off_x, const int off_y)
{
    int row, row_end;
    int col, col_end;
    const unsigned char *ptr;
    unsigned char data;
    unsigned char max;
    unsigned char min;

    max = 0;
    min = 255;
    row = y;
    row_end = y + off_y;
    col_end = x + off_x;
    ptr = imgdata + y * width;
    for (row = y; row < row_end; ++row, ptr += width) {
        for (col = x; col < col_end; ++col) {
            data = ptr[col];
            if (min > data)
                min = data;
            if (max < data)
                max = data;
        }

    }

    return ((int)max + (int)min) >> 1;
}

int DOTCODE_scanGrayscale(int *a1, unsigned int *a2)
{
    float v14;
    float v15;
    float v16;
    float v17;
    float *v18;
    float v19;
    float v20;
    float v23, v24, v25;
    int v26, v27, i;
    char *ptr, v68;

    DOTCODE_best_EC = -1;
    *(unsigned int *)(NULL + (a2[66] + 1908)) = -1;
    if (dword_11DCE0) {
        v14 = fmax(dword_11DCE0[0], 0.0);
        v15 = fmax(dword_11DCE0[1], 0.0);
        v16 = fmin(dword_11DCE0[3], 100.0);
        v17 = fmin(dword_11DCE0[0] + dword_11DCE0[2], 100.0);
    } else {
        v14 = 0.0;
        v15 = 0.0;
        v16 = 100.0;
        v17 = 100.0;
    }

    v18 = (float *)a2[52];
    if (v18 == NULL) {
        v18 = (float *)malloc(sizeof(float) * 4);
        a2[52] = v18;
    }

    v19 = (float)v2[1];
    v18[0] = (float)(v14 * v19) / 100.0;
    v19 = (float)v2[2];
    v20 = (float)(v16 - v15) * v19;
    v23 = v15 * v19;
    v24 = (v17 - v14) * v19 / 100.0;
    v25 = v20 / 100.0;
    v26 = (int)v24;
    v27 = (int)v25;
    v18[3] = v25;
    v18[2] = v24;
    v18[1] = v23 / 100.0;
    ptr = (char *)malloc(v26 * v27);
    v31 = (int)(v23 / 100);
    i = 0;
    v68 = ptr;
    while (i < v27) {
        memcpy(ptr, *(unsigned int *)(v2[0] + 4 * v31 + 4 * i) + (int)v20, v26);
        ptr += v26;
        ++i;
    }


    return v14;
}

void MWB_scanGrayScaleImage(const unsigned char *imgdata, const int width, const int height, char **decode_result)
{
}

//3个参数
int sub_2F1AC(const void *param)
{
    int *v1;
    int v2;
    int v5;
    float v3;
    float v4;
    char *v6;
    int result;
    int v8;
    int v13, v14;
    int v15;
    unsigned int v16, v17, v18, v19;
    float v20, v21;

    v5 = 
}
