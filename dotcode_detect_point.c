#include <string.h>
#include "compilers.h"
#include "rbtree.h"
#include "dotcode_detect_point.h"

struct rb_dotcode_point {
    struct dotcode_point pt;
    struct rb_node node;
};

static UNUSED struct rb_dotcode_point *dotcode_rb_point_insert(
        struct rb_root *root, struct dotcode_point *pt)
{
    int cmp_ret;
    struct rb_node **new, *parent;
    struct rb_dotcode_point *rb_pt;

    if (root == NULL || pt == NULL)
        return NULL;

    parent = NULL;
    new = &root->rb_node;
    while (*new) {
        parent = *new;
        rb_pt = rb_entry(*new, struct rb_dotcode_point, node);
        cmp_ret = memcmp(&pt->center, &rb_pt->pt.center, sizeof(pt->center));
        if (cmp_ret > 0) {
            new = &(*new)->rb_right;
        } else if (cmp_ret < 0) {
            new = &(*new)->rb_left;
        } else {
            ++rb_pt->pt.weight;
            return rb_pt;
        }
    }

    rb_pt = (struct rb_dotcode_point *)mem_alloc(sizeof(*rb_pt));
    if (rb_pt == NULL)
        return NULL;

    memcpy(&rb_pt->pt, pt, sizeof(*pt));
    rb_pt->pt.weight = 0;
    rb_link_node(&rb_pt->node, parent, new);
    rb_insert_color(&rb_pt->node, root);

    return rb_pt;
}

static UNUSED void dotcode_rb_point_clean(struct rb_root *root)
{
    struct rb_node *node;

    if (root == NULL || root->rb_node == NULL)
        return;

    while ((node = rb_last(root)) != NULL) {
        rb_erase(node, root);
        free(rb_entry(node, struct rb_dotcode_point, node));
    }
}

static UNUSED struct rb_dotcode_point *dotcode_rb_point_find(
        struct rb_root *root, int x, int y)
{
    int cmp_ret;
    struct rb_node *node;
    struct rb_dotcode_point *rb_pt;
    const struct point pt = {x, y};

    node = root->rb_node;
    rb_pt = NULL;
    while (node) {
        rb_pt = rb_entry(node, struct rb_dotcode_point, node);
        cmp_ret = memcmp(&pt, &rb_pt->pt.center, sizeof(pt));
        if (cmp_ret == 0)
            break;

        if (cmp_ret > 0) {
            node = node->rb_right;
        } else {
            node = node->rb_left;
        }
    }

    return rb_pt;
}

#define IMAGE_RFEDGE_AMP_LIMIT_MIN      15

unsigned int image_find_raise_fall_edges_by_offset_dotcode(
        const struct image *img, const struct point *pstart,
        const struct point *setup_off, const unsigned int len,
        struct image_raise_fall_edge *pedge, const unsigned int num)
{
    int imgdata_off;
    struct point off_end;
    unsigned int cnt, i;
    unsigned char grad;
    unsigned char gray;
    unsigned char cur_type;
    unsigned char last_type;
    unsigned char max_grad;
    unsigned char max_amplitude;
    const unsigned char *imgdata;
    struct image_raise_fall_edge *cur_edge;
    struct image_raise_fall_edge *max_edge;
    struct image_raise_fall_edge *last_edge;
    struct image_raise_fall_edge *buff;
    struct image_raise_fall_edge *buff_end;
    struct image_raise_fall_edge *buff_prev;

    if (img == NULL || pstart == NULL || setup_off == NULL || len <= 1 || pedge == NULL || num == 0)
        return 0;

    if (pstart->x < 0 || pstart->x >= (int)img->width || pstart->y < 0 || pstart->y >= (int)img->height)
        return 0;

