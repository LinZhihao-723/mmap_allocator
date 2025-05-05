cmake_minimum_required(VERSION 3.22)

project(MMapAllocator VERSION 0.0.1 LANGUAGES C)

include(FetchContent)

set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MI_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(MI_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
set(MI_OVERRIDE OFF CACHE BOOL "" FORCE)
set(MI_OVERRIDE_PREFIX mim_ CACHE STRING "" FORCE)
set(MI_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        mimalloc
        GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
        GIT_TAG v2.1.2
)
FetchContent_MakeAvailable(mimalloc)

add_library(mmap_allocator SHARED
        src/mmap_allocator/default_config.c
        src/mmap_allocator/heap.c
        src/mmap_allocator/mmap_allocator.c
        src/mmap_allocator/mmap_mgr.c
        src/mmap_allocator/profiling.c
)

target_link_libraries(mmap_allocator PRIVATE mimalloc-static)

target_include_directories(mmap_allocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${mimalloc_SOURCE_DIR}/include
    ${mimalloc_BINARY_DIR}
)
