#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define FORCE_INLINE inline __attribute__((always_inline))
#define LOCAL_HELPER static inline

extern void* (*std_malloc)(size_t);
extern void *(*std_calloc)(size_t, size_t);
extern void *(*std_free)(size_t);
extern void *(*std_realloc)(void *, size_t);
extern void *(*std_reallocarray)(void *, size_t, size_t);

#define FREE(x) (std_free((size_t) (x)))

#endif