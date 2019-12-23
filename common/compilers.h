#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INLINE
#ifdef __GNUC__
#define INLINE static inline __attribute__((always_inline))
#else
#define INLINE static inline
#endif
#endif

#ifdef __cplusplus
}
#endif