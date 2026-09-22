#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/max_heap.h"

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================
 */

/* Swap two heap nodes */
static void swap_nodes(heap_node **a, heap_node **b) {
    heap_node *tmp = *a;
    *a = *b;
    *b = tmp;
}

/* Update min_node_index after a modification - O(1) comparison update */
static void update_min_node_index(max_heap *heap, int changed_index) {
    if (heap->size == 0) {
        heap->min_node_index = -1;
        return;
    }
    
    // If we only have one node, it's the min
    if (heap->size == 1) {
        heap->min_node_index = 0;
        return;
    }
    
    // If min_node_index is invalid, do full scan
    if (heap->min_node_index < 0 || heap->min_node_index >= heap->size) {
        heap->min_node_index = 0;
        for (int i = 1; i < heap->size; i++) {
            if (heap->nodes[i]->risk < heap->nodes[heap->min_node_index]->risk)
                heap->min_node_index = i;
        }
        return;
    }
    
    // Check if changed node is now the minimum
    if (changed_index >= 0 && changed_index < heap->size) {
        if (heap->nodes[changed_index]->risk < heap->nodes[heap->min_node_index]->risk) {
            heap->min_node_index = changed_index;
        }
    }
}

/* Full scan to find min_node_index (used after removal) */
static void find_min_node_index(max_heap *heap) {
    if (heap->size == 0) {
        heap->min_node_index = -1;
        return;
    }
    
    heap->min_node_index = 0;
    for (int i = 1; i < heap->size; i++) {
        if (heap->nodes[i]->risk < heap->nodes[heap->min_node_index]->risk)
            heap->min_node_index = i;
    }
}

/* Bubble up to maintain max-heap property */
static void bubble_up(max_heap *heap, int index) {
    if (index == 0) return; // root
    int parent = (index - 1) / 2;
    if (heap->nodes[index]->risk > heap->nodes[parent]->risk) {
        swap_nodes(&heap->nodes[index], &heap->nodes[parent]);
        
        // Update min tracking if we swapped the min node
        if (heap->min_node_index == index) {
            heap->min_node_index = parent;
        } else if (heap->min_node_index == parent) {
            heap->min_node_index = index;
        }
        
        bubble_up(heap, parent);
    }
}

/* Bubble down max-heap property (used after pop) */
static void bubble_down(max_heap *heap, int index) {
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < heap->size && heap->nodes[left]->risk > heap->nodes[largest]->risk)
        largest = left;
    if (right < heap->size && heap->nodes[right]->risk > heap->nodes[largest]->risk)
        largest = right;

    if (largest != index) {
        swap_nodes(&heap->nodes[index], &heap->nodes[largest]);
        
        // Update min tracking if we swapped the min node
        if (heap->min_node_index == index) {
            heap->min_node_index = largest;
        } else if (heap->min_node_index == largest) {
            heap->min_node_index = index;
        }
        
        bubble_down(heap, largest);
    }
}

/* ============================================================================
 * CORE FUNCTIONS
 * ============================================================================
 */

max_heap* max_heap_new(int post_capacity) {
    max_heap *heap = malloc(sizeof(max_heap));
    if (!heap) return NULL;

    heap->size = 0;
    heap->total_posts = 0;
    heap->post_capacity = post_capacity;
    heap->capacity = post_capacity; // simplify: 1 node per post max
    heap->min_node_index = -1;
    heap->nodes = malloc(sizeof(heap_node*) * heap->capacity);
    if (!heap->nodes) {
        free(heap);
        return NULL;
    }
    return heap;
}

void max_heap_free(max_heap *heap) {
    if (!heap) return;
    for (int i = 0; i < heap->size; i++) {
        free(heap->nodes[i]->posts);
        free(heap->nodes[i]);
    }
    free(heap->nodes);
    free(heap);
}

bool max_heap_is_empty(const max_heap *heap) {
    return (!heap || heap->total_posts == 0);
}

bool max_heap_is_full(const max_heap *heap) {
    return (heap && heap->total_posts >= heap->post_capacity);
}

int max_heap_size(const max_heap *heap) {
    return heap ? heap->total_posts : 0;
}

post* max_heap_peek_max(const max_heap *heap) {
    if (!heap || heap->size == 0) return NULL;
    return heap->nodes[0]->posts[0];
}

post* max_heap_peek_min(const max_heap *heap) {
    if (!heap || heap->size == 0 || heap->min_node_index < 0) return NULL;
    return heap->nodes[heap->min_node_index]->posts[0];
}

