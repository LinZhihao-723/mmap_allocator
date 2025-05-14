#ifndef MMAP_ALLOCATOR_MMAP_MGR_H
#define MMAP_ALLOCATOR_MMAP_MGR_H

#include <stdlib.h>

void* mmap_reserve(size_t const size);
int mmap_maptemp(void* addr, size_t const size, char* template);
int mmap_unmap(void* addr, size_t const size);

#endif
