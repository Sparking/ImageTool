#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "maths.h"
#include "qr_finder_pattern.h"
#include "port_memory.h"

struct qr_finder_pattern {
    unsigned int row;
    unsigned int column;
    float estimate_modules_size;
    unsigned int count;
};

struct qr_finder_pattern_finder {
    const struct bitmatrix *image;
    struct linkedlist_queue finder_pattern;
};

static void shift2counters(unsigned int n[5])
{
    n[0] = n[2];
    n[1] = n[3];
    n[2] = n[4];
    n[3] = 1;
    n[4] = 0;
}

/* 判断比例是否为1:1:3:1:1 */
static unsigned char found_patttern_cross(const unsigned int count[5])
{
    unsigned int total_module_size = 0;

    for (unsigned int i = 0; i < 5; ++i) {
        if (count[i] == 0)
            return 0;

        total_module_size += count[i];
    }

    if (total_module_size < 7)
        return 0;

    const float module_size = total_module_size / 7.0f;
    const float max_variance = module_size / 2.0f;

    return fabs(module_size - (float)count[0]) < max_variance &&
        fabs(module_size - (float)count[1]) < max_variance &&
        fabs(3.0f * module_size - (float)count[2]) < 3 * max_variance &&
        fabs(module_size - (float)count[3]) < max_variance &&
        fabs(module_size - (float)count[4]) < max_variance;
}

static unsigned char found_pattern_diagonal(const unsigned int count[5])
{
    unsigned int total_module_size = 0;

    for (unsigned int i = 0; i < 5; i++) {
      if (count[i] == 0)
        return 0;

      total_module_size += count[i];
    }

    if (total_module_size < 7)
      return 0;

    const float module_size = total_module_size / 7.0f;
    const float max_variance = module_size / 1.333f;
    // Allow less than 75% variance from 1-1-3-1-1 proportions

    return  fabs(module_size - (float)count[0]) < max_variance &&
            fabs(module_size - (float)count[1]) < max_variance &&
            fabs(3.0f * module_size - (float)count[2]) < 3 * max_variance &&
            fabs(module_size - (float)count[3]) < max_variance &&
            fabs(module_size - (float)count[4]) < max_variance;
}

/* 计算1:1:3:1:1中心点的坐标 */
static unsigned int center_from_end(const unsigned int count[5], const unsigned int end)
{
    return (end - 1 - count[4] - count[3]) - (count[2] >> 1);
}

/* 检查对角线 */
static unsigned char cross_check_diagnoal(const struct qr_finder_pattern_finder *finder,
    const unsigned int center_row, const unsigned int center_column)
{
    unsigned int i = 0;
    unsigned int count[5];

    memset(count, 0, sizeof(count));
    while (center_row >= i && center_column >= i
            && !bitmatrix_get(finder->image, center_row - i, center_column - i)) {
        count[2]++;
        i++;
    }
    if (count[2] == 0)
        return 0;

    while (center_row >= i && center_column >= i
            && bitmatrix_get(finder->image, center_row - i, center_column - i)) {
        count[1]++;
        i++;
    }
    if (count[1] == 0)
        return 0;

    while (center_row >= i && center_column >= i
            && bitmatrix_get(finder->image, center_row - i, center_column - i) == 0) {
        count[0]++;
        i++;
    }
    if (count[0] == 0)
        return 0;

    const unsigned int max_i = finder->image->row;
    const unsigned int max_j = finder->image->column;
    i = 1;
    while (center_row + i < max_i && center_column + i < max_j
            && bitmatrix_get(finder->image, center_row + i, center_column + i) == 0) {
        count[2]++;
        i++;
    }
    if (count[2] == 0)
        return 0;

    while (center_row + i < max_i && center_column + i < max_j
            && bitmatrix_get(finder->image, center_row + i, center_column + i)) {
        count[3]++;
        i++;
    }
    if (count[3] == 0)
        return 0;

    while (center_row + i < max_i && center_column + i < max_j
            && bitmatrix_get(finder->image, center_row + i, center_column + i) == 0) {
        count[4]++;
        i++;
    }
    if (count[4] == 0)
        return 0;

    return found_pattern_diagonal(count);
}

