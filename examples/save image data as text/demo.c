#include <stdio.h>
#include "image.h"

int main(int argc, char *argv[])
{
	int i;
	FILE *fp;
	struct image *img;
	struct image *gray;
	unsigned int x, y, off;
	char savefilename[2048];

	if (argc < 2)
		return -1;

	for (i = 1; i < argc; ++i) {
		img = image_open(argv[i]);
		if (img == NULL) {
			fprintf(stderr, "failed to open image file : %s.\n", argv[i]);
			continue;
		}

		if (img->format != IMAGE_FORMAT_GRAY) {
			gray = image_convert_gray(img);
			image_release(img);
			if (gray == NULL)
				continue;

			img = gray;
		}

		snprintf(savefilename, sizeof(savefilename), "%s.m", argv[i]);
		if ((fp = fopen(savefilename, "w")) == NULL) {
			image_release(img);
			continue;
		}

		fprintf(fp, "x = 1:1:%u; y = 1:1:%u;\n", img->width, img->height);
		fprintf(fp, "color = [\n");
		for (y = 0, off = 0; y < img->height; ++y, off += img->row_size) {
			fprintf(fp, "[");
			for (x = 0; x < img->width; ++x) {
				fprintf(fp, "%u ", img->data[off + x]);
			}
			if (y == img->height - 1) {
				fprintf(fp, "]];\n");
			} else {
				fprintf(fp, "],\n");
			}
		}
		fprintf(fp, "mesh(x,y,color);\n");

		image_release(img);
		fclose(fp);
	}

	return 0;
}
