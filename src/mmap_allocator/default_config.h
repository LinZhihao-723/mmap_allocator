#ifndef MMAP_ALLOCATOR_DEFAULT_CONFIG_H
#define MMAP_ALLOCATOR_DEFAULT_CONFIG_H

#include <stdlib.h>

#define MAX_NAMING_TEMPLATE_SIZE 128

extern char* const env_mmap_heap_size;
extern char* const env_mmap_allocator_min_bsize;
extern char* const env_naming_template;

extern char* const env_profile_file_path;
extern char* const env_profile_frequency;

extern size_t const default_mmap_heap_size;
extern size_t const default_mmap_allocator_min_bsize;
extern char* const default_naming_template;

#endif
