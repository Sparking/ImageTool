#include <limits.h>
#include <stdint.h>
#include <string.h>
#include "maths.h"

unsigned int bits_count(unsigned int i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    i = (i + (i >> 4)) & 0x0f0f0f0f;
    i = i + (i >> 8);
    i = i + (i >> 16);

    return (i & 0x3f);
}

int great_common_divisor(const int a, const int b)
{
    return (a % b == 0) ? b : great_common_divisor(b, a % b);
}

float fast_inv_sqrtf(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5f;

    x2 = number * 0.5f;
    y = number;
    i = *(long *)&y;
    i = 0x5F3759DF - (i >> 1);
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y));

    return y;
}

/* 快速开平方 */
float fast_sqrtf(const float value)
{
    if (value <= 1e-8 && value >= -1e-8) {
        return 0.0f;
    }

    return 1.0f / fast_inv_sqrtf(value);
}

unsigned int find_max_common_divisor(const unsigned int a, const unsigned int b)
{
	unsigned int new_divisor, divisor;

    for (divisor = 1, new_divisor = 2; new_divisor <= a && new_divisor <= b;
            ++new_divisor) {
        if (a % new_divisor == 0 && b % new_divisor == 0)
            divisor = new_divisor;
    }

    return divisor;
}

int vector_dot(const struct vector *vec1, const struct vector *vec2)
{
    return vec1->i * vec2->i + vec1->j * vec2->j;
}

int vector_cross_product(const struct vector *vec1, const struct vector *vec2)
{
    return vec1->i * vec2->j - vec1->j * vec2->i;
}

int vector_cross_product3(const struct vector *vec1, const struct vector *vec2,
    const struct vector *vec3)
{
    return (vec3->i - vec2->i) * (vec1->j - vec2->j)
            - (vec3->j - vec2->j) * (vec1->i - vec2->i);
}


float vector_length(const struct vector *vec)
{
    if (vec->i == 0 || vec->j == 0)
        return (float)fabs(vec->i + vec->j);

    return SQRTF((float)vector_dot(vec, vec));
}

float points_distance(const struct point *p1, const struct point *p2)
{
    struct vector delta;

    delta.i = p1->x - p2->x;
    delta.j = p1->y - p2->y;

    return vector_length(&delta);
}

int point_position_to_line(const struct point *p, const struct line *l)
{
    struct vector ldir_vec;  /* 从线的起始点指向线的终点的向量 */
    struct vector l2p_vec;   /* 从线的起始点指向点p的向量 */

    ldir_vec.i = l->end.x - l->start.x;
    ldir_vec.j = l->end.y - l->start.y;
    l2p_vec.i = p->x - l->start.x;
    l2p_vec.j = p->y - l->start.y;

    return vector_cross_product(&l2p_vec, &ldir_vec);
}

/**
 * @brief is_point_in_line_segment 判断点是在线段中, 还是在线段的延长线中
 * @param p 点
 * @param l 线
 * @return false 点在线段的延长线上
 *         true  点在线段之中(包括顶点)
 * @note 使用该函数的前提是确定点落在线段所在的直线上
 */
static bool is_point_in_line_segment(const struct point *p,
        const struct line *l)
{
    struct vector p2start_vec;
    struct vector p2end_vec;

    p2start_vec.i = p->x - l->start.x;
    p2start_vec.j = p->y - l->start.y;
    p2end_vec.i = p->x - l->end.x;
    p2end_vec.j = p->y - l->end.y;

    return vector_dot(&p2start_vec, &p2end_vec) <= 0;
}

