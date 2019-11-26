#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "maths.h"
#include "queue.h"
#include "qr_decoder.h"
#include "qr_version.h"
#include "qr_finder_pattern.h"
#include "qr_alignment_pattern.h"
#include "port_memory.h"

static float sizeof_black_white_black_run(const struct bitmatrix *image_bitmatrix,
    int fromx, int fromy, int tox, int toy)
{
    // Mild variant of Bresenham's algorithm;
    // see http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
    struct vector diff;
    unsigned char steep = unsigned_diff(fromy, toy) > unsigned_diff(fromx, tox);

    if (steep) {
        int temp = fromx;
        fromx = fromy;
        fromy = temp;
        temp = tox;
        tox = toy;
        toy = temp;
    }

    int dx = (int)unsigned_diff(fromx, tox);
    int dy = (int)unsigned_diff(fromy, toy);
    int error = -(dx >> 1);
    int xstep = fromx < tox ? 1 : -1;
    int ystep = fromy < toy ? 1 : -1;

    int state = 0;
    int xlimit = tox + xstep;
    for (int x = fromx, y = fromy; x != xlimit; x += xstep) {
        int realx = steep ? y : x;
        int realy = steep ? x : y;

        if ((state == 1) == !bitmatrix_get(image_bitmatrix, realy, realx)) {
            if (state == 2) {
                diff.i = x - fromx;
                diff.j = y - fromy;

                return vector_length(&diff);
            }
            ++state;
        }

        error += dy;
        if (error > 0) {
            if (y == toy)
                break;
            y += ystep;
            error = -dx;
        }
    }

    if (state == 2) {
        diff.i = tox - fromx + xstep;
        diff.j = toy - fromy;

        return vector_length(&diff);
    }

    return FLT_MAX;
}

static float sizeof_black_white_black_run_both_ways(const struct bitmatrix *image_bitmatrix,
    int fromx, int fromy, int tox, int toy)
{
    float result = sizeof_black_white_black_run(image_bitmatrix, fromx, fromy, tox, toy);

    float scale = 1.0f;
    int other_to_x = fromx - (tox - fromx);
    if (other_to_x < 0) {
        scale = fromx / (float)(fromx - other_to_x);
        other_to_x = 0;
    } else if (other_to_x >= (int)image_bitmatrix->column) {
        scale = (image_bitmatrix->column - 1 - fromx) / (float)(other_to_x - fromx);
        other_to_x = image_bitmatrix->column - 1;
    }
    int other_to_y = (int)(fromy - (toy - fromy) * scale);

    scale = 1.0f;
    if (other_to_y < 0) {
        scale = fromy /(float)(fromy - other_to_y);
        other_to_y = 0;
    } else if (other_to_y >= (int)image_bitmatrix->row) {
        scale = (image_bitmatrix->row - 1 - fromy) / (float)(other_to_y - fromy);
        other_to_y = image_bitmatrix->row - 1;
    }
    other_to_x = (int)(fromx + (other_to_x - fromx) * scale);
    result += sizeof_black_white_black_run(image_bitmatrix, fromx, fromy, other_to_x, other_to_y);

    return result - 1.0f;
}

static float calculate_module_size_one_way(const struct bitmatrix *image_bitmatrix,
        const struct qr_finder_pattern_center *pattern1, const struct qr_finder_pattern_center *pattern2)
{
    float module_size1, module_size2;

    module_size1 = sizeof_black_white_black_run_both_ways(image_bitmatrix,
            (int)pattern1->column, (int)pattern1->row, (int)pattern2->column, (int)pattern2->row);
    module_size2 = sizeof_black_white_black_run_both_ways(image_bitmatrix,
            (int)pattern2->column, (int)pattern2->row, (int)pattern1->column, (int)pattern1->row);
    return (module_size2 + module_size1) / 14.0f;
}

static float calculate_module_size(const struct bitmatrix *image_bitmatrix, const struct qr_finder_pattern_info *info)
{
    return (calculate_module_size_one_way(image_bitmatrix, &info->left_top, &info->right_top) +
            calculate_module_size_one_way(image_bitmatrix, &info->left_top, &info->left_bottom)) / 2.0f;
}

static unsigned int compute_dimension(const struct qr_finder_pattern_info *info, const float module_size)
{
    struct point tl = {info->left_top.column, info->left_top.row};
    struct point tr = {info->right_top.column, info->right_top.row};
    struct point bl = {info->left_bottom.column, info->left_bottom.row};
    unsigned int tltr_centers_dimension = (unsigned int)roundf(points_distance(&tl, &tr) / module_size);
    unsigned int tlbl_centers_dimension = (unsigned int)roundf(points_distance(&tl, &bl) / module_size);
    unsigned int dimension = (unsigned int)((tlbl_centers_dimension + tltr_centers_dimension) / 2.0f) + 7;

    switch (dimension & 0x03) {
    case 0:
        dimension++;
        break;
    case 2:
        dimension--;
        break;
    case 3:
        //dimension = (unsigned int)~0;
        dimension -= 2;
    }

    return dimension;
}