    cnt = 0;
    max_grad = 0;
    max_edge = NULL;
    max_amplitude = 0;
    cur_edge = pedge - 1;
    off_end.x = pstart->x + setup_off->x;
    off_end.y = pstart->y + setup_off->y;
    imgdata_off = setup_off->y * img->width + setup_off->x;
    imgdata = img->data + pstart->y * img->width + pstart->x;
    gray = *imgdata;
    imgdata += imgdata_off;
    last_type = IMAGE_RFEDGE_TYPE_NONE;
    buff_end = pedge + num - 1;
    for (i = 1; i < len && off_end.x >= 0 && off_end.x < (int)img->width
                && off_end.y >= 0 && off_end.y < (int)img->height;
            ++i, imgdata += imgdata_off, off_end.x += setup_off->x, off_end.y += setup_off->y) {
        if (gray == *imgdata) {
            last_type = IMAGE_RFEDGE_TYPE_NONE;
            continue;
        } else if (gray < *imgdata) {
            grad = *imgdata - gray;
            cur_type = IMAGE_RFEDGE_TYPE_RAISE;
        } else {
            grad = gray - *imgdata;
            cur_type = IMAGE_RFEDGE_TYPE_FALL;
        }

        if (grad > max_grad)
            max_grad = grad;

        if (last_type != cur_type) {
            if (max_edge == NULL) {
                max_edge = pedge;
            } else if (max_edge->max_grad <= cur_edge->max_grad
                    && max_edge->amplitude <= cur_edge->amplitude) {
                max_edge = cur_edge;
            }

            if (cur_edge >= buff_end)
                break;

            ++cur_edge;
            cur_edge->begin = i - 1;
            cur_edge->end = i;
            cur_edge->type = cur_type;
            cur_edge->max_grad = grad;
            cur_edge->min_grad = grad;
            cur_edge->amplitude = grad;
            last_type = cur_type;
            cur_edge->max_gray = gray;
            cur_edge->min_gray = *imgdata;
            cur_edge->dpos_256x = grad * i;
        } else {
            cur_edge->end = i;
            if (grad > cur_edge->max_grad) {
                cur_edge->max_grad = grad;
            } else if (grad < cur_edge->min_grad) {
                cur_edge->min_grad = grad;
            }
            cur_edge->min_gray = *imgdata;
            cur_edge->amplitude += grad;
            cur_edge->dpos_256x += grad * i;
            if (cur_edge->amplitude > max_amplitude)
                max_amplitude = cur_edge->amplitude;
        }
        gray = *imgdata;
    }

    if (cur_edge < pedge)
        return 0;

    if (max_edge->amplitude <= IMAGE_RFEDGE_AMP_LIMIT_MIN)
        return 0;

    cnt = (unsigned int)(cur_edge - pedge + 1);
    buff = (struct image_raise_fall_edge *)mem_alloc(sizeof(struct image_raise_fall_edge) * cnt);
    if (buff == NULL)
        return 0;