static unsigned int cross_check_horizontal(const struct qr_finder_pattern_finder *finder,
        const unsigned int center_row, const unsigned int start_column,
        const unsigned int max_count, const unsigned int origin_count_total)
{
    unsigned int j, count_total;
    unsigned int count[5];

    /* 1. 从中心向左边计数, 先记录中心模块的宽度 */
    count[2] = 0;
    for (j = start_column + 1; j-- > 0; ++count[2]) {
        if (bitmatrix_get(finder->image, center_row, j))
            break;
    }
    if (j == (unsigned int)~0)
        return (unsigned int)~0;

    for (count[1] = 1; j-- > 0; ++count[1]) {
        if (!(bitmatrix_get(finder->image, center_row, j) && count[1] <= max_count))
            break;
    }
    if (j == (unsigned int)~0 || count[1] > max_count)
        return (unsigned int)~0;

    for (count[0] = 1; j-- > 0; ++count[0]) {
        if (!(!bitmatrix_get(finder->image, center_row, j) && count[0] <= max_count))
            break;
    }
    if (count[0] > max_count)
        return (unsigned int)~0;

    /* 2. 从中心向下计数, 先记录中心模块的宽度 */
    for (j = start_column + 1; j < finder->image->column; ++j, ++count[2]) {
        if (bitmatrix_get(finder->image, center_row, j))
            break;
    }
    if (j == finder->image->column)
        return (unsigned int)~0;

    for (count[3] = 0; j < finder->image->column; ++j, ++count[3]) {
        if (!(bitmatrix_get(finder->image, center_row, j) && count[3] <= max_count))
            break;
    }
    if (j == finder->image->column || count[3] > max_count)
        return (unsigned int)~0;

    for (count[4] = 0; j < finder->image->column; ++j, ++count[4]) {
        if (!(!bitmatrix_get(finder->image, center_row, j) && count[4] <= max_count))
            break;
    }
    if (count[4] > max_count)
        return (unsigned int)~0;

    count_total = count[0] + count[1] + count[2] + count[3] + count[4];
    if (5 * unsigned_diff(count_total ,origin_count_total) >= (origin_count_total << 1))
        return (unsigned int)~0;

    return found_patttern_cross(count) ? center_from_end(count, j) : (unsigned int)~0;
}

static unsigned int cross_check_vertical(const struct qr_finder_pattern_finder *finder,
        const unsigned int start_row, const unsigned int center_column,
        const unsigned int max_count, const unsigned int origin_count_total)
{
    unsigned int i, count_total;
    unsigned int count[5];

    count[2] = 0;
    /* 1. 从中心向上计数, 先记录中心模块的宽度 */
    for (i = start_row + 1; i-- > 0; ++count[2]) {
        if (bitmatrix_get(finder->image, i, center_column))
            break;
    }
    if (i == (unsigned int)~0)
        return (unsigned int)~0;

    for (count[1] = 1; i-- > 0; ++count[1]) {
        if (!(bitmatrix_get(finder->image, i, center_column) && count[1] <= max_count))
            break;
    }
    if (i == (unsigned int)~0 || count[1] > max_count)
        return (unsigned int)~0;

    for (count[0] = 1; i-- > 0; ++count[0]) {
        if ((bitmatrix_get(finder->image, i, center_column) && count[0] <= max_count))
            break;
    }
    if (count[0] > max_count)
        return (unsigned int)~0;

    /* 2. 从中心向下计数, 先记录中心模块的宽度 */
    for (i = start_row + 1; i < finder->image->row; ++i, ++count[2]) {
        if (bitmatrix_get(finder->image, i, center_column))
            break;
    }
    if (i == finder->image->row)
        return (unsigned int)~0;

    for (count[3] = 0; i < finder->image->row; ++i, ++count[3]) {
        if (!(bitmatrix_get(finder->image, i, center_column) && count[3] <= max_count))
            break;
    }
    if (i == finder->image->row || count[3] >= max_count)
        return (unsigned int)~0;

    for (count[4] = 0; i < finder->image->row; ++i, ++count[4]) {
        if (!(!bitmatrix_get(finder->image, i, center_column) && count[4] <= max_count))
            break;
    }
    if (count[4] >= max_count)
        return (unsigned int)~0;

    count_total = count[0] + count[1] + count[2] + count[3] + count[4];
    if (5 * unsigned_diff(count_total, origin_count_total) >= (origin_count_total << 1))
        return (unsigned int)~0;

    return found_patttern_cross(count) ? center_from_end(count, i) : (unsigned int)~0;
}

static unsigned char qr_finder_pattern_about_equals(const struct qr_finder_pattern *pattern,
        const float module_size, const unsigned int center_row, const unsigned int center_column)
{
    if (unsigned_diff(pattern->column, center_column) < module_size
            && unsigned_diff(pattern->row, center_row) < module_size) {
        float module_size_diff = (float)fabs(module_size - pattern->estimate_modules_size);
        return module_size_diff <= 1.0f || module_size_diff <= module_size;
    }

    return 0;
}