static inline int _max(const int a, const int b)
{
    return a >= b ? a : b;
}

static inline int _min(const int a, const int b)
{
    return a <= b ? a : b;
}

static struct qr_alignment_pattern *find_alignment_in_region(struct linkedlist_queue *pattern_queue, const struct bitmatrix *image_bitmatrix,
        const float overall_est_module_size, const unsigned int est_alignment_x, const unsigned int est_alignment_y, const float allowance_factor)
{
    unsigned int allowance = (unsigned int)(allowance_factor * overall_est_module_size);
    int alignment_area_left_x = _max(0, (int)est_alignment_x - (int)allowance);
    int alignment_area_right_x = _min(image_bitmatrix->column - 1, est_alignment_x + allowance);

    if (alignment_area_right_x - alignment_area_left_x < 3.0f * overall_est_module_size)
        return NULL;

    int alignment_area_top_y = _max(0, (int)est_alignment_y - (int)allowance);
    int alignment_area_bottom_y = _min(image_bitmatrix->row - 1, est_alignment_y + allowance);
    if (alignment_area_bottom_y - alignment_area_top_y < 3.0f * overall_est_module_size)
        return NULL;

    return qr_alignment_pattern_find(pattern_queue, image_bitmatrix, alignment_area_top_y, alignment_area_left_x,
            alignment_area_right_x - alignment_area_left_x,
            alignment_area_bottom_y - alignment_area_top_y, overall_est_module_size);
}

static struct qr_alignment_pattern *process_finder_pattern_info(const struct bitmatrix *image_bitmatrix,
        struct qr_finder_pattern_info *info)
{
    struct qr_alignment_pattern *pattern;
    float module_size;
    unsigned int dimension;

    module_size = calculate_module_size(image_bitmatrix, info);
    dimension = compute_dimension(info, module_size);
    if (dimension == (unsigned int)~0)
        return NULL;

    pattern = NULL;
    struct linkedlist_queue pattern_queue;
    unsigned int provisional_version = (dimension - 17) >> 2;
    unsigned int modules_between_FP_centers = (provisional_version << 2) + 17 - 7;

    linkedlist_queue_init(&pattern_queue, sizeof(struct qr_alignment_pattern *));
    if (qr_version_blocks[provisional_version - 1].alignment_pattern_num > 0) {
        /* 粗略计算出右下角的校正图形所在的点 */
        unsigned int bottom_right_x = info->right_top.column + info->left_bottom.column - info->left_top.column;
        unsigned int bottom_right_y = info->right_top.row + info->left_bottom.row - info->left_top.row;
        float correction_to_top_left = 1.0f - 3.0f / modules_between_FP_centers;
        unsigned int estAlignment_x = (unsigned int)((int)info->left_top.column + (int)(correction_to_top_left * ((int)bottom_right_x - (int)info->left_top.column)));
        unsigned int estAlignment_y = (unsigned int)((int)info->left_top.row + (int)(correction_to_top_left * ((int)bottom_right_y - (int)info->left_top.row)));

        for (int i = 4; i <= 16; i <<= 1) {
            pattern = find_alignment_in_region(&pattern_queue, image_bitmatrix, module_size, estAlignment_x, estAlignment_y, (float)i);
            if (pattern != NULL)
                break;
        }


        while (linkedlist_queue_size(&pattern_queue) != 0) {
            void *temp;

            linkedlist_queue_deque(&pattern_queue, &temp, 1);
            mem_free(temp);
        }
    }
    while (linkedlist_queue_size(&pattern_queue) != 0) {
        void *temp;

        linkedlist_queue_deque(&pattern_queue, &temp, 1);
        mem_free(temp);
    }

    return pattern;
}

static struct bitmatrix *qr_sample_data(const struct bitmatrix *image_bitmatrix, const struct qr_finder_pattern_info *info)
{
    unsigned int row_num, column_num;
    unsigned int *row_coordinate, *column_coordinate;
    struct bitmatrix *sample_data;
    float module_size = calculate_module_size(image_bitmatrix, info);

    row_coordinate = (unsigned int *)mem_alloc(sizeof(unsigned int) * (image_bitmatrix->row + image_bitmatrix->column));
    if (row_coordinate == NULL)
        return NULL;

    row_num = column_num = 0;
    column_coordinate = row_coordinate + image_bitmatrix->row;
    memset(row_coordinate, 0, sizeof(unsigned int) * (image_bitmatrix->row + image_bitmatrix->column));

