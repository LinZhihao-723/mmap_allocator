#include "default_config.h"

char* const env_mmap_heap_size = "ENV_MMAP_HEAP_SIZE";
char* const env_mmap_alloctor_min_bsize = "ENV_MMAP_ALLOCATOR_MIN_BSIZE";
char* const env_naming_template = "ENV_NAMING_TEMPLATE";

const size_t default_mmap_heap_size = (size_t) 1024 * 1024 * 1024 * 512;
const size_t default_mmap_alloctor_min_bsize = 1024 * 1024 * 4;
char* const  default_naming_template = "/tmp/mmap_alloc.XXXXXXXX";