#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "constants.h"
#include "heap.h"
#include "list.h"
#include "mmap_mgr.h"

#define OUT_OF_MEMORY() do { \
  fprintf(stderr, "Out of space. Failed to allocate more memory.\n"); \
  errno = ENOMEM; \
} while (0)

/******************************************************************************/
// Allocator status
enum allocator_status {
  FAILED_TO_LOAD = -1,
  NOT_LOADED = 0,
  LOADED = 1
};
static enum allocator_status allocator_status = NOT_LOADED;

/******************************************************************************/
// References binding to the original stdlib functions.
void* (*std_malloc)(size_t) = NULL;
void *(*std_calloc)(size_t, size_t) = NULL;
void *(*std_free)(size_t) = NULL;
void *(*std_realloc)(void *, size_t) = NULL;
void *(*std_reallocarray)(void *, size_t, size_t) = NULL;

/******************************************************************************/
// Global lock.
static pthread_mutex_t glock = PTHREAD_MUTEX_INITIALIZER;

#define GLOBAL_LOCK_ACQUIRE() do {\
  pthread_mutex_lock(&glock);\
} while (0)

#define GLOBAL_LOCK_RELEASE() do {\
  pthread_mutex_unlock(&glock);\
} while (0)

/******************************************************************************/
// Parameters
static size_t page_size = 0;
static size_t mmap_region_size = (size_t) 1024 * 1024 * 1024 * 512; // 512GB
static size_t mmap_alloctor_min_size = 1024 * 1024 * 4;
static char* naming_template = "/tmp/mmap_alloc.XXXXXXXX";
static void* mmap_region_base = NULL;

#define CEILING_PAGE_SIZE(size) (((size) + (page_size - 1)) & ~(page_size - 1))

/******************************************************************************/
// Heap
heap_t mmap_heap;

/******************************************************************************/
// Allocate from mmap_heap
LOCAL_HELPER void mmap_allocator_init() {
  GLOBAL_LOCK_ACQUIRE();
  if (allocator_status != NOT_LOADED) {
    GLOBAL_LOCK_RELEASE();
    return;
  }

  // Getting the address of the default allocator APIs.
  std_malloc = dlsym(RTLD_NEXT, "malloc");
  std_free = dlsym(RTLD_NEXT, "free");
  std_calloc = dlsym(RTLD_NEXT, "calloc");
  std_realloc = dlsym(RTLD_NEXT, "realloc");
  std_reallocarray = dlsym(RTLD_NEXT, "reallocarray");

  if (
    !std_malloc ||
    !std_free ||
    !std_calloc ||
    !std_realloc ||
    !std_reallocarray
  ) {
    fprintf(stderr, "Failed to link to stdlib: %s\n", dlerror());
    allocator_status = FAILED_TO_LOAD;
    GLOBAL_LOCK_RELEASE();
    return;
  }

  // Initialize page size.
  page_size = sysconf(_SC_PAGE_SIZE);

  // Reserve mmap region.
  mmap_region_base = mmap_reserve(mmap_region_size);
  if (!mmap_region_base) {
    // Failed to reserve the mmap region.
    fprintf(stderr, "Failed to reserve mmap alloctor region.\n");
    allocator_status = FAILED_TO_LOAD;
    GLOBAL_LOCK_RELEASE();
    return;
  }

  // Initialize mmap heap.
  if (!heap_init(&mmap_heap, mmap_region_base, mmap_region_size)) {
    allocator_status = FAILED_TO_LOAD;
    GLOBAL_LOCK_RELEASE();
    return;
  }

  allocator_status = LOADED;
  GLOBAL_LOCK_RELEASE();
}

LOCAL_HELPER void* mmap_allocate(size_t size) {
  heap_check(&mmap_heap);
  if (size == 0) {
    // Ideally will never happen.
    return NULL;
  }

  // Round to ensure page aligned
  size = CEILING_PAGE_SIZE(size);

  GLOBAL_LOCK_ACQUIRE();
  list_node_t block = heap_allocate(&mmap_heap, size);
  GLOBAL_LOCK_RELEASE();

  if (!block) {
    return NULL;
  }

  assert(block->size == size);
  const int retval = mmap_maptemp(block->addr, block->size, naming_template);
  if (0 != retval) {
    // Failed to allocate the swap file.
    // We will return this node back to theheap.
    GLOBAL_LOCK_ACQUIRE();
    heap_free(&mmap_heap, block);
    GLOBAL_LOCK_RELEASE();
    fprintf(stderr, "Failed to create mmap region. Error code: %d\n", retval);
    return NULL;
  }

  return (void*) block->addr;
}

