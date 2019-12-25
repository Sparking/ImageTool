#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INLINE
#ifdef __GNUC__
#define INLINE static inline __attribute__((always_inline))
#define UNUSED __attribute__((unused))
#else
#define INLINE static inline
#ifndef UNUSED
#define UNUSED
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif