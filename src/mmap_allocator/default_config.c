#include "default_config.h"

char* const env_mmap_heap_size = "ENV_MMAP_HEAP_SIZE";
char* const env_mmap_allocator_min_bsize = "ENV_MMAP_ALLOCATOR_MIN_BSIZE";
char* const env_naming_template = "ENV_NAMING_TEMPLATE";

char* const env_profile_file_path = "ENV_PROFILE_FILE_PATH";
char* const env_profile_frequency = "ENV_PROFILE_FREQUENCY";

size_t const default_mmap_heap_size = (size_t)1024 * 1024 * 1024 * 512;
size_t const default_mmap_allocator_min_bsize = 1024 * 1024 * 4;
char* const default_naming_template = "/tmp/mmap_alloc.XXXXXXXX";
