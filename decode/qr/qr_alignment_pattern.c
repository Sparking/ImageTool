#include <math.h>
#include <stdlib.h>
#include "maths.h"
#include "port_memory.h"
#include "qr_alignment_pattern.h"

static void alignment_combine_esimate(struct qr_alignment_pattern *alignment,
        const unsigned int row, const unsigned int column, const float module_size)
{
    alignment->row = (alignment->row + row) >> 1;
    alignment->column = (alignment->column + column) >> 1;
    alignment->estimated_module_size = (alignment->estimated_module_size + module_size) / 2.0f;
}

static unsigned char alignment_about_equals(const struct qr_alignment_pattern *alignment,
        float module_size, const unsigned int row, const unsigned int column)
{
    if (unsigned_diff(row, alignment->row) <= module_size && unsigned_diff(column, alignment->column) <= module_size) {
        float module_size_diff = (float)fabs(module_size - alignment->estimated_module_size);
        return module_size_diff <= 1.0f || module_size_diff <= alignment->estimated_module_size;
    }

    return 0;
}

static unsigned char found_pattern_cross(const unsigned int count[3], const float module_size)
{
    float max_variance = module_size / 2.0f;

    for (int i = 0; i < 3; ++i) {
        if (fabs(module_size - (float)count[i]) >= max_variance)
            return 0;
    }

    return 1;
}

static unsigned int center_from_end(const unsigned int count[3], const unsigned int end)
{
    return (end - 1 - count[2]) - (count[1] >> 1);
}

static unsigned int cross_check_horizontal(const struct bitmatrix *image_matrix, const unsigned int center_row,
        const unsigned int start_column, const unsigned int max_count, const unsigned int origin_total_count, const float module_size)
{
    unsigned int max_column = image_matrix->column;
    unsigned int count[3] = {0};

    unsigned int column = start_column + 1;
    while (column-- > 0) {
        if (!(!bitmatrix_get(image_matrix, center_row, column) && count[1] <= max_count))
            break;
        ++count[1];
    }
    if (column == (unsigned int)~0 || count[1] > max_count)
        return (unsigned int)~0;

    while (column-- > 0) {
        if (!(bitmatrix_get(image_matrix, center_row, column) && count[0] <= max_count))
            break;
        ++count[0];
    }
    if (count[0] > max_count)
        return (unsigned int)~0;

    column = start_column + 1;
    while (column < max_column && !bitmatrix_get(image_matrix, center_row, column) && count[1] <= max_count) {
        ++count[1];
        ++column;
    }
    if (column == max_column || count[1] > max_count)
        return (unsigned int)~0;

    while (column < max_column && bitmatrix_get(image_matrix, center_row, column) && count[2] <= max_count) {
        ++count[2];
        ++column;
    }
    if (count[2] > max_count)
        return (unsigned int)~0;

    unsigned int total_count = count[0] + count[1] + count[2];
    if (5 * unsigned_diff(total_count, origin_total_count) >= (origin_total_count << 1))
        return (unsigned int)~0;

    return found_pattern_cross(count, module_size) ? center_from_end(count, column) : (unsigned int)~0;
}

static unsigned int cross_check_vertical(const struct bitmatrix *image_matrix, const unsigned int start_row,
        const unsigned int center_column, const unsigned int max_count, const unsigned int origin_total_count, const float module_size)
{
    unsigned int max_row = image_matrix->row;
    unsigned int count[3] = {0};

    unsigned int row = start_row + 1;
    while (row-- > 0) {
        if (!(!bitmatrix_get(image_matrix, row, center_column) && count[1] <= max_count))
            break;
        ++count[1];
    }
    if (row == (unsigned int)~0 || count[1] > max_count)
        return (unsigned int)~0;

    while (row-- > 0) {
        if (!(bitmatrix_get(image_matrix, row, center_column) && count[0] <= max_count))
            break;
        ++count[0];
    }
    if (count[0] > max_count)
        return (unsigned int)~0;

    row = start_row + 1;
    while (row < max_row && !bitmatrix_get(image_matrix, row, center_column) && count[1] <= max_count) {
        ++count[1];
        ++row;
    }
    if (row == max_row || count[1] > max_count)
        return (unsigned int)~0;

    while (row < max_row && bitmatrix_get(image_matrix, row, center_column) && count[2] <= max_count) {
        ++count[2];
        ++row;
    }
    if (count[2] > max_count)
        return (unsigned int)~0;

    unsigned int total_count = count[0] + count[1] + count[2];
    if (5 * unsigned_diff(total_count, origin_total_count) >= (origin_total_count << 1))
        return (unsigned int)~0;

    return found_pattern_cross(count, module_size) ? center_from_end(count, row) : (unsigned int)~0;
}