int point_position_to_polygon(const struct point *p, const struct polygon *s)
{
    int ret;
    int last_position;
    int new_position;
    const struct line *edge;
    struct line last_edge;
	unsigned int i;

    edge = (struct line *)s->vertex;
    last_position = point_position_to_line(p, edge);
    if (last_position == 0) {
        return is_point_in_line_segment(p, edge) ? 0 : -1;
    }

    ret = 1;
    for (i = 1; i < s->n - 1; i++) {
        edge = (struct line *)(s-> vertex + i);
        new_position = point_position_to_line(p, edge);
        if (new_position == 0) {
            return is_point_in_line_segment(p, edge) ? 0 : -1;
        } else if (new_position * last_position < 0) {
            ret = -2;
        }

        last_position = new_position;
    }

    last_edge.start.x = s->vertex[s->n - 1].x;
    last_edge.start.y = s->vertex[s->n - 1].y;
    last_edge.end.x = s->vertex[0].x;
    last_edge.end.y = s->vertex[0].y;
    new_position = point_position_to_line(p, &last_edge);
    if (new_position == 0) {
        ret = is_point_in_line_segment(p, &last_edge) ? 0 : -1;
    } else if (new_position * last_position < 0) {
        ret = -2;
    }

    return ret;
}

int gaussian_elimination(float *matrix, float *res, const unsigned int rows)
{
    int ret;
    unsigned int j, i;
	float current_main_element, temp;
    unsigned int next_row, next_offset, current_offset, row_offset;
	unsigned int max_main_element_row, max_main_element_row_offset;
    const unsigned int columns = rows + 1;

    if (matrix == NULL || res == NULL || rows == 0)
        return -1;

    ret = 0;
    /* 化成三角形 */
    for (j = 0; j < rows; ++j) {
		max_main_element_row = j;
        current_offset = j * columns + j;
        max_main_element_row_offset = current_offset;
        current_main_element = (float)fabs(matrix[current_offset]);

        /* 1. 将列主元最大的行移到剩余行中的第一行 */
        /* 1.1 找出主元最大的一行 */
        for (next_offset = current_offset + columns, next_row = j + 1;
                next_row < rows; ++next_row, next_offset += columns) {
            if (fabs(matrix[next_offset]) > current_main_element) {
                max_main_element_row = next_row;
                max_main_element_row_offset = next_offset;
            }
        }
        /* 1.2 交换两行的顺序 */
        if (max_main_element_row != j) {
            for (i = 0; i + j < columns; ++i) {
                /* 前面都是0, 所以不用再交换 */
                temp = matrix[current_offset + i];
                matrix[current_offset + i] = matrix[max_main_element_row_offset + i];
                matrix[max_main_element_row_offset + i] = temp;
            }
        }

        /* 2. 消主元 */
        for (next_offset = current_offset + columns, next_row = j + 1;
                next_row < rows; ++next_row, next_offset += columns) {
            if (matrix[next_offset] == 0.0f)
                continue;

            temp = matrix[current_offset] / matrix[next_offset];
            matrix[next_offset] = 0.0f;
            for (i = 1; i + j < columns; ++i)
                matrix[next_offset + i] =
                        matrix[next_offset + i] * temp - matrix[current_offset + i];
        }

    }

    /* 回代求解 */
    for (j = rows; j-- > 0;) {
        row_offset = j * columns;
        if (matrix[row_offset + j] == 0.0f) {   /* 无唯一解 */
            ret = -1;
            break;
        }

        temp = 0.0f;
        for (i = columns - 2; i > j; --i)
            temp += res[i] * matrix[row_offset + i];
        res[j] = (matrix[row_offset + rows] - temp) / matrix[row_offset + j];
    }

    return ret;
}

/** y = a + b * x */
int least_square_method_fit_line(float *a, float *b, const struct point *p, const unsigned int n)
{
    unsigned int i;
    float sum_x, sum_y, sum[2], avg_x;

    if (a == NULL || b == NULL || p == NULL || n < 2) {
        return -1;  /* 无法拟合直线 */
    }

    for (sum_x = 0.0f, sum_y = 0.0f, sum[0] = 0.0f, sum[1] = 0.0f, i = 0; i < n; ++i) {
        sum_x += p[i].x;
        sum_y += p[i].y;
        sum[0] += p[i].x * p[i].y;
        sum[1] += p[i].x * p[i].x;
    }
    avg_x = sum_x / (float)n;
    sum[0] -= avg_x * sum_y;
    sum[1] -= avg_x * sum_x;

    if (fabs(sum[1]) < 1e-12)   /* b为无穷大 */ {
        *a = avg_x; /* 把a设置为截距 */
        return 1;
    }

    *b = sum[0] / sum[1];
    *a = (sum_y - *b * sum_x) / (float)n;

    return 0;
}

