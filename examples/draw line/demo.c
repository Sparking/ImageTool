#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "maths.h"

struct line_param {
	float K;
	float b;
	unsigned char bHori;
};

void pt_to_line(struct line_param *line,
		const struct point *a, const struct point *b)
{
	float dx, dy;

	dx = a->x - b->y;
	dy = a->y - b->y;
	if (unsigned_diff(a->x, b->x) >= unsigned_diff(a->y, b->y)) {
		if (dy == 0)
			return;

		line->bHori = 1;
		line->K = dx / dy;
		line->b = (float)a->y - line->K * a->x;
	} else {
		if (dx == 0)
			return;

		line->bHori = 0;
		line->K = dy / dx;
		line->b = (float)a->x - line->K * a->y;
	}
}

int main(void)
{
	struct point a, b;
	int line_xk, line_xb;
	unsigned char line_dir;
	struct line_param line;
	int x, y, off, off1, off2;
	struct image *img;

	printf("enter data:");
	scanf("%d %d %d %d", &a.x, &a.y, &b.x, &b.y);
	pt_to_line(&line, &a, &b);
	line_dir = line.bHori;
	line_xk = line.K * 64.0f;
	line_xb = line.b * 64.0f;
	img = image_create(a.y > b.y ? a.y: b.y, a.x > b.x ? a.x : b.x, IMAGE_FORMAT_GRAY);
	memset(img->data, 0, img->size);

	if (line_dir) {
		if (b.x > a.x)
			off = 1;
		else
			off = -1;

		off1 = line_xb + line_xk * a.x;
		off2 = line_xk * off;
		for (x = a.x; x != b.x; x += off, off1 += off2) {
			y = off1 >> 6;
			memset(img->data + y * img->row_size + x * img->pixel_size, 0xFF, img->pixel_size);
		}
	} else {
		if (b.y > a.y)
			off = 1;
		else
			off = -1;

		off1 = line_xb + line_xk * a.y;
		off2 = line_xk * off;
		for (y = a.y; y != b.y; y += off, off1 += off2) {
			x = off1 >> 6;
			memset(img->data + y * img->row_size + x * img->pixel_size, 0xFF, img->pixel_size);
		}
	}

	image_save("xxx.bmp", img, IMAGE_FILE_BITMAP);
	image_release(img);

	return 0;
}