static struct qr_alignment_pattern *handle_possible_center(struct linkedlist_queue *pattern_queue, const struct bitmatrix *image_matrix,
        const unsigned int count[3], const unsigned int row, const unsigned int column, const float module_size)
{
    unsigned int total_count = count[0] + count[1] + count[2];
    unsigned int center_column = center_from_end(count, column);
    unsigned int center_row = cross_check_vertical(image_matrix, row, center_column, 2 * count[1], total_count, module_size);

    if (center_row == (unsigned int)~0) {
        return NULL;
    }

    center_column = cross_check_horizontal(image_matrix, center_row, center_column, 2 * count[1],total_count, module_size);
    if (center_column != (unsigned int)~0) {
        float estimated_module_size = (count[0] + count[1] + count[2]) / 3.0f;
        struct linkedlist_queue_iterator it;
        for (struct qr_alignment_pattern **pattern = linkedlist_queue_iterator_init(pattern_queue, &it);
                pattern != NULL; pattern = linkedlist_queue_iterator_move(&it, 1)) {
            if (alignment_about_equals(*pattern, estimated_module_size, center_row, center_column)) {
                alignment_combine_esimate(*pattern, center_row, center_column, estimated_module_size);
                return *pattern;
            }
        }

        /* 只保存, 不返回 */
        struct qr_alignment_pattern *alignment;
        alignment = (struct qr_alignment_pattern *)mem_alloc(sizeof(*alignment));
        if (alignment == NULL)
            return NULL;

        alignment->column = center_column;
        alignment->row = center_row;
        alignment->estimated_module_size = estimated_module_size;
        if (linkedlist_queue_enque(pattern_queue, &alignment, 1) != 1)
            mem_free(alignment);
    }

    return NULL;
}

struct qr_alignment_pattern *qr_alignment_pattern_find(struct linkedlist_queue *pattern_queue,
        const struct bitmatrix *image_matrix, const unsigned int start_row, const unsigned int start_column,
        const unsigned int width, const unsigned int height, const float module_size)
{
    struct qr_alignment_pattern *pattern;
    int start_x = (int)start_column;
    int max_i = start_x + (int)width;
    int middle_j = start_row + (int)(height >> 1);

    for (int i_gen = 0; i_gen < (int)height; ++i_gen) {
        int i;
        int j = middle_j + ((i_gen & 0x01) == 0 ? ((i_gen + 1) >> 1) : -((i_gen + 1) >> 1));

        /* 先过滤白色像素 */
        for (i = start_x; i < max_i && !bitmatrix_get(image_matrix, j, i); ++i)
            continue;

        unsigned char current_gray, last_gray;
        unsigned char current_state = 0;
        unsigned int count[3] = {1, 0, 0};
        last_gray = bitmatrix_get(image_matrix, j, i);
        while (++i < max_i) {
            current_gray = bitmatrix_get(image_matrix, j, i);
            if (current_gray == last_gray) {
                ++count[current_state];
                continue;
            }

            last_gray = current_gray;
            if (current_state < 2) {
                ++count[++current_state];
                continue;
            }

            if (found_pattern_cross(count, module_size)) {
                pattern = handle_possible_center(pattern_queue, image_matrix, count, j, i, module_size);
                if (pattern != NULL) {
                    return pattern;
                }
            }

            count[0] = count[2];
            count[1] = 1;
            count[2] = 0;
            current_state = 1;
        }
    }

    pattern = NULL;
    if (linkedlist_queue_size(pattern_queue) != 0)
        linkedlist_queue_deque(pattern_queue, &pattern, 1);

    return pattern;
}