    buff_end = pedge + cnt;
    buff_prev = buff + (max_edge - pedge);
    cur_edge = buff_prev;
    last_edge = max_edge;
    gray = (max_edge->amplitude + 2) >> 2;
    grad = (max_edge->max_grad + 1) >> 1;
    if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
        gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
    while (++last_edge < buff_end) {
        if (last_edge->amplitude < 5 && last_edge->end - last_edge->begin < 3)
            continue;

        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            ++cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude) >> 2;
            grad = (cur_edge->max_grad  + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
            if (gray > cur_edge->amplitude >> 1)
                gray = cur_edge->amplitude >> 1;
        }
    }
    buff_end = cur_edge;
    cur_edge = buff_prev;
    last_edge = max_edge;
    gray = (max_edge->amplitude + 2) >> 2;
    grad = (max_edge->max_grad + 1) >> 1;
    if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
        gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
    while (--last_edge >= pedge) {
        if (last_edge->amplitude < 5 && last_edge->end - last_edge->begin < 3)
            continue;

        if (last_edge->type == cur_edge->type) {
            if (last_edge->amplitude >= cur_edge->amplitude) {
                if (last_edge->max_grad >= cur_edge->max_grad) {
                    memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
                }
            }
        } else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
            --cur_edge;
            memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
            gray = (cur_edge->amplitude) >> 2;
            grad = (cur_edge->max_grad + 1) >> 1;
            if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
                gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
            if (gray > cur_edge->amplitude >> 1)
                gray = cur_edge->amplitude >> 1;
        }
    }
    cnt = (int)(buff_end - cur_edge + 1);
    memcpy(pedge, cur_edge, sizeof(struct image_raise_fall_edge) * cnt);
    mem_free(buff);

    cur_edge = pedge;
    last_edge = pedge + cnt;
    while (cur_edge < last_edge) {
        if (cur_edge->type == IMAGE_RFEDGE_TYPE_RAISE) {
            gray = cur_edge->max_gray;
            cur_edge->max_gray = cur_edge->min_gray;
            cur_edge->min_gray = gray;
        }
        cur_edge->dpos_256x = (cur_edge->dpos_256x << 8) / cur_edge->amplitude;
        cur_edge->dpos = (cur_edge->dpos_256x + (1 << 7)) >> 8;
        ++cur_edge;
    }

    return cnt;
}
#if 0
unsigned int image_find_raise_fall_edges_pt2pt_dotcode(const struct image *img,
	    const struct point *start, const struct point *end,
		struct image_raise_fall_edge *pedge, const unsigned int num)
{
	int i, j, step;
	struct point delta;
	struct point setup;
	struct point coordinate[3];
	unsigned int cnt;
	unsigned int pos;
	unsigned char grad;
	unsigned char gray;
	unsigned char max_grad;
	unsigned char cur_type;
	unsigned char last_type;
	unsigned char max_amplitude;
	const unsigned char *imgdata;
	struct image_raise_fall_edge *cur_edge;
	struct image_raise_fall_edge *max_edge;
	struct image_raise_fall_edge *last_edge;
	struct image_raise_fall_edge *buff;
	struct image_raise_fall_edge *buff_end;
	struct image_raise_fall_edge *buff_prev;

	if (img == NULL || start == NULL || end == NULL || pedge == NULL || num == 0)
		return 0;

	if (start->x < 0 || start->x >= (int)img->width || start->y < 0 || start->y >= (int)img->height)
		return 0;

	if (end->x < 0 || end->x >= (int)img->width || end->y < 0 || end->y >= (int)img->height)
		return 0;

	if (start->x == end->x && start->y == end->y)
		return 0;

	memcpy(&coordinate[1], start, sizeof(coordinate[1]));
	memcpy(&coordinate[2], end, sizeof(coordinate[2]));
	if (coordinate[1].y < 0) {
		coordinate[1].y = 0;
	} else if (coordinate[1].y >= img->height) {
		coordinate[1].y = img->height - 1;
	}

	if (coordinate[2].y < 0) {
		coordinate[2].y = 0;
	} else if (coordinate[2].y >= img->height) {
		coordinate[2].y = img->height - 1;
	}

	if (coordinate[1].x < 0) {
		coordinate[1].x = 0;
	} else if (coordinate[1].x >= img->height) {
		coordinate[1].x = img->width - 1;
	}

	if (coordinate[2].x < 0) {
		coordinate[2].x = 0;
	} else if (coordinate[2].x >= img->height) {
		coordinate[2].x = img->width - 1;
	}
	memcpy(&coordinate[0], &coordinate[1], sizeof(coordinate[0]));

	delta.x = end->x - start->x;
	delta.y = start->y - end->y;
	setup.x = 1;
	setup.y = 1;
	if (delta.x < 0) {
		delta.x = -delta.x;
		setup.x = -1;
	}

	if (delta.y > 0) {
		delta.y = -delta.y;
		setup.y = -1;
	}

