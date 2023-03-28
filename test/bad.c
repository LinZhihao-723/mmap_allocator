#include <stdlib.h>
#include <stdio.h>

int main() {
  int* p = NULL;
  int* q = NULL;

  p = malloc((size_t) 1024 * 1024 * 1024 * 128 * 6);
  if (!p) {
    fprintf(stderr, "Success!\n");
  } else {
    fprintf(stderr, "Failed.\n");
    return -1;
  }

  p = malloc((size_t) 1024 * 1024 * 1024 * 128);
  if (p) {
    fprintf(stderr, "Success!\n");
  } else {
    fprintf(stderr, "Failed.\n");
    return -1;
  }

  p = malloc((size_t) 1024 * 1024 * 1024 * 128 * 6);
  if (!p) {
    fprintf(stderr, "Success!\n");
  } else {
    fprintf(stderr, "Failed.\n");
    return -1;
  }

  p = malloc((size_t) 1024 * 1024 * 1024 * 128);
  if (p) {
    fprintf(stderr, "Success!\n");
  } else {
    fprintf(stderr, "Failed.\n");
    return -1;
  }

  p = malloc((size_t) 1024 * 1024 * 1024 * 128);
  if (p) {
    fprintf(stderr, "Success!\n");
  } else {
    fprintf(stderr, "Failed.\n");
    return -1;
  }

  p = malloc((size_t) 1024 * 1024 * 1024 * 128);
  if (p) {
    fprintf(stderr, "Success!\n");
  } else {
    fprintf(stderr, "Failed.\n");
    return -1;
  }

  q = malloc((size_t) 1024 * 1024 * 1024 * 128);
  if (!q) {
    fprintf(stderr, "Success!\n");
  } else {
    fprintf(stderr, "Failed.\n");
    return -1;
  }

  fprintf(stderr, "Success! Next test free.\n");

  free(p);
  free(q);
  int* m = p + 1;
  free(m);

  fprintf(stderr, "Success! It's the end of this test.\n");
  return 0;
}