bool max_heap_evict_min(max_heap *heap, post *out) {
    if (!heap || heap->size == 0 || heap->min_node_index < 0) return false;

    int min_idx = heap->min_node_index;
    *out = *(heap->nodes[min_idx]->posts[0]);

    // OPTIMIZATION: Replace with last post instead of shifting - O(1)
    int last_post_idx = heap->nodes[min_idx]->post_count - 1;
    heap->nodes[min_idx]->posts[0] = heap->nodes[min_idx]->posts[last_post_idx];
    heap->nodes[min_idx]->post_count--;
    heap->total_posts--;

    // Remove node if empty
    if (heap->nodes[min_idx]->post_count == 0) {
        free(heap->nodes[min_idx]->posts);
        free(heap->nodes[min_idx]);
        
        // Replace with last node
        heap->nodes[min_idx] = heap->nodes[heap->size - 1];
        heap->size--;
        
        // Need to recompute min after removal
        find_min_node_index(heap);
        
        // Maintain heap property if we moved a node
        if (min_idx < heap->size) {
            bubble_up(heap, min_idx);
            bubble_down(heap, min_idx);
        }
    }
    
    return true;
}

bool max_heap_pop_max(max_heap *heap, post *out) {
    if (!heap || heap->size == 0) return false;
    *out = *(heap->nodes[0]->posts[0]);

    // OPTIMIZATION: Replace with last post instead of shifting - O(1)
    int last_post_idx = heap->nodes[0]->post_count - 1;
    heap->nodes[0]->posts[0] = heap->nodes[0]->posts[last_post_idx];
    heap->nodes[0]->post_count--;
    heap->total_posts--;

    if (heap->nodes[0]->post_count == 0) {
        free(heap->nodes[0]->posts);
        free(heap->nodes[0]);
        heap->nodes[0] = heap->nodes[heap->size - 1];
        heap->size--;
        
        if (heap->size > 0) {
            // Check if we need to update min_node_index
            if (heap->min_node_index == heap->size) {
                // We just moved the old last node to position 0
                heap->min_node_index = 0;
            }
            bubble_down(heap, 0);
            // After bubble_down, recheck min
            find_min_node_index(heap);
        } else {
            heap->min_node_index = -1;
        }
    } else {
        // Node still has posts, just update min if this was the min node
        if (heap->min_node_index == 0) {
            find_min_node_index(heap);
        }
    }
    
    return true;
}

bool max_heap_insert(max_heap *heap, post *p) {
    if (!heap || !p) return false;

    // Delete posts with risk = 1 automatically
    if (p->risk >= 1.0) return false;

    // OPTIMIZATION: Use cached min_node_index for O(1) comparison
    if (heap->total_posts >= heap->post_capacity) {
        if (heap->min_node_index >= 0 && 
            heap->min_node_index < heap->size &&
            p->risk > heap->nodes[heap->min_node_index]->risk) {
            post temp;
            max_heap_evict_min(heap, &temp);
        } else {
            return false; // discard post
        }
    }

    // Check if node with same risk exists
    for (int i = 0; i < heap->size; i++) {
        if (heap->nodes[i]->risk == p->risk) {
            // Append post
            if (heap->nodes[i]->post_count >= heap->nodes[i]->post_capacity) {
                heap->nodes[i]->post_capacity *= 2;
                heap->nodes[i]->posts = realloc(heap->nodes[i]->posts, 
                                                sizeof(post*) * heap->nodes[i]->post_capacity);
            }
            heap->nodes[i]->posts[heap->nodes[i]->post_count++] = p;
            heap->total_posts++;
            return true;
        }
    }

    // Create new node
    heap_node *node = malloc(sizeof(heap_node));
    node->risk = p->risk;
    node->post_capacity = 2;
    node->post_count = 1;
    node->posts = malloc(sizeof(post*) * node->post_capacity);
    node->posts[0] = p;

    int new_index = heap->size;
    heap->nodes[heap->size++] = node;
    heap->total_posts++;
    
    // Update min_node_index with O(1) comparison
    if (heap->min_node_index < 0 || node->risk < heap->nodes[heap->min_node_index]->risk) {
        heap->min_node_index = new_index;
    }
    
    bubble_up(heap, heap->size - 1);
    
    // After bubble_up, the new node might have moved, so recheck min
    update_min_node_index(heap, heap->size - 1);

    return true;
}