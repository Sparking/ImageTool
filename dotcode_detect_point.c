#include "dotcode_detect_point.h"

unsigned int dotcode_detect_point(const struct image *img,
        const struct point *pseed, const unsigned int inverse,
        struct dotcode_point *pdtp, const unsigned int ndtp);
{
    unsigned int cnt;

#if defined(CHECK_ENTRY_PARAM)
    if (img == NULL || pseed == NULL || pdtp == NULL || ndtp == 0)
        return 0;
#endif


    return cnt;
}

