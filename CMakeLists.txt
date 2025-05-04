cmake_minimum_required(VERSION 3.26)

project(MMapAllocator VERSION 0.0.1 LANGUAGES C)

add_library(mmap_allocator SHARED
        src/mmap_allocator/default_config.c
        src/mmap_allocator/heap.c
        src/mmap_allocator/mmap_allocator.c
        src/mmap_allocator/mmap_mgr.c
        src/mmap_allocator/profiling.c
)

target_include_directories(mmap_allocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