LOCAL_HELPER bool mmap_release(void* addr) {
  GLOBAL_LOCK_ACQUIRE();
  list_node_t block = list_find_in_use(&mmap_heap.node_list, addr);
  if (!block) {
    fprintf(stderr, "Failed to find the block with the given addr: %p\n", addr);
    GLOBAL_LOCK_RELEASE();
    return false;
  }

  // Unmap the region.
  if (mmap_unmap(addr, block->size)) {
    fprintf(stderr, "Failed to unmap the region.\n");
    GLOBAL_LOCK_RELEASE();
    return false;
  }

  if (!heap_free(&mmap_heap, block)) {
    fprintf(
      stderr, 
      "Failed to free as stdlib cannot allocate more memory for the heap.\n"
    );
    GLOBAL_LOCK_RELEASE();
    return false;
  }

  GLOBAL_LOCK_RELEASE();
  return true;
}

LOCAL_HELPER bool mmap_release_with_copy(
  list_node_t block, 
  void* dest, 
  const size_t new_size
) {
  void* addr = block->addr;
  // We will need to copy the data from the src to the new dest.
  const size_t copy_size = (block->size < new_size) ? block->size : new_size;
  memcpy(dest, addr, copy_size);

  // Unmap the region.
  if (mmap_unmap(addr, block->size)) {
    fprintf(stderr, "Failed to unmap the region.\n");
    GLOBAL_LOCK_RELEASE();
    return false;
  }

  GLOBAL_LOCK_ACQUIRE();
  bool success = heap_free(&mmap_heap, block);
  GLOBAL_LOCK_RELEASE();

  if (!success) {
    fprintf(
      stderr, 
      "Failed to free as stdlib cannot allocate more memory for the heap.\n"
    );
    return false;
  }
  return true;
}

/******************************************************************************/
// User library interface
void* malloc(size_t size);
void* calloc(size_t num_elements, size_t element_size);
void* reallocarray(void *addr, size_t size, size_t count);
void* realloc(void *addr, size_t size);

void* malloc(size_t size) {
  if (allocator_status == NOT_LOADED && !std_malloc) {
    mmap_allocator_init();
  }

  if (allocator_status != LOADED || size < mmap_alloctor_min_size) {
    return std_malloc(size);
  }

  void* ret = mmap_allocate(size);
  if (!ret) {
    OUT_OF_MEMORY();
  }
  heap_check(&mmap_heap);
  return ret;
}

void* calloc(size_t num_elements, size_t element_size) {
  if (allocator_status == NOT_LOADED && !std_calloc) {
    mmap_allocator_init();
  }

  const size_t total_size = num_elements * element_size;
  if (allocator_status != LOADED || total_size < mmap_alloctor_min_size) {
    return std_calloc(num_elements, element_size);
  }

  void* ret = mmap_allocate(total_size);
  if (!ret) {
    OUT_OF_MEMORY();
  }
  // As required of calloc, memory region must be initialized to 0.
  memset(ret, 0, total_size);
  return ret;
}

void* reallocarray(void *addr, size_t size, size_t count) {
  return realloc(addr, size * count);
}

void* realloc(void *addr, size_t size) {
  if (!addr) return malloc(size); // It should behave like malloc.

  if (allocator_status == NOT_LOADED && !std_realloc) {
    mmap_allocator_init();
  }

  if (allocator_status != LOADED) {
    return std_realloc(addr, size);
  }

  // mmap allocator is properly initialized.
  if (addr >= mmap_region_base) {

    GLOBAL_LOCK_ACQUIRE();
    list_node_t block = list_find_in_use(&mmap_heap.node_list, addr);
    GLOBAL_LOCK_RELEASE();
    if (!block) {
      fprintf(stderr, 
        "Realloc failed: cannot find the given address: %p.\n", addr);
      return NULL;
    }
    
    if (block->size >= size) return addr; // Already large enough.
    
    // const size_t alloc_size = CEILING_PAGE_SIZE(size);
    void* new_region = mmap_allocate(size);
    if (!new_region) {
      OUT_OF_MEMORY();
      return NULL;
    }

    if (!mmap_release_with_copy(block, new_region, size)) {
      fprintf(stderr, "Realloc copy failed.\n");
      mmap_release(new_region);
      return NULL;
    }

    heap_check(&mmap_heap);
    return new_region;
  }

  // The old address is allcoated by the std heap. We don't know the size.
  void* realloc_buffer = std_realloc(addr, size);
  if (!realloc_buffer) {
    fprintf(stderr, "Failed to reallocate a buffer using std realloc.\n");
    return NULL;
  }

  if (size < mmap_alloctor_min_size) {
    return realloc_buffer;
  }

  // const size_t alloc_size = CEILING_PAGE_SIZE(size);
  void* new_region = mmap_allocate(size);
  if (!new_region) {
    OUT_OF_MEMORY();
    FREE(realloc_buffer);
    return NULL;
  }

  memcpy(new_region, realloc_buffer, size);
  FREE(realloc_buffer);

  heap_check(&mmap_heap);
  return new_region;
}

void free(void* addr) {
  if (!addr) return;

  if (allocator_status == NOT_LOADED && !std_free) {
    mmap_allocator_init();
  }

  if (allocator_status != LOADED || addr < mmap_region_base) {
    FREE(addr);
    return;
  }

  if (!mmap_release(addr)) {
    fprintf(stderr, "Failed to free at addr: %p\n", addr);
  }
  heap_check(&mmap_heap);
}