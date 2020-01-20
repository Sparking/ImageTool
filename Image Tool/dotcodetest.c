#include <dotcode_detect_point.h>

unsigned int dotcode_detect_point(const struct image *img,
	struct dotcode_point *pdtp, const unsigned int ndtp)
{
	image_find_dot_by_grad(img);
	return 0;
}