static unsigned char handle_possible_center(struct qr_finder_pattern_finder *finder, const unsigned int count[5],
        const unsigned int row, const unsigned int column)
{
    unsigned int count_total = count[0] + count[1] + count[2] + count[3] + count[4];
    unsigned int center_j = center_from_end(count, column);
    unsigned int center_i = cross_check_vertical(finder, row, center_j, count[2], count_total);

    if (center_i == (unsigned int)~0)
        return 0;

    center_j = cross_check_horizontal(finder, center_i, center_j, count[2], count_total);
    if (center_j == (unsigned int)~0 || !cross_check_diagnoal(finder, center_i, center_j))
        return 0;

    unsigned char found = 0;
    float estimate_modules_size = (float)count_total / 7.0f;

    if (cross_check_vertical(finder, row, center_j - (unsigned int)estimate_modules_size, count[2], count_total) == (unsigned int)~0
        || cross_check_vertical(finder, row, center_j + (unsigned int)estimate_modules_size, count[2], count_total) == (unsigned int)~0
        || cross_check_horizontal(finder, center_i - (unsigned int)estimate_modules_size, center_j, count[2], count_total) == (unsigned int)~0
        || cross_check_horizontal(finder, center_i + (unsigned int)estimate_modules_size, center_j, count[2], count_total) == (unsigned int)~0)
        return 0;

    struct linkedlist_queue_iterator it;
    for (struct qr_finder_pattern *pattern = linkedlist_queue_iterator_init(&finder->finder_pattern, &it);
            pattern != NULL; pattern = linkedlist_queue_iterator_move(&it, 1)) {
        if (qr_finder_pattern_about_equals(pattern, estimate_modules_size, center_i, center_j)) {
            found = 1;
            /* todo: 融合坐标 */
            /* 融合模块大小 */
            ++pattern->count;
            pattern->estimate_modules_size = pattern->estimate_modules_size * pattern->count;
            pattern->estimate_modules_size /= pattern->count;
            break;
        }
    }

    if (!found) {
        struct qr_finder_pattern pattern;
        pattern.row = center_i;
        pattern.column = center_j;
        pattern.estimate_modules_size = estimate_modules_size;
        pattern.count = 1;

        if (linkedlist_queue_enque(&finder->finder_pattern, &pattern, 1) != 1)
            return 0;
    }

    return 1;
}

static unsigned int find_row_skip(struct qr_finder_pattern_finder *pattern_finder)
{
    struct linkedlist_queue_iterator it;
    struct qr_finder_pattern *first_pattern;

    if (linkedlist_queue_size(&pattern_finder->finder_pattern) <= 1)
        return 0;

    first_pattern = NULL;
    for (struct qr_finder_pattern *pattern = linkedlist_queue_iterator_init(&pattern_finder->finder_pattern, &it);
            pattern != NULL; pattern = linkedlist_queue_iterator_move(&it, 1)) {
        if (pattern->count >= 2) {
            if (first_pattern == NULL) {
                first_pattern = pattern;
            } else {
                return unsigned_diff(unsigned_diff(first_pattern->column, pattern->column), unsigned_diff(first_pattern->row, pattern->row)) >> 1;
            }
        }
    }

    return 0;
}

static unsigned char have_multiply_confirmed_centers(struct qr_finder_pattern_finder *finder)
{
    struct linkedlist_queue_iterator it;
    unsigned int confirmed_count = 0;
    float total_module_size = 0.0f;

    for (struct qr_finder_pattern *pattern = linkedlist_queue_iterator_init(&finder->finder_pattern, &it);
            pattern != NULL; pattern = linkedlist_queue_iterator_move(&it, 1)) {
        if (pattern->count >= 2) {
            ++confirmed_count;
            total_module_size += pattern->estimate_modules_size;
        }
    }

    if (confirmed_count < 3)
        return 0;

    float average = total_module_size / (float)linkedlist_queue_size(&finder->finder_pattern);
    float total_deviation = 0.0f;
    for (struct qr_finder_pattern *pattern = linkedlist_queue_iterator_init(&finder->finder_pattern, &it);
            pattern != NULL; pattern = linkedlist_queue_iterator_move(&it, 1))
        total_deviation += (float)fabs(pattern->estimate_modules_size - average);

    return total_deviation <= 0.05f * total_module_size;
}

static int pattern_compare_estimate_modules_size(const void *p1, const void *p2)
{
    const struct qr_finder_pattern *pattern[2] = {p1, p2};

    if (pattern[0]->estimate_modules_size < pattern[1]->estimate_modules_size)
        return -1;

    return (pattern[0]->estimate_modules_size > pattern[1]->estimate_modules_size) ? 1 : 0;
}

