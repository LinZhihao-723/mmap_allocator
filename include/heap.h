#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "constants.h"
#include "list.h"

#define INIT_HEAP_CAPACITY 8

#define HEAP_GET_LEFT_IDX(idx) (idx * 2)
#define HEAP_GET_RIGHT_IDX(idx) (idx * 2 + 1)
#define HEAP_GET_PARENT_IDX(idx) (idx / 2)

#define HEAP_IDX_NULL 0
#define HEAP_IDX_ROOT 1

typedef struct heap {
  size_t capacity;
  size_t size;
  list_node_t* node_array;
} heap_t;

void heap_heapify_down(heap_t* heap, const size_t idx);
void heap_heapify_up(heap_t* heap, const size_t idx);

list_node_t heap_allocate(heap_t* heap, const size_t size);
bool heap_free(heap_t* heap, list_node_t node);

void FORCE_INLINE
heap_set_idx(heap_t* heap, const size_t idx, list_node_t node) {
  heap->node_array[idx] = node;
  node->idx = idx;
}

bool FORCE_INLINE 
heap_init(heap_t* heap, list_node_t init_node) {
  list_node_t* array = 
    (list_node_t*) std_malloc(sizeof(list_node_t) * INIT_HEAP_CAPACITY);
  if (!array) {
    fprintf(stderr, "Error: failed to allocate memory for the heap.\n");
    return false;
  }

  array[0] = NULL;

  *heap = (heap_t) {
    .capacity = INIT_HEAP_CAPACITY,
    .size = 2,
    .node_array = array
  };

  heap_set_idx(heap, 1, init_node);

  return true;
}

size_t FORCE_INLINE
heap_get_num_elements(heap_t* heap) {
  return heap->size - 1;
}

size_t FORCE_INLINE 
heap_compare_GT(heap_t* heap, const size_t l, const size_t r) {
  return (heap->node_array[l]->size > heap->node_array[r]->size) ? l : r;
}

void FORCE_INLINE 
heap_swap(heap_t* heap, const size_t a, const size_t b) {
  list_node_t temp = heap->node_array[a];
  heap->node_array[a] = heap->node_array[b];
  heap->node_array[b] = temp;
}

void FORCE_INLINE
heap_remove_idx(heap_t* heap, const size_t idx) {
  assert(idx > 0 && idx < heap->size && "Heap remove invalid idx.");

  heap->node_array[idx]->idx = HEAP_IDX_NULL;
  const size_t heap_elements = heap_get_num_elements(heap);
  if (heap_elements == idx) {
    heap->size -= 1;
    return;
  }

  heap_set_idx(heap, idx, heap->node_array[heap->size - 1]);
  heap->size -= 1;
  heap_heapify_down(heap, idx);
}

list_node_t FORCE_INLINE
heap_get_root(heap_t* heap) {
  if (heap->size < HEAP_IDX_ROOT) {
    return NULL;
  }
  return heap->node_array[HEAP_IDX_ROOT];
}

// Insert a new node to the heap with heap property maintained.
// Return 0 for success.
bool FORCE_INLINE
heap_insert(heap_t* heap, list_node_t node) {
  if (heap->size == heap->capacity) {
    // Needs to expand.
    list_node_t* new_array = std_realloc(
      heap->node_array, 
      sizeof(list_node_base_t) * heap->capacity * 2
    );
    if (new_array == NULL) {
      fprintf(
        stderr, 
        "Error: failed to allocate more memory for internal heap.\n"
      );
      return false;
    }
    heap->node_array = new_array;
    heap->capacity *= 2;
  }

  heap_set_idx(heap, heap->size, node);
  node->is_free = true;
  heap->size += 1;
  heap_heapify_up(heap, node->idx);

  return 0;
}

#endif