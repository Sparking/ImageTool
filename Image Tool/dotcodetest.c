#include "dotcode_detect_point.h"

int get_edgepos_16x(const int *grads, const int len, const int srchpos)
{
	int pos16x, i, j;

	pos16x = 0;
	if (grads[srchpos] > 0) {
		for (i = srchpos; i >= 0; --i) {
			if (grads[i] <= 0)
				break;
		}

		for (j = grads[i], pos16x = grads[i] * i; i++ < len;) {
			if (grads[i] <= 0)
				break;

			pos16x += grads[i] * i;
			j += grads[i];
		}
	} else {
		for (i = srchpos; i >= 0; --i) {
			if (grads[i] >= 0)
				break;
		}

		for (j = grads[i], pos16x = grads[i] * i; i++ < len;) {
			if (grads[i] >= 0)
				break;

			pos16x += grads[i] * i;
			j += grads[i];
		}
	}
	if (j == 0)
		return 0;

	pos16x = (pos16x << 4) / j;

	return pos16x;
}

bool get_dots_edge(unsigned char *data, const int len, const int center, int *center_offset, int *w16x, int *isblack)
{
	int grads[100], head, tail;
	int i, j, npos, tmp;
	unsigned char gradabs[100], maxgrad;
	unsigned char maxgradpos[50];

	for (maxgrad = 0, i = 1, j = 0; i < len; ++i, ++j) {
		grads[j] = (int)data[i] - (int)data[j];
		if (grads[j] < 0) {
			gradabs[j] = (unsigned char)-grads[j];
		} else {
			gradabs[j] = (unsigned char)grads[j];
		}
		if (maxgrad < gradabs[j])
			maxgrad = gradabs[j];
	}

	for (npos = 0, i = 1, tmp = len - 2, j = -1; i < tmp && npos < 50; ++i) {
		if ((gradabs[i] > gradabs[i - 1] && gradabs[i] >= gradabs[i + 1]) ||
			(gradabs[i] >= gradabs[i - 1] && gradabs[i] > gradabs[i + 1])) {
			if (gradabs[i] * 5 < (maxgrad << 1))    /**剔除小干扰**/
				continue;

			maxgradpos[npos++] = i;
			if (i >= center && j == -1)
				j = npos - 1;
		}
	}

	if (npos < 2 || j < 1)
		return false;

	i = maxgradpos[j - 1];
	j = maxgradpos[j];
	if (grads[i] * grads[j] >= 0)
		return false;

	head = get_edgepos_16x(grads, len - 1, i);
	tail = get_edgepos_16x(grads, len - 1, j);
	if (head == 0 || tail == 0)
		return false;

	*w16x = tail - head;
	*center_offset = ((head + tail + 16) >> 5) - center;
	*isblack = data[(head + tail + 16) >> 5] < (data[(head + 8) >> 4] + data[(tail + 8) >> 4]);

	return true;
}

int image_find_dot_by_grad(const struct image *srcimg, const struct point *pt, const int len)
{
	int i, j, n;
	struct image *gray, *img;
	unsigned char imgdata[480];

	gray = image_convert_gray(srcimg);
	if (gray == NULL)
		return -1;

	img = image_convert_format(gray, IMAGE_FORMAT_BGRA);
	if (img == NULL) {
		image_release(gray);
		return -1;
	}

	n = 0;
	i = pt->y * gray->width + pt->x;
	j = i + len + 1;
	i = i - len - 1;
	while (i <= j)
		imgdata[n++] = gray->data[i++];

	if (get_dots_edge(imgdata, n, len + 1, &i, &n, &j)) {
	}
	image_release(gray);
	image_release(img);

	return 0;
}

unsigned int dotcode_detect_point(const struct image *img,
	struct dotcode_point *pdtp, const unsigned int ndtp)
{
	unsigned char imgdata[100];
	int n, i, j, len;
	struct point pnt, *pt = &pnt;

	len = 20;
	pt->x = 100;
	pt->y = 200;
	n = 0;
	i = pt->y * img->width + pt->x;
	j = i + len + 1;
	i = i - len - 1;
	while (i <= j)
		imgdata[n++] = img->data[i++];

	if (get_dots_edge(imgdata, n, len + 1, &i, &n, &j)) {
		ushow_pt(1, pt->x + i, pt->y, REDCOLOR);
	} else {
		ushow_pt(1, pt->x, pt->y, GREENCOLOR);
	}

	return 0;
}