	cnt = 0;
	max_grad = 0;
	max_edge = NULL;
	max_amplitude = 0;
	cur_edge = pedge - 1;
	buff_end = pedge + num - 1;
	last_type = IMAGE_RFEDGE_TYPE_NONE;
	imgdata = img->data + start->y * img->width + start->x;
	gray = *imgdata;
	if (-delta.y < delta.x) {
		for (pos = 1; ;  ++pos) {
			j = (int)((i * delta.y * 1.0 / delta.x + 0.5) + start->y);
			imgdata = &img->data[img->width * j + start->x + i];
			ushow_pt(1, start->x + i, j, YELLOWCOLOR);
			if (gray == *imgdata) {
				last_type = IMAGE_RFEDGE_TYPE_NONE;
				continue;
			} else if (gray < *imgdata) {
				grad = *imgdata - gray;
				cur_type = IMAGE_RFEDGE_TYPE_RAISE;
			} else {
				grad = gray - *imgdata;
				cur_type = IMAGE_RFEDGE_TYPE_FALL;
			}

			if (grad > max_grad)
				max_grad = grad;

			if (last_type != cur_type) {
				if (max_edge == NULL) {
					max_edge = pedge;
				} else if (max_edge->max_grad <= cur_edge->max_grad
					&& max_edge->amplitude <= cur_edge->amplitude) {
					max_edge = cur_edge;
				}

				if (cur_edge >= buff_end)
					break;

				++cur_edge;
				cur_edge->begin = pos - 1;
				cur_edge->end = pos;
				cur_edge->type = cur_type;
				cur_edge->max_grad = grad;
				cur_edge->min_grad = grad;
				cur_edge->amplitude = grad;
				last_type = cur_type;
				cur_edge->max_gray = gray;
				cur_edge->min_gray = *imgdata;
				cur_edge->dpos_256x = grad * pos;
			} else {
				cur_edge->end = pos;
				if (grad > cur_edge->max_grad) {
					cur_edge->max_grad = grad;
				} else if (grad < cur_edge->min_grad) {
					cur_edge->min_grad = grad;
				}
				cur_edge->min_gray = *imgdata;
				cur_edge->amplitude += grad;
				cur_edge->dpos_256x += grad * pos;
				if (cur_edge->amplitude > max_amplitude)
					max_amplitude = cur_edge->amplitude;
			}
			gray = *imgdata;
		}
	} else {
		step = 1;
		if (delta.y < 0)
			step = -1;

		for (pos = 1, j = 1; pos <= (unsigned int)delta_abs.y; j += step, ++pos) {
			i = (int)((j * delta.x * 1.0 / delta.y + 0.5) + start->x);
			imgdata = &img->data[img->width * (j + start->y) + i];
			ushow_pt(1, i, j + start->y, YELLOWCOLOR);

			if (gray == *imgdata) {
				last_type = IMAGE_RFEDGE_TYPE_NONE;
				continue;
			} else if (gray < *imgdata) {
				grad = *imgdata - gray;
				cur_type = IMAGE_RFEDGE_TYPE_RAISE;
			} else {
				grad = gray - *imgdata;
				cur_type = IMAGE_RFEDGE_TYPE_FALL;
			}

			if (grad > max_grad)
				max_grad = grad;

			if (last_type != cur_type) {
				if (max_edge == NULL) {
					max_edge = pedge;
				} else if (max_edge->max_grad <= cur_edge->max_grad
					&& max_edge->amplitude <= cur_edge->amplitude) {
					max_edge = cur_edge;
				}

				if (cur_edge >= buff_end)
					break;

				++cur_edge;
				cur_edge->begin = pos - 1;
				cur_edge->end = pos;
				cur_edge->type = cur_type;
				cur_edge->max_grad = grad;
				cur_edge->min_grad = grad;
				cur_edge->amplitude = grad;
				last_type = cur_type;
				cur_edge->max_gray = gray;
				cur_edge->min_gray = *imgdata;
				cur_edge->dpos_256x = grad * pos;
			} else {
				cur_edge->end = pos;
				if (grad > cur_edge->max_grad) {
					cur_edge->max_grad = grad;
				} else if (grad < cur_edge->min_grad) {
					cur_edge->min_grad = grad;
				}
				cur_edge->min_gray = *imgdata;
				cur_edge->amplitude += grad;
				cur_edge->dpos_256x += grad * pos;
				if (cur_edge->amplitude > max_amplitude)
					max_amplitude = cur_edge->amplitude;
			}
			gray = *imgdata;
		}
	}

