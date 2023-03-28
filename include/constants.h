#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define FORCE_INLINE inline __attribute__((always_inline))
#define LOCAL_HELPER static FORCE_INLINE

#define FREE(x) (std_free((size_t) (x)))

#define DEBUG_HEAP 0
#define DEBUG_LIST 0

#endif