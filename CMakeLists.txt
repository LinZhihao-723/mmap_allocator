cmake_minimum_required(VERSION 3.26)

include(ExternalProject)

ExternalProject_Add(jemalloc_build
  GIT_REPOSITORY https://github.com/jemalloc/jemalloc.git
  GIT_TAG 5.3.0
  CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh --with-jemalloc-prefix=je_ --prefix=${CMAKE_BINARY_DIR}/install
  BUILD_COMMAND make -j
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
)

project(MMapAllocator VERSION 0.0.1 LANGUAGES C)

add_library(mmap_allocator SHARED
        src/mmap_allocator/default_config.c
        src/mmap_allocator/heap.c
        src/mmap_allocator/mmap_allocator.c
        src/mmap_allocator/mmap_mgr.c
        src/mmap_allocator/profiling.c
)

add_dependencies(mmap_allocator jemalloc_build)

target_include_directories(mmap_allocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/install/include
)

target_link_directories(mmap_allocator PRIVATE ${CMAKE_BINARY_DIR}/install/lib)
target_link_libraries(mmap_allocator PRIVATE jemalloc)