	if (cur_edge < pedge)
		return 0;

	if (max_edge->amplitude <= 8)
		return 0;

	cnt = (unsigned int)(cur_edge - pedge + 1);
	buff = (struct image_raise_fall_edge *)mem_alloc(sizeof(struct image_raise_fall_edge) * cnt);
	if (buff == NULL)
		return 0;

	buff_end = pedge + cnt;
	buff_prev = buff + (max_edge - pedge);
	cur_edge = buff_prev;
	last_edge = max_edge;
	gray = (max_edge->amplitude + 2) >> 2;
	grad = (max_edge->max_grad + 1) >> 1;
	if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
		gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
	memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
	while (++last_edge < buff_end) {
		if (last_edge->amplitude < 5 && last_edge->end - last_edge->begin < 3)
			continue;

		if (last_edge->type == cur_edge->type) {
			if (last_edge->amplitude >= cur_edge->amplitude) {
				if (last_edge->max_grad >= cur_edge->max_grad) {
					memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
				}
			}
		} else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
			++cur_edge;
			memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
			gray = (cur_edge->amplitude) >> 2;
			grad = (cur_edge->max_grad + 1) >> 1;
			if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
				gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
			if (gray > cur_edge->amplitude >> 1)
				gray = cur_edge->amplitude >> 1;
		}
	}
	buff_end = cur_edge;
	cur_edge = buff_prev;
	last_edge = max_edge;
	gray = (max_edge->amplitude + 2) >> 2;
	grad = (max_edge->max_grad + 1) >> 1;
	if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
		gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
	while (--last_edge >= pedge) {
		if (last_edge->amplitude < 5 && last_edge->end - last_edge->begin < 3)
			continue;

		if (last_edge->type == cur_edge->type) {
			if (last_edge->amplitude >= cur_edge->amplitude) {
				if (last_edge->max_grad >= cur_edge->max_grad) {
					memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
				}
			}
		} else if (last_edge->max_grad >= grad || last_edge->amplitude >= gray) {
			--cur_edge;
			memcpy(cur_edge, last_edge, sizeof(struct image_raise_fall_edge));
			gray = (cur_edge->amplitude) >> 2;
			grad = (cur_edge->max_grad + 1) >> 1;
			if (gray < IMAGE_RFEDGE_AMP_LIMIT_MIN)
				gray = IMAGE_RFEDGE_AMP_LIMIT_MIN;
			if (gray > cur_edge->amplitude >> 1)
				gray = cur_edge->amplitude >> 1;
		}
	}
	cnt = (unsigned int)(buff_end - cur_edge + 1);
	memcpy(pedge, cur_edge, sizeof(struct image_raise_fall_edge) * cnt);
	mem_free(buff);

	cur_edge = pedge;
	last_edge = pedge + cnt;
	while (cur_edge < last_edge) {
		if (cur_edge->type == IMAGE_RFEDGE_TYPE_RAISE) {
			gray = cur_edge->max_gray;
			cur_edge->max_gray = cur_edge->min_gray;
			cur_edge->min_gray = gray;
		}
		cur_edge->dpos_256x = (cur_edge->dpos_256x << 8) / cur_edge->amplitude;
		cur_edge->dpos = (cur_edge->dpos_256x + (1 << 7)) >> 8;
		++cur_edge;
	}

	return cnt;
}
#endif

static bool dotcode_detect_point_get_hori_width(const struct image *img,
        const struct point *pos, const int edge_type, struct point *newpos, unsigned int *width)
{
    unsigned int cnt;
    struct point edge_off;
    struct image_raise_fall_edge rfe_buff[20], rfe_tmp;

    edge_off.y = 0;
    edge_off.x = -1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;
    rfe_tmp = rfe_buff[0];

    edge_off.x = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    newpos->x = (((pos->x << 8) + rfe_buff[0].dpos_256x - ((*width + 1) >> 1) + 128) >> 8);
    newpos->y = pos->y;
    *width = (*width + 128) >> 8;

    return true;
}

