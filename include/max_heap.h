#ifndef MAX_HEAP_H
#define MAX_HEAP_H

#include <stdbool.h>
#include "post.h"

/* ============================================================================
 * HEAP NODE STRUCTURE
 * ============================================================================
 * Each node holds all posts with the same risk level
 * Uses a dynamic array to store posts with identical risk
 */
typedef struct {
    float risk;                         // Risk level for this node
    post **posts;                       // Dynamic array of post pointers
    int post_count;                     // Number of posts in this node
    int post_capacity;                  // Allocated capacity for posts array
} heap_node;

/* ============================================================================
 * MIN-MAX HEAP STRUCTURE
 * ============================================================================
* Implements the priority queue ADT using a max heap.
 * - Supports efficient access to both the riskiest (max) and least risky (min) posts.
 * - Automatically deletes posts with risk level 1 (100% risky) upon insertion.
 * - Evicts the lowest-risk post when the heap is full.
 * - Processes riskiest posts first for human review.
 */
typedef struct {
    heap_node **nodes;     // Array representing the heap (max-heap by risk)
    int size;              // Current number of nodes in the heap
    int capacity;          // Max number of distinct risk nodes
    int total_posts;                   // Total number of posts stored
    int post_capacity;                  // Maximum total posts allowed
    int min_node_index;                 // Index of node with minimum risk (O(1) access)
} max_heap;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================
 */

/**
 * Create and initialize a new min-max heap
 * @param post_capacity Maximum number of posts the heap can hold
 * @return Pointer to newly created heap, or NULL on failure
 */
max_heap* max_heap_new(int post_capacity);

/**
 * Free all memory associated with the heap
 * @param heap Pointer to the heap to free
 */
void max_heap_free(max_heap *heap);

/**
* Insert a post into the heap.
 * - Posts with risk level 1 are deleted automatically and not stored.
 * - If heap is full and post risk > current min risk, evict min-risk post.
 * - If post risk <= current min and heap full, discard post.
 * @param heap Pointer to the heap
 * @param p Pointer to the post to insert
 * @return true if inserted, false if discarded or deleted (risk=1)
 */
bool max_heap_insert(max_heap *heap, post *p);

/**
 * Peek at the highest risk post without removing it
 * @param heap Pointer to the heap
 * @return Pointer to highest risk post, or NULL if heap is empty
 */
post* max_heap_peek_max(const max_heap *heap);

/**
* Pop and return the riskiest post (for human review).
 * - Posts with risk=1 are never stored; only posts in the heap are returned.
 * @param heap Pointer to the heap
 * @param out Pointer to store the removed post (caller owns memory)
 * @return true if successful, false if heap is empty
 */
bool max_heap_pop_max(max_heap *heap, post *out);

/**
 * Peek at the lowest risk post without removing it
 * @param heap Pointer to the heap
 * @return Pointer to lowest risk post, or NULL if heap is empty
 */
post* max_heap_peek_min(const max_heap *heap);

/**
* Evict the lowest-risk post (used when heap is full).
 * @param heap Pointer to the heap
 * @param out Pointer to store the removed post (caller owns memory)
 * @return true if successful, false if heap is empty
 */
bool max_heap_evict_min(max_heap *heap, post *out);

/**
 * Check if heap is empty
 * @param heap Pointer to the heap
 * @return true if empty, false otherwise
 */
bool max_heap_is_empty(const max_heap *heap);

/**
 * Check if heap is full
 * @param heap Pointer to the heap
 * @return true if at capacity, false otherwise
 */
bool max_heap_is_full(const max_heap *heap);

/**
 * Get current number of posts in heap
 * @param heap Pointer to the heap
 * @return Number of posts currently stored
 */
int max_heap_size(const max_heap *heap);

#endif // MAX_HEAP_H