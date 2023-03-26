#include "heap.h"
#include "list.h"

/*
Since these functions are recursive, they cannot be inlined. 
Put them in the source code instead.
*/

/*
Heapify downward. This happens when we replace a node by a smaller number.
*/
void heap_heapify_down(heap_t* heap, const size_t idx) {
  const size_t left_idx = HEAP_GET_LEFT_IDX(idx);
  const size_t right_idx = HEAP_GET_RIGHT_IDX(idx);
  size_t largest_idx = idx;
  if (left_idx < heap->size) {
    largest_idx = heap_compare_GT(heap, largest_idx, left_idx);
  }
  if (right_idx < heap->size) {
    largest_idx = heap_compare_GT(heap, largest_idx, right_idx);
  }
  if (largest_idx != idx) {
    // Swap. Heapify needs to propagate downward.
    heap_swap(heap, idx, largest_idx);
    heap_heapify_down(heap, largest_idx);
  }
}

/*
Heapify upward. This happens when we replace a node by a larger number.
*/
void heap_heapify_up(heap_t* heap, const size_t idx) {
  if (idx <= HEAP_IDX_ROOT) return;

  const size_t parent = HEAP_GET_PARENT_IDX(idx);
  if (heap_compare_GT(heap, idx, parent) != parent) {
    heap_swap(heap, idx, parent);
    heap_heapify_up(heap, parent);
  }
}

list_node_t heap_allocate(heap_t* heap, const size_t size) {
  list_node_t root = heap_get_root(heap);
  if (!root) {
    return NULL;
  }

  // First, try to find the smallest node on the top of the heap >= size
  list_node_t victim = root;
  const size_t heap_size = heap->size;
  const size_t left_idx = HEAP_GET_LEFT_IDX(HEAP_IDX_ROOT);
  const size_t right_idx = HEAP_GET_RIGHT_IDX(HEAP_IDX_ROOT);
  list_node_t left = left_idx < heap_size ? 
    heap->node_array[left_idx] : NULL;
  list_node_t right = right_idx < heap_size ? 
    heap->node_array[right_idx] : NULL;
  
  if (left && left->size >= size) {
    victim = left;
  }
  if (right && right->size >= size && right->size < victim->size) {
    victim = right;
  }

  if (victim->size == size) {
    // Perfect match!
    heap_remove_idx(heap, victim->idx);
    victim->is_free = false;
    return victim;
  }

  // The block is larger than what's being asked. Need to split...
  list_node_t ret_node = list_create_node(
    victim->prev,
    victim,
    victim->addr,
    victim->size,
    HEAP_IDX_NULL,
    false
  );
  if (!ret_node) {
    // Running out of memory to allocate node...
    // But we can still return the allocated block
    heap_remove_idx(heap, victim->idx);
    victim->is_free = false;
    return victim;
  }

  victim->prev->next = ret_node;
  victim->prev = ret_node;
  victim->addr += size;
  victim->size -= size;
  heap_heapify_down(heap, victim->idx);

  return ret_node;
}

bool heap_free(heap_t* heap, list_node_t node) {
  // The idea is that we want to merge internal free blocks,
  // and try to avoid insert new node into the heap.
  if (node->next->is_free) {

    // The next block is free.
    if (node->prev->is_free) {
      // First check if the previous block is also free.
      // Merge and remove the previous is true.
      list_node_t previous = node->prev;
      node->size += previous->size;
      node->addr = previous->addr;
      list_remove(previous);
      heap_remove_idx(heap, previous->idx);
      FREE((size_t) previous);
    }

    // Merge the current into next.
    node->next->size += node->size;
    node->next->addr = node->addr;
    list_remove(node);
    FREE(node);
    return true;

  } else if (node->prev->is_free) {

    // Merge the current block into the previous.
    list_node_t previous = node->prev;
    previous->size += node->size;
    heap_heapify_up(heap, previous->idx);
    list_remove(node);
    FREE(node);
    return true;

  } else {

    // Nothing we can merge. Insert the current node into the heap.
    node->is_free = true;
    return heap_insert(heap, node);

  }
}