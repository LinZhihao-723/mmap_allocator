#ifndef MMAP_ALLOCATOR_STD_BINDING_H
#define MMAP_ALLOCATOR_STD_BINDING_H

#include <stdlib.h>

extern void* (*std_malloc)(size_t);
extern void* (*std_calloc)(size_t, size_t);
extern void* (*std_free)(size_t);
extern void* (*std_realloc)(void*, size_t);
extern void* (*std_reallocarray)(void*, size_t, size_t);

#endif