int line_cross_point(struct point *p, const float a1, const float b1,
        const float a2, const float b2)
{
    float matrix[2][3], res[2];

    if (p == NULL)
        return -1;  /* 无法求出交点 */

    matrix[0][0] = -b1;
    matrix[0][1] = 1.0f;
    matrix[0][2] = a1;
    matrix[1][0] = -b2;
    matrix[1][1] = 1.0f;
    matrix[1][2] = a2;
    if (gaussian_elimination(matrix[0], res, 2) == -1)
        return -1;

    p->x = (int)(res[0] + 0.5f);
    p->y = (int)(res[1] + 0.5f);

    return 0;
}

int vector_tan_pow2(const struct vector *vec1, const struct vector *vec2,
        const unsigned int e)
{
    const int dot_product = vector_dot(vec1, vec2);
    int cross_product;

    if (dot_product == 0)
        return INT_MAX;

    cross_product = vector_cross_product(vec1, vec2);

    return (cross_product << e) / dot_product;
}

bool points_in_line(const struct point *a, const struct point *b,
        const struct point *c)
{
	int res;
    struct vector vec[2];

    vec[0].i = b->x - a->x;
    vec[0].j = b->y - a->y;
    vec[1].i = c->x - b->x;
    vec[1].j = c->y - b->y;
    res = vector_tan_pow2(vec, vec + 1, 8);

    return ((int)fabs(res) <= 55);
}

bool line4p_is_parell(const struct point *ap1, const struct point *ap2,
        const struct point *bp1, const struct point *bp2)
{
    struct vector vec[2];

    vec[0].i = ap2->x - ap1->x;
    vec[0].j = ap2->y - ap1->y;
    vec[1].i = bp2->x - bp1->x;
    vec[1].j = bp2->y - bp1->y;

    return (fabs(vector_tan_pow2(vec, vec + 1, 2)) >= 1);
}

bool get_line_dirpos(const struct point *start, const struct point *end, const struct point *base,
	    const int len, struct point *pos)
{
	float inv_sqr;
	struct point d;

	d.x = end->x - start->x;
	d.y = end->y - start->y;
	if (d.x == 0 && d.y == 0)
		return false;

	inv_sqr = fast_inv_sqrtf((float)(d.x * d.x + d.y * d.y));
	pos->x = (int)(base->x + d.x * len * inv_sqr + 0.5f);
	pos->y = (int)(base->y + d.y * len * inv_sqr + 0.5f);

	return true;
}

bool get_linepos_veroffset(const struct point *start, const struct point *final,
        const struct point *appoint, const int noffset, struct point *value)
{
	int dx, dy;
	float max, v;

	dx = start->y - final->y;
	dy = start->x - final->x;
	if (dx == 0 && dy == 0) {
		value->x = appoint->x;
		value->y = appoint->y;
		return true;
	}

	max = fast_inv_sqrtf((float)(dx * dx + dy * dy));
	v = (float)(appoint->x - noffset * dx * max);
	value->x = (int)(v + 0.5f);
	v = (float)(appoint->y + noffset * dy * max);
	value->y = (int)(v + 0.5f);

	return true;
}

void get_pos_in_pt2pt(const struct point *start, const struct point *end,
	struct point *pt, const unsigned int pos)
{
	int step;
	struct point delta;
	struct point delta_abs;

	step = (int)pos;
	delta.x = end->x - start->x;
	delta.y = end->y - start->y;
	delta_abs.x = (int)fabs(delta.x);
	delta_abs.y = (int)fabs(delta.y);
	if (delta_abs.x >= delta_abs.y) {
		if (delta.x < 0)
			step = -step;

		pt->x = start->x + step;
		pt->y = (((step * delta.y << 8) / delta.x + 128) >> 8) + start->y;
	} else {
		if (delta.y < 0)
			step = -step;

		pt->y = step + start->y;
		pt->x = (((step * delta.x << 8) / delta.y + 128) >> 8) + start->x;
	}
}
