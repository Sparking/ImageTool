#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <image.h>
#include <maths.h>
#include <port_memory.h>
#include "dotcode_detect_point.h"

int get_edgepos_16x(const int *grads, const int len, const int srchpos)
{
	int pos16x, i, j;

	if (grads[srchpos] == 0)
		return 0;

	pos16x = 0;
	if (grads[srchpos] > 0) {
		for (i = srchpos - 1; i >= 0; --i) {
			if (grads[i] <= 0)
				break;
		}

		for (j = 0, pos16x = 0; ++i < len;) {
			if (grads[i] <= 0)
				break;

			pos16x += grads[i] * i;
			j += grads[i];
		}
	} else {
		for (i = srchpos - 1; i >= 0; --i) {
			if (grads[i] >= 0)
				break;
		}

		for (j = 0, pos16x = 0; ++i < len;) {
			if (grads[i] >= 0)
				break;

			pos16x += grads[i] * i;
			j += grads[i];
		}
	}
	pos16x = (pos16x << 4) / j;

	return pos16x;
}

bool get_dots_edge(unsigned char *data, const int len, const int center, int *center_offset, int *w16x, bool isblack)
{
	int head, tail;
	int grads[100], diff, last_diff;
	int i, j, npos, tmp;
	unsigned char maxgradpos[50];
	unsigned char gradabs[100], maxgrad;

	for (maxgrad = 0, i = 1, j = 0; i < len; ++i, ++j) {
		grads[j] = (int)data[i] - (int)data[j];
		if (data[j] >= data[i]) {
			gradabs[j] = data[j] - data[i];
		} else {
			gradabs[j] = data[i] - data[j];
		}
		if (maxgrad < gradabs[j])
			maxgrad = gradabs[j];
	}

	for (npos = 0, i = 1, tmp = len - 2, j = -1; i < tmp && npos < 50; ++i) {
		if ((grads[i] > 0
			&& ((grads[i] > grads[i - 1] && grads[i] >= grads[i + 1]) ||
			(grads[i] >= grads[i - 1] && grads[i] > grads[i + 1])))
			|| (grads[i] < 0
				&& ((grads[i] < grads[i - 1] && grads[i] <= grads[i + 1]) ||
				(grads[i] <= grads[i - 1] && grads[i] < grads[i + 1])))) {
			if ((gradabs[i] << 2) < maxgrad)    /**剔除小干扰**/
				continue;

			if (i >= center && j == -1)
				j = npos;
			maxgradpos[npos++] = i;
		}
	}

	if (npos < 2)
		return false;

	if (j < 1) {
		i = maxgradpos[npos - 2];
		j = maxgradpos[npos - 1];
	} else {
		tmp = j;
		i = maxgradpos[j - 1];
		j = maxgradpos[j];
		if (grads[i] * grads[j] >= 0) {
			if (gradabs[i] > gradabs[j]) {
				j = tmp + 1;
				if (j >= npos)
					return false;

				j = maxgradpos[j];
			} else {
				i = tmp - 2;
				if (i < 0)
					return false;

				i = maxgradpos[i];
			}
		}
	}

	if (grads[i] * grads[j] >= 0 || ((isblack && grads[i] < 0) || (!isblack && grads[i] > 0)))
		return false;


	head = get_edgepos_16x(grads, len - 1, i);
	tail = get_edgepos_16x(grads, len - 1, j);
	*w16x = tail - head;
	*center_offset = ((head + tail + 1) >> 1) - (center << 4);
#if 0
	head = i;
	tail = j;
	last_diff = grads[head] - grads[head - 1];
	for (i = head - 1; i >= 0; --i) {
		if (grads[i] * grads[i + 1] <= 0) {
			head = (i + 1) << 4;
			break;
		}

		diff = grads[i + 1] - grads[i];
		if (last_diff * diff < 0) {
			head = (i << 4) - (last_diff << 4) / (diff - last_diff);
			break;
		}
	}
	if (i < 0)
		head = 0;

	last_diff = grads[tail + 1] - grads[tail];
	for (j = len - 1, i = tail + 1; i <= j; ++i) {
		if (grads[i - 1] * grads[i] <= 0) {
			tail = (i - 1) << 4;
			break;
		}

		diff = grads[i] - grads[i - 1];
		if (last_diff * diff < 0) {
			tail = (i << 4) - (last_diff << 4) / (diff - last_diff);
			break;
		}
	}
	if (i > j)
		tail = j << 4;
	*w16x = tail - head;
	*center_offset = ((head + tail + 1) >> 1) - (center << 4);
#endif

	return true;
}

