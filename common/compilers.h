#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INLINE
#ifdef __GNUC__
#define INLINE static __inline __attribute__((always_inline))
#else
#define INLINE static __inline
#endif
#endif

#ifndef UNUSED
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif
#endif

#ifdef __cplusplus
}
#endif