    {
        column_coordinate[3] = info->left_top.column;
        column_coordinate[2] = column_coordinate[3] - (unsigned int)module_size;
        column_coordinate[1] = column_coordinate[2] - (unsigned int)module_size;
        column_coordinate[0] = column_coordinate[1] - (unsigned int)module_size;
        column_coordinate[4] = column_coordinate[3] + (unsigned int)module_size;
        column_coordinate[5] = column_coordinate[4] + (unsigned int)module_size;
        column_coordinate[6] = column_coordinate[5] + (unsigned int)module_size;
        column_num = 7;

        unsigned char last_gray, current_gray;
        unsigned int begin, i;
        unsigned int scan_row = info->left_top.row + (unsigned int)(3 * module_size);

        for (i = column_coordinate[6]; i < image_bitmatrix->column; ++i) {   /* 过滤黑色 */
            last_gray = bitmatrix_get(image_bitmatrix, scan_row, i);
            if (last_gray != 0)
                break;
        }

        if (i == image_bitmatrix->column) {
            mem_free(row_coordinate);
            return NULL;
        }

        begin = i;
        for (; i < info->right_top.column - (unsigned int)(3 * module_size); ++i) {
            current_gray = bitmatrix_get(image_bitmatrix, scan_row, i);
            if (current_gray != last_gray) {
                last_gray = current_gray;
                column_coordinate[column_num++] = (begin + i) >> 1;
                begin = i;
            }
        }

        column_coordinate[column_num++] = info->right_top.column - (unsigned int)(3 * module_size);
        column_coordinate[column_num++] = info->right_top.column - (unsigned int)(2 * module_size);
        column_coordinate[column_num++] = info->right_top.column - (unsigned int)module_size;
        column_coordinate[column_num++] = info->right_top.column;
        column_coordinate[column_num++] = info->right_top.column + (unsigned int)module_size;
        column_coordinate[column_num++] = info->right_top.column + (unsigned int)(2 * module_size);
        column_coordinate[column_num++] = info->right_top.column + (unsigned int)(3 * module_size);
    }

    {
        row_coordinate[3] = info->left_top.row;
        row_coordinate[2] = row_coordinate[3] - (unsigned int)module_size;
        row_coordinate[1] = row_coordinate[2] - (unsigned int)module_size;
        row_coordinate[0] = row_coordinate[1] - (unsigned int)module_size;
        row_coordinate[4] = row_coordinate[3] + (unsigned int)module_size;
        row_coordinate[5] = row_coordinate[4] + (unsigned int)module_size;
        row_coordinate[6] = row_coordinate[5] + (unsigned int)module_size;
        row_num = 7;

        unsigned char last_gray, current_gray;
        unsigned int begin, i;
        unsigned int scan_column = info->left_top.column + (unsigned int)(3 * module_size);

        for (i = row_coordinate[6]; i < image_bitmatrix->row; ++i) {   /* 过滤黑色 */
            last_gray = bitmatrix_get(image_bitmatrix, i, scan_column);
            if (last_gray != 0)
                break;
        }

        if (i == image_bitmatrix->row) {
            mem_free(row_coordinate);
            return NULL;
        }

        begin = i;
        for (; i < info->left_bottom.row - (unsigned int)(3 * module_size); ++i) {
            current_gray = bitmatrix_get(image_bitmatrix, i, scan_column);
            if (current_gray != last_gray) {
                last_gray = current_gray;
                row_coordinate[row_num++] = (begin + i) >> 1;
                begin = i;
            }
        }

        row_coordinate[row_num++] = info->left_bottom.row - (unsigned int)(3 * module_size);
        row_coordinate[row_num++] = info->left_bottom.row - (unsigned int)(2 * module_size);
        row_coordinate[row_num++] = info->left_bottom.row - (unsigned int)module_size;
        row_coordinate[row_num++] = info->left_bottom.row;
        row_coordinate[row_num++] = info->left_bottom.row + (unsigned int)module_size;
        row_coordinate[row_num++] = info->left_bottom.row + (unsigned int)(2 * module_size);
        row_coordinate[row_num++] = info->left_bottom.row + (unsigned int)(3 * module_size);
    }


    unsigned int version = (row_num - 17) >> 2;
    if (version < 1 || version > 40 || row_num != column_num) {
        mem_free(row_coordinate);
        return NULL;
    }

    sample_data = bitmatrix_create(row_num, column_num, 0);
    if (sample_data != NULL) {
        for (unsigned int i, j = 0; j < row_num; ++j) {
            for (i = 0; i < column_num; ++i) {
                bitmatrix_set(sample_data, j, i,
                    !bitmatrix_get(image_bitmatrix, row_coordinate[j], column_coordinate[i]));
            }
        }
    }

    mem_free(row_coordinate);
    return sample_data;
}

struct bitmatrix *qr_detect(const struct image *img)
{
    struct bitmatrix *qr_image, *origin_img_matrix;
    struct qr_finder_pattern_info *info;

    origin_img_matrix = image_create_bitmatrix(img);
    if (origin_img_matrix == NULL) {
        return NULL;
    }

    info = qr_finder_pattern_find(origin_img_matrix);
    if (info == NULL) {
        bitmatrix_release(origin_img_matrix);
        return NULL;
    }

    qr_image = qr_sample_data(origin_img_matrix, info);
    bitmatrix_release(origin_img_matrix);
    (void)process_finder_pattern_info;
    mem_free(info);

    return qr_image;
}