bool dotcode_gooddot_confirmx(const struct image *srcimg, struct point *pt16x, int *plen, const bool isblack)
{
	struct point pt;
	unsigned char imgdata[100];
	int i, j, len, off, pos;

	if (*plen >= 49 || *plen <= 3)
		return false;

	len = 0;
	pos = *plen + 1;
	pt.x = (pt16x->x + 8) >> 4;
	pt.y = (pt16x->y + 8) >> 4;
	i = pt.y * srcimg->width;
	off = pt.x + (*plen + 1);
	if (off >= (int)srcimg->width) {
		off = srcimg->width - 1;
	}
	j = i + off;

	off = pt.x - (*plen + 1);
	if (off < 0) {
		pos += off;
		off = 0;
	}
	i = i + off;

	while (i <= j) {
		imgdata[len++] = srcimg->data[i++];
	}

	if (!get_dots_edge(imgdata, len, pos, &i, &len, isblack)) {
		return false;
	}

	*plen = (len + 8) >> 4;
	pt16x->x = (pt.x << 4) + i;

	return true;
}

bool dotcode_gooddot_confirmy(const struct image *srcimg, struct point *pt16x, int *plen, const bool isblack)
{
	struct point pt;
	unsigned char imgdata[100];
	int i, j, len, off, pos;

	if (*plen >= 49 || *plen <= 3)
		return false;

	len = 0;
	pos = *plen + 1;
	pt.x = (pt16x->x + 8) >> 4;
	pt.y = (pt16x->y + 8) >> 4;
	off = pt.y + (*plen + 1);
	if (off >= (int)srcimg->height) {
		off = srcimg->height - 1;
	}
	off *= srcimg->width;
	j = pt.x + off;

	off = pt.y - (*plen + 1);
	if (off < 0) {
		pos += off;
		off = 0;
	}
	off *= srcimg->width;
	i = pt.x + off;

	while (i <= j) {
		imgdata[len++] = srcimg->data[i];
		i += srcimg->width;
	}

	if (!get_dots_edge(imgdata, len, pos, &i, &len, isblack)) {
		return false;
	}

	*plen = (len + 8) >> 4;
	pt16x->y = (pt.y << 4) + i;

	return true;
}

bool dotcode_gooddot_confirm45(const struct image *srcimg, const struct point *pt16x, int *plen, const bool isblack)
{
	int r, k;
	struct point pt;
	int i, j, len, off, pos;
	unsigned char imgdata[100];

	if (*plen >= 49 || *plen <= 3)
		return false;

	len = 0;
	pos = *plen + 1;
	pt.x = (pt16x->x + 8) >> 4;
	pt.y = (pt16x->y + 8) >> 4;
	r = pt.x - (*plen + 1);
	if (r < 0) {
		return false;
	}

	k = pt.x + (*plen + 1);
	if (k >= (int)srcimg->width) {
		k = srcimg->width - 1;
	}

	off = pt.y + (*plen + 1);
	if (off >= (int)srcimg->height) {
		off = srcimg->height - 1;
	}
	j = off * srcimg->width;

	off = pt.y - (*plen + 1);
	if (off < 0) {
		pos += off;
		off = 0;
	}
	i = off * srcimg->width;

	while (i <= j && r <= k) {
		imgdata[len++] = srcimg->data[i + r];
		i += srcimg->width;
		++r;
	}

	if (!get_dots_edge(imgdata, len, pos, &i, &len, isblack)) {
		return false;
	}

	*plen = ((len * 3 >> 1) + 8) >> 4;

	return true;
}

bool dotcode_gooddot_confirm135(const struct image *srcimg, const struct point *pt16x, int *plen, const bool isblack)
{
	int r, k;
	struct point pt;
	int i, j, len, off, pos;
	unsigned char imgdata[100];

	if (*plen >= 49 || *plen <= 3)
		return false;

	len = 0;
	pos = *plen + 1;
	pt.x = (pt16x->x + 8) >> 4;
	pt.y = (pt16x->y + 8) >> 4;
	r = pt.x - (*plen + 1);
	if (r < 0) {
		return false;
	}

	k = pt.x + (*plen + 1);
	if (k >= (int)srcimg->width) {
		k = srcimg->width - 1;
	}

	off = pt.y + (*plen + 1);
	if (off >= (int)srcimg->height) {
		off = srcimg->height - 1;
	}
	j = off * srcimg->width;

	off = pt.y - (*plen + 1);
	if (off < 0) {
		pos += off;
		off = 0;
	}
	i = off * srcimg->width;

	while (i <= j && r <= k) {
		imgdata[len++] = srcimg->data[j + r];
		j -= srcimg->width;
		++r;
	}

	if (!get_dots_edge(imgdata, len, pos, &i, &len, isblack)) {
		return false;
	}

	*plen = ((len * 3 >> 1) + 8) >> 4;

	return true;
}