static bool dotcode_detect_point_get_vertical_width(const struct image *img,
        const struct point *pos, const int edge_type, struct point *newpos, unsigned int *width)
{
    unsigned int cnt;
    struct point edge_off;
    struct image_raise_fall_edge rfe_buff[20], rfe_tmp;

    edge_off.x = 0;
    edge_off.y = -1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;
    rfe_tmp = rfe_buff[0];

    edge_off.y = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    newpos->x = pos->x;
    newpos->y = (((pos->y << 8) + rfe_buff[0].dpos_256x - ((*width + 1) >> 1) + 128) >> 8);
    *width = (*width + 128) >> 8;

    return true;
}

static bool dotcode_detect_point_get_45_width(const struct image *img,
        const struct point *pos, const int edge_type, unsigned int *width)
{
    unsigned int cnt;
    struct point edge_off;
    struct image_raise_fall_edge rfe_buff[20], rfe_tmp;

    edge_off.x = -1;
    edge_off.y = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;
    rfe_tmp = rfe_buff[0];

    edge_off.x = 1;
    edge_off.y = -1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 20);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    *width = (*width + 128) >> 8;

    return true;
}

static bool dotcode_detect_point_get_135_width(const struct image *img,
        const struct point *pos, const int edge_type, unsigned int *width)
{
    unsigned int cnt;
    struct point edge_off;
    struct image_raise_fall_edge rfe_buff[20], rfe_tmp;

    edge_off.x = 1;
    edge_off.y = 1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 10);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;
    rfe_tmp = rfe_buff[0];

    edge_off.x = -1;
    edge_off.y = -1;
    cnt = image_find_raise_fall_edges_by_offset_dotcode(img, pos,
            &edge_off, img->width, rfe_buff, 10);
    if (cnt == 0 || rfe_buff[0].type != edge_type)
        return false;

    *width = rfe_tmp.dpos_256x + rfe_buff[0].dpos_256x;
    *width = (*width + 128) >> 8;

    return true;
}

bool dotcode_judge_width(const unsigned int a, const unsigned int b)
{
	unsigned int diff;

	if ((a <= 3 && b < 6) || (b <= 3 && a < 6)) {
		return true;
	}

	diff = unsigned_diff(a, b) << 2;
	return (a * 3 > diff && b * 3 > diff) && (a * 3 <= 5 * b && b * 3 <= 5 * a);
}

bool dotcode_checkdot_with_ref(const struct image *img, const int ref_rfet,
        struct dotcode_point *pt, struct point *coordinate, const struct dotcode_point *ref)
{
	if (img == NULL || pt == NULL || coordinate == NULL || ref == NULL) {
		return false;
	}

	pt->score = 0;
	if (dotcode_detect_point_get_hori_width(img, coordinate, ref_rfet, &pt->center, &pt->nw)) {
		if (dotcode_judge_width(pt->nw, ref->nw)) {
			memcpy(coordinate, &pt->center, sizeof(*coordinate));
			++pt->score;
		}
	}

	if (dotcode_detect_point_get_vertical_width(img, coordinate, ref_rfet, &pt->center, &pt->nh)) {
		if (dotcode_judge_width(pt->nh, ref->nh)) {
			++pt->score;
		}
	}

	if (dotcode_detect_point_get_45_width(img, &pt->center, ref_rfet, &pt->n45)) {
		if (dotcode_judge_width(pt->n45, ref->n45)) {
			++pt->score;
		}
	}

	if (dotcode_detect_point_get_135_width(img, &pt->center, ref_rfet, &pt->n135)) {
		if (dotcode_judge_width(pt->n135, ref->n135)) {
			++pt->score;
		}
	}
	pt->isblack = (ref_rfet == IMAGE_RFEDGE_TYPE_RAISE);

	return pt->score >= 4;
}

