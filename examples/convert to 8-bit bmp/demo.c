#include <stdio.h>
#include "image.h"

int main(int argc, char *argv[])
{
	int i;
	struct image *img;
	struct image *gray_img;

	if (argc < 2)
		return -1;

	for (i = 1; i < argc; ++i) {
		img = image_open(argv[i]);
		if (img == NULL) {
			fprintf(stderr, "failed to open image file : %s.\n", argv[i]);
			continue;
		}

		if (img->format == IMAGE_FORMAT_GRAY) {
			image_release(img);
			continue;
		}

		gray_img = image_convert_gray(img);
		image_release(img);
		if (gray_img == NULL) {
			fprintf(stderr, "failed to convert image file as gray : %s.\n", argv[i]);
			continue;
		}

		if (image_save(argv[i], gray_img, IMAGE_FILE_BITMAP) == -1) {
			fprintf(stderr, "failed to save image file as gray : %s.\n", argv[i]);
		}
		image_release(gray_img);
	}

	return 0;
}
