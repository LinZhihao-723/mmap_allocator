#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <pthread.h>
#include <stdbool.h>

// To run with 4GB memory.

#define BATCH_SIZE 1
#define MALLOC_SIZE_MAX (1024 * 1024 * 128)
#define BASE_SIZE (1024 * 1024)

#define REALLOC_TEST 1
#define TEST_TIME 4

#define TESTING_THREAD 4

#define PARALLEL_TEST 1

int test(int thread_id) {
  int* addrs[BATCH_SIZE];
  size_t size_list[BATCH_SIZE];
  size_t resize_list[BATCH_SIZE];

  for (int i = 0; i < TEST_TIME; ++i) {
    fprintf(stderr, "<%d> Test #%d\n", thread_id, i);

    // Allocate
    for (int j = 0; j < BATCH_SIZE; ++j) {
      fprintf(stderr, "  <%d> Allocate #%d\n", thread_id, j);
      size_t size = rand() % MALLOC_SIZE_MAX + BASE_SIZE;
      size_list[j] = size;
      addrs[j] = malloc(size * 4);
      for (int k = 0; k < size; k += 1000) {
        addrs[j][k] = j + k;
      }
    }

#if REALLOC_TEST
    // Reallocate
    for (int j = 0; j < BATCH_SIZE; ++j) {
      fprintf(stderr, " <%d> Realloc #%d\n", thread_id, j);
      size_t new_size = rand() % MALLOC_SIZE_MAX + BASE_SIZE;
      size_t old_size = size_list[j];
      resize_list[j] = new_size;
      addrs[j] = realloc(addrs[j], new_size * 4);
      for (int k = 0; k < new_size; k += 1000) {
        if (k >= old_size) {
          addrs[j][k] = j + k;
        }
        addrs[j][k] += j * 2;
      }
    }
#endif

    // Free
    for (int j = 0; j < BATCH_SIZE; ++j) {
      fprintf(stderr, "  <%d> Free #%d\n", thread_id, j);
      size_t size = resize_list[j];
      for (int k = 0; k < size; k += 1000) {
        if (addrs[j][k] != j * 3 + k) {
          fprintf(stderr, "<%d> Batch #%d failed: %d\n", thread_id, j, k);
          fprintf(stderr, "<%d> Expected: %d; Real: %d\n", thread_id, j * 3 + k, addrs[j][k]);
          return 1;
        }
      }
      free(addrs[j]);
    }
  }

  return 0;
}

int main() {
  srand(3190);

  int result[TESTING_THREAD];

#if PARALLEL_TEST
  #pragma omp parallel for
#endif
  for (int i = 0; i < TESTING_THREAD; ++i) {
    result[i] = test(i);
  }

  int retval = 0;
  for (int i = 0; i < TESTING_THREAD; ++i) {
    retval += result[i];
  }

  if (retval) {
    fprintf(stderr, "Test Finished. Failed :(\n");
  } else {
    fprintf(stderr, "Test Finished. You are all good :)\n");
  }

  return retval;
}