bool dotcode_checkdot(const struct image *img, const int ref_rfet,
        struct dotcode_point *pt, struct point *coordinate)
{
	if (img == NULL || pt == NULL || coordinate == NULL) {
		return false;
	}

	if (!dotcode_detect_point_get_vertical_width(img, coordinate, ref_rfet, &pt->center, &pt->nh)) {
		return false;
	}

    if (coordinate->y != pt->center.y) {
        coordinate->y = pt->center.y;
		if (!dotcode_detect_point_get_hori_width(img, coordinate, ref_rfet, &pt->center, &pt->nw)) {
			return false;
		}
    }

	if (!dotcode_judge_width(pt->nh, pt->nw)) {
		return false;
	}

	if (!dotcode_detect_point_get_45_width(img, &pt->center, ref_rfet, &pt->n45)) {
		return false;
	}

	if (!dotcode_judge_width(pt->nw, pt->n45)) {
		return false;
	}

	if (!dotcode_detect_point_get_135_width(img, &pt->center, ref_rfet, &pt->n135)) {
		return false;
	}

	if (!dotcode_judge_width(pt->n45, pt->n135)) {
		return false;
	}

	if (pt->nw > pt->nh) {
		pt->nh = pt->nw;
	} else {
		pt->nw = pt->nh;
	}
    pt->isblack = (ref_rfet == IMAGE_RFEDGE_TYPE_RAISE);

    return true;
}

void dotcode_get_edge_pos_in_pt2pt(const struct point *start, const struct point *end,
	struct point *pt, const unsigned int pos)
{
	int step;
	struct point delta;
	struct point delta_abs;

	step = (int)pos;
	delta.x = end->x - start->x;
	delta.y = end->y - start->y;
	delta_abs.x = fabs(delta.x);
	delta_abs.y = fabs(delta.y);
	if (delta_abs.x >= delta_abs.y) {
		if (delta.x < 0)
			step = -step;

		pt->x = start->x + step;
		pt->y = (step * delta.y * 1.0 / delta.x + 0.5) + start->y;
	}
	else {
		if (delta.y < 0)
			step = -step;

		pt->y = step + start->y;
		pt->x = (step * delta.x * 1.0 / delta.y + 0.5) + start->x;
	}
}

bool dotcode_gooddot_search(const struct image *img, const struct dotcode_point *pdtp, struct dotcode_point_lineset *lines)
{
	int scan_off;
	UNUSED int edge_type;
	unsigned int nedge;
	UNUSED unsigned int iedge;
	UNUSED struct dotcode_point curpt;
	UNUSED struct dotcode_point lastpt;
	struct dotcode_point_line dtline;
	struct point pt, last_secpt;
	struct point scan_range[2];
	struct point scan_endpos;
	UNUSED struct point pos_eosl;
	struct image_raise_fall_edge edges[50];

	const int dbg_ushow_scan_flag = 0;
	const int color[2][3] = { {REDCOLOR, YELLOWCOLOR, CYANCOLOR},{GREENCOLOR, PINKCOLOR, BLUECOLOR} };
	int ci;

	if (img == NULL || pdtp == NULL || lines == NULL)
		return 0;

	ci = !!pdtp->isblack;

	lines->nline = 0;
	scan_off = (pdtp->nw + 1) >> 1;
	memset(&last_secpt, 0, sizeof(last_secpt));
	memcpy(&scan_range[0], &pdtp->center, sizeof(struct point));
	memcpy(&scan_range[1], &pdtp->center, sizeof(struct point));
	scan_range[1].x += pdtp->nw * (-3);
	scan_range[1].y += pdtp->nh * (5);
	scan_endpos.x = pdtp->center.x + pdtp->nw * 5;
	scan_endpos.y = pdtp->center.y + pdtp->nh * (-3) - scan_off;
	ushow_ptWidth(dbg_ushow_scan_flag, scan_range[0].x, scan_range[0].y, color[ci][0], 3);
	ushow_ptWidth(dbg_ushow_scan_flag, scan_range[1].x, scan_range[1].y, color[ci][1], 1);
	do {
		if (fabs(scan_range[0].x - scan_endpos.x) >= fabs(scan_range[0].y - scan_endpos.y)) {
			scan_endpos.y += scan_off;
		} else {
			scan_endpos.x -= scan_off;
		}

		if (scan_endpos.x < 0 || scan_endpos.x >= img->width || scan_endpos.y < 0 || scan_endpos.y >img->height)
			break;
		ushow_ptWidth(dbg_ushow_scan_flag, scan_endpos.x, scan_endpos.y, color[0][2], 2);

		memcpy(&dtline.pt[0], pdtp, sizeof(*pdtp));
		nedge = image_find_raise_fall_edges_pt2pt(img, &scan_range[0], &scan_endpos, edges, 50);
		if (nedge < 3)
			continue;

		dotcode_get_edge_pos_in_pt2pt(&scan_range[0], &scan_endpos, &pt,
			(edges[2].dpos_256x + edges[1].dpos_256x + 256) >> 9);
		//ushow_ptWidth(dbg_ushow_scan_flag, pt.x, pt.y, YELLOWCOLOR, 1);
		if (dotcode_checkdot_with_ref(img, edges[0].type, &curpt, &pt, pdtp)) {
			ushow_ptWidth(1, curpt.center.x, curpt.center.y, YELLOWCOLOR, 1);
		}
   	} while (scan_endpos.x >= scan_range[1].x);

	return false;
}