static bool dotcode_judge_width(const unsigned int a, const unsigned int b)
{
	if (a <= 0 || b <= 0)
		return false;

	if (a == b)
		return true;

	if (b <= 4 && a <= 5)
		return true;

	if (a <= 4 && b <= 5)
		return true;

	return (a << 1) / unsigned_diff(a, b) >= 4;
}

int dotcode_edge_search_length(const int ref)
{
	int value;

	if (ref >= 50) {
		value = -1;
	} else {
		value = ref << 1;
		if (value >= 50)
			value = 49;
	}

	return value;
}

int image_find_dot_by_grad(const struct image *img)
{
	bool ret;
	unsigned int i, j, rfe_cnt;
	struct dotcode_point pt;
	struct point coordinate, dbgcoord;
	struct point hori_edge_off;
	struct point hori_edge_start;
	struct image_raise_fall_edge rfe_hori[500];

	if (img == NULL)
		return 0;

	hori_edge_off.x = 1;
	hori_edge_off.y = 0;
	hori_edge_start.x = 0;
	for (j = 0; j < img->height; ++j) {
		hori_edge_start.y = j;
		rfe_cnt = image_find_raise_fall_edges_by_offset_dotcode(img,
			&hori_edge_start, &hori_edge_off, img->width, rfe_hori, 500);
		if (rfe_cnt < 2)
			continue;

		for (i = 1; i < rfe_cnt; ++i) {
			coordinate.y = j << 4;
			pt.nw = rfe_hori[i].dpos_256x - rfe_hori[i - 1].dpos_256x;
			coordinate.x = (rfe_hori[i - 1].dpos_256x + ((pt.nw + 1) >> 1) + 16) >> 4;
			pt.nw = (pt.nw + 128) >> 8;
			pt.center = coordinate;

			(void)dbgcoord;
			dbgcoord.x = (coordinate.x + 8) >> 4;
			dbgcoord.y = (coordinate.y + 8) >> 4;
			if (dbgcoord.y == 102 && dbgcoord.x == 140) {
				hori_edge_off.y = 0;
			}
			pt.isblack = rfe_hori[i - 1].type != IMAGE_RFEDGE_TYPE_FALL;
			pt.nh = dotcode_edge_search_length(pt.nw);
			ret = dotcode_gooddot_confirmy(img, &coordinate, &pt.nh, pt.isblack);
			if (!ret || !dotcode_judge_width(pt.nw, pt.nh))
				continue;

			pt.nw = dotcode_edge_search_length(pt.nh);
			ret = dotcode_gooddot_confirmx(img, &coordinate, &pt.nw, pt.isblack);
			if (!ret || !dotcode_judge_width(pt.nh, pt.nw))
				continue;

			pt.n45 = dotcode_edge_search_length(imax(pt.nw, pt.nh));
			ret = dotcode_gooddot_confirm45(img, &coordinate, &pt.n45, pt.isblack);
			if (!ret || (!dotcode_judge_width(imax(pt.nw, pt.nh), pt.n45)))
				continue;

			pt.n135 = dotcode_edge_search_length(imax(pt.nw, pt.nh));
			ret = dotcode_gooddot_confirm135(img, &coordinate, &pt.n135, pt.isblack);
			if (!ret || !dotcode_judge_width(pt.n45, pt.n135))
				continue;

			pt.center.x = (coordinate.x + 8) >> 4;
			pt.center.y = (coordinate.y + 8) >> 4;
			if (pt.isblack) {
				ushow_pt(1, pt.center.x, pt.center.y, REDCOLOR);
			} else {
				ushow_pt(1, pt.center.x, pt.center.y, YELLOWCOLOR);
			}
		}
	}

	return 0;
}

unsigned int dotcode_detect_point(const struct image *img,
	struct dotcode_point *pdtp, const unsigned int ndtp)
{
	image_find_dot_by_grad(img);
	return 0;
}