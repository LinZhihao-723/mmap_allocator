#ifndef MMAP_ALLOCATOR_LIST_H
#define MMAP_ALLOCATOR_LIST_H

#include <mmap_allocator/constants.h>
#include <mmap_allocator/std_binding.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct list_node_base {
    struct list_node_base *prev;
    struct list_node_base *next;
    uint8_t *addr; // The block actual address
    size_t size;   // Size of the block
    size_t idx;
    bool is_free;
} list_node_base_t;

typedef list_node_base_t *list_node_t;

typedef struct list {
    list_node_t virtual_head;
    list_node_t virtual_tail;
} list_t;

list_node_t FORCE_INLINE list_create_node(list_node_t prev, list_node_t next, uint8_t *addr,
                                          size_t const size, size_t const idx, bool const is_free) {
    list_node_t new_node = (list_node_t)std_malloc(sizeof(list_node_base_t));
    if (!new_node) {
        return NULL;
    }
    *new_node = (list_node_base_t){
        .prev = prev, .next = next, .addr = addr, .size = size, .idx = idx, .is_free = is_free};
    return new_node;
}

bool FORCE_INLINE list_init(list_t *list, uint8_t *addr, size_t const size) {
    list_node_t virtual_head = (list_node_t)std_malloc(sizeof(list_node_base_t));
    list_node_t init_node = (list_node_t)std_malloc(sizeof(list_node_base_t));
    list_node_t virtual_tail = (list_node_t)std_malloc(sizeof(list_node_base_t));
    if (!virtual_head || !init_node || !virtual_tail) {
        fprintf(stderr, "Error: failed to allocate memory for the node list.\n");
        return false;
    }

    *virtual_head = (list_node_base_t){
        .prev = NULL, .next = init_node, .addr = NULL, .size = 0, .idx = 0, .is_free = false};

    *init_node = (list_node_base_t){.prev = virtual_head,
                                    .next = virtual_tail,
                                    .addr = addr,
                                    .size = size,
                                    .idx = 0,
                                    .is_free = true};

    *virtual_tail = (list_node_base_t){
        .prev = init_node, .next = NULL, .addr = NULL, .size = 0, .idx = 0, .is_free = false};

    *list = (list_t){.virtual_head = virtual_head, .virtual_tail = virtual_tail};

    return true;
}

void FORCE_INLINE node_unlink(list_node_t node) {
    assert(node);
    assert(node->next != node->prev->next && "WTF???");
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = node->next = NULL;
}

list_node_t FORCE_INLINE list_find_in_use(list_t *list, uint8_t *addr) {
    list_node_t curr = list->virtual_head->next;
    list_node_t end = list->virtual_tail;
    while (curr != end) {
        assert(curr);
        if (addr == curr->addr) {
            assert(!curr->is_free && "Double free detected...");
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

#endif