static int comare_unsigned(const void *p1, const void *p2)
{
    const unsigned int *u[2] = {p1, p2};

    if (u[0][0] > u[1][0])
        return 1;

    return u[0][0] == u[1][0] ? 0 : 1;
}

static unsigned char select_best_patterns(struct linkedlist_queue *patterns_queue)
{
    const unsigned int start_size = linkedlist_queue_size(patterns_queue);
    unsigned int distortion = (unsigned int)~0;
    struct qr_finder_pattern *best_patterns[3] = {NULL, NULL, NULL};

    if (start_size < 3) {
        linkedlist_queue_clear(patterns_queue);
        return 0;
    }

    struct qr_finder_pattern *patterns = (struct qr_finder_pattern *)mem_alloc(sizeof(struct qr_finder_pattern) * start_size);
    if (patterns == NULL) {
        linkedlist_queue_clear(patterns_queue);
        return 0;
    }

    if (linkedlist_queue_deque(patterns_queue, patterns, start_size) != start_size) {
        linkedlist_queue_clear(patterns_queue);
        mem_free(patterns);
        return 0;
    }

    linkedlist_queue_clear(patterns_queue);
    qsort(patterns, start_size, sizeof(*patterns), pattern_compare_estimate_modules_size);
    for (unsigned int r, j, i = 0; i < start_size - 2; ++i) {
        float min_module_size = patterns[i].estimate_modules_size;

        for (j = i + 1; j < start_size - 1; ++j) {
            unsigned int diff_x = unsigned_diff(patterns[i].column, patterns[j].column);
            unsigned int diff_y = unsigned_diff(patterns[i].row, patterns[j].row);
            unsigned int squares0 = diff_y * diff_y + diff_x * diff_x;

            for (r = j + 1; r < start_size; ++r) {
                unsigned int squares[3];
                float max_module_size = patterns[r].estimate_modules_size;

                if (max_module_size > min_module_size * 1.4f) {
                    /* 模块大小并不相似 */
                    continue;
                }

                squares[0] = squares0;
                diff_x = unsigned_diff(patterns[r].column, patterns[j].column);
                diff_y = unsigned_diff(patterns[r].row, patterns[j].row);
                squares[1] = diff_y * diff_y + diff_x * diff_x;
                diff_x = unsigned_diff(patterns[r].column, patterns[i].column);
                diff_y = unsigned_diff(patterns[r].row, patterns[i].row);
                squares[2] = diff_y * diff_y + diff_x * diff_x;
                qsort(squares, 3, sizeof(unsigned int), comare_unsigned);

                unsigned int d = unsigned_diff(squares[2], squares[1] << 1) + unsigned_diff(squares[2], squares[0] << 1);
                if (d < distortion) {
                    distortion = d;
                    best_patterns[0] = patterns + i;
                    best_patterns[1] = patterns + j;
                    best_patterns[2] = patterns + r;
                }
            }
        }
    }

    if (distortion == (unsigned int)~0U) {
        mem_free(patterns);
        return 0;
    }

    for (unsigned char i = 0; i < 3; ++i) {
        if (linkedlist_queue_enque(patterns_queue, best_patterns[i], 1) != 1) {
            linkedlist_queue_clear(patterns_queue);
            mem_free(patterns);
            return 0;
        }
    }

    mem_free(patterns);
    return 1;
}

static void order_best_patterns(struct qr_finder_pattern_info *info, struct linkedlist_queue *que)
{
    struct qr_finder_pattern *pattern;
    struct linkedlist_queue_iterator it;
    struct qr_finder_pattern_center center[3], *a, *b, *c;
    struct vector v[2];
    int distance[3], i;

    /* 这里确保que中的数据只有3个, 不对队列的大小再进行判断了 */
    for (i = 0, pattern = linkedlist_queue_iterator_init(que, &it);
            pattern != NULL; pattern = linkedlist_queue_iterator_move(&it, 1), ++i) {
        center[i].row = pattern->row;
        center[i].column = pattern->column;
    }

    v[0].i = (int)center[0].row - (int)center[1].row;
    v[0].j = (int)center[0].column - (int) center[1].column;
    distance[0] = vector_dot(v, v); /* 实际上是两点距离的平方 */
    v[0].i = (int)center[0].row - (int)center[2].row;
    v[0].j = (int)center[0].column - (int)center[2].column;
    distance[1] = vector_dot(v, v);
    v[0].i = (int)center[1].row - (int)center[2].row;
    v[0].j = (int)center[1].column - (int)center[2].column;
    distance[2] = vector_dot(v, v);

    if (distance[1] < distance[0] && distance[2] < distance[0]) {
        a = &center[2];
        b = &center[0];
        c = &center[1];
    } else if (distance[0] < distance[1] && distance[2] < distance[1]) {
        a = &center[1];
        b = &center[0];
        c = &center[2];
    } else {
        a = &center[0];
        b = &center[1];
        c = &center[2];
    }

    v[0].i = (int)c->row - (int)a->row;
    v[0].j = (int)c->column - (int)a->column;
    v[1].i = (int)b->row - (int)a->row;
    v[1].j = (int)b->column - (int)a->column;
    if (vector_cross_product(v, v + 1) < 0) {
        struct qr_finder_pattern_center *temp;

        temp = b;
        b = c;
        c = temp;
    }

    memcpy(&info->left_top, a, sizeof(struct qr_finder_pattern_center));
    memcpy(&info->right_top, b, sizeof(struct qr_finder_pattern_center));
    memcpy(&info->left_bottom, c, sizeof(struct qr_finder_pattern_center));
}