unsigned int dotcode_detect_point(const struct image *img,
        struct dotcode_point *pdtp, const unsigned int ndtp)
{
    unsigned int i, j, off;
    unsigned int rfe_cnt;
    unsigned int dtp_size;
    struct point edge_start;
    struct point hori_edge_off;
    struct point hori_edge_start;
    struct dotcode_point pt;
    struct rb_root dtp_root;
    struct rb_node *dtp_node;
    struct rb_dotcode_point *rb_pt;
    struct image_raise_fall_edge rfe_hori[500];
	struct dotcode_point_lineset lineset;

    if (img == NULL || pdtp == NULL || ndtp == 0)
        return 0;

    dtp_size = 0;
    dtp_root = RB_ROOT;
    hori_edge_off.x = 1;
    hori_edge_off.y = 0;
    hori_edge_start.x = 0;
	off = 2;
    for (j = 0; j < img->height; j += off) {
        hori_edge_start.y = j;
        rfe_cnt = image_find_raise_fall_edges_by_offset_dotcode(img,
                &hori_edge_start, &hori_edge_off, img->width, rfe_hori, 500);
        if (rfe_cnt < 2)
            continue;

		off = 2;
        for (i = 1; i < rfe_cnt; ++i) {
            pt.nw = rfe_hori[i].dpos_256x - rfe_hori[i - 1].dpos_256x;
            edge_start.x = (rfe_hori[i - 1].dpos_256x + ((pt.nw + 1) >> 1) + 128) >> 8;
            pt.nw = (pt.nw + 128) >> 8;
            edge_start.y = j;
			if (!dotcode_checkdot(img, rfe_hori[i].type, &pt, &edge_start))
				continue;

			if (pt.nw <= 6)
				continue;

			pt.score = 4;
			if (dotcode_gooddot_search(img, &pt, &lineset))
				continue;

			if (off < (pt.nh >> 1))
				off = (pt.nh >> 1);

            rb_pt = dotcode_rb_point_insert(&dtp_root, &pt);
            if (rb_pt == NULL)
                continue;
        }

		if (i != rfe_cnt)
			break;
    }

    dtp_size = 0;
    for (dtp_node = rb_first(&dtp_root); dtp_node != NULL; dtp_node = rb_next(dtp_node)) {
        rb_pt = rb_entry(dtp_node, struct rb_dotcode_point, node);
        if (dtp_size < ndtp) {
            memcpy(pdtp + dtp_size, &rb_pt->pt, sizeof(*pdtp));
            ++dtp_size;
        }
    }
    dotcode_rb_point_clean(&dtp_root);

    return dtp_size;
}
