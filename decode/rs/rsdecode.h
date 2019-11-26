#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "generic_gf.h"

extern int rsdecode(const struct generic_gf *field,unsigned int *received, unsigned int length, unsigned int twos);

#ifdef __cplusplus
}
#endif