struct qr_finder_pattern_info *qr_finder_pattern_find(const struct bitmatrix *image_bitmatrix)
{
    struct qr_finder_pattern_info *info;
    struct qr_finder_pattern_finder *pattern_finder;
    unsigned char done, has_skipped;

    if (image_bitmatrix == NULL)
        return NULL;

    info = (struct qr_finder_pattern_info *)mem_alloc(sizeof(struct qr_finder_pattern_info));
    if (info == NULL)
        return NULL;
    memset(info, 0, sizeof(*info));

    pattern_finder = (struct qr_finder_pattern_finder *)mem_alloc(sizeof(struct qr_finder_pattern_finder));
    if (pattern_finder == NULL) {
        mem_free(info);
        return NULL;
    }
    linkedlist_queue_init(&pattern_finder->finder_pattern, sizeof(struct qr_finder_pattern));
    pattern_finder->image = image_bitmatrix;

    unsigned int row_skipped = 3 * pattern_finder->image->row / (4 * 97);
    if (row_skipped < 2)
        row_skipped = 2;

    done = 0;
    has_skipped = 0;
    for (unsigned int count[5], i, j = 0; j < pattern_finder->image->row && !done; j += row_skipped) {
        unsigned char last_gray, current_gray, current_state;

        /* 1. 先过滤白色边界 */
        for (i = 0; i < pattern_finder->image->column; ++i) {
            last_gray = bitmatrix_get(pattern_finder->image, j, i);
            if (last_gray == 0)
                break;
        }
        if (i == pattern_finder->image->column)
            continue;

        /* 2. 扫描当前行, 尝试找出1:1:3:1:1的模块 */
        last_gray = 0;
        memset(count, 0, sizeof(count));
        count[0] = 1, current_state = 0;
        while (i++ < pattern_finder->image->column) {
            if (i < pattern_finder->image->column) {
                current_gray = bitmatrix_get(pattern_finder->image, j, i);
                if (current_gray == last_gray) {
                    ++count[current_state];
                    continue;
                }

                last_gray = current_gray;
                if (current_state < 4) {
                    count[++current_state] = 1;
                    continue;
                }
            } else if (current_state != 4) {
                break;
            }

            current_state = 3;
            if (!found_patttern_cross(count)) {
                shift2counters(count);    /* 丢掉前两个的记录 */
                continue;
            }

            /* 扫描到1:1:3:1:1的模块 */
            if (handle_possible_center(pattern_finder, count, j, i)) {
                unsigned int _row_skipped;

                row_skipped = 2;
                if (has_skipped) { /* 说明找到第三个探测图形了 */
                    done = have_multiply_confirmed_centers(pattern_finder);
                } else {
                    /* 快速定位出下一个探测图形的扫描位置 */
                    _row_skipped = find_row_skip(pattern_finder);
                    if (_row_skipped > count[2]) {
                        has_skipped = 1;
                        j += _row_skipped - count[2] - row_skipped;
                        i = pattern_finder->image->column;
                    }
                }
            }
            shift2counters(count);    /* 丢掉前两个的记录 */
        }
    }

    /* 选择最佳的探测图形, 并排序 */
    if (select_best_patterns(&pattern_finder->finder_pattern)) {
        order_best_patterns(info, &pattern_finder->finder_pattern);
    } else {
        mem_free(info);
        info = NULL;
    }

    linkedlist_queue_clear(&pattern_finder->finder_pattern);
    mem_free(pattern_finder);

    return info;
}
