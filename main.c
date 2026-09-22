#include <stdio.h>
#include <string.h>
#include "include/max_heap.h"
#include "include/post.h"

int main() {
    // Create heap with max 5 posts
    max_heap *heap = max_heap_new(5);

    // Create some test posts
    post a = {0}; a.id = 1; a.risk = 0.8; strcpy(a.content, "Post A");
    post b = {0}; b.id = 2; b.risk = 0.6; strcpy(b.content, "Post B");
    post c = {0}; c.id = 3; c.risk = 0.55; strcpy(c.content, "Post C");
    post d = {0}; d.id = 4; d.risk = 0.75; strcpy(d.content, "Post D");
    post e = {0}; e.id = 5; e.risk = 1.0; strcpy(e.content, "Post E - Risk 1"); // should be deleted
    post f = {0}; f.id = 6; f.risk = 0.9; strcpy(f.content, "Post F");
    post g = {0}; g.id = 7; g.risk = 0.65; strcpy(g.content, "Post G"); // new post
    post h = {0}; h.id = 8; h.risk = 0.7; strcpy(h.content, "Post H");  // new post

    // Insert posts
    max_heap_insert(heap, &a);
    max_heap_insert(heap, &b);
    max_heap_insert(heap, &c);

    printf("Heap size after 3 inserts: %d\n", max_heap_size(heap)); // expect 3

    max_heap_insert(heap, &d); // may trigger eviction if full
    max_heap_insert(heap, &e); // risk=1, should not be inserted
    max_heap_insert(heap, &f); // triggers eviction if full
    max_heap_insert(heap, &g); // triggers eviction if full
    max_heap_insert(heap, &h); // triggers eviction if full

    printf("Heap size after all inserts: %d\n", max_heap_size(heap));

    // Peek max
    post *max_post = max_heap_peek_max(heap);
    if (max_post)
        printf("Riskiest post: %s (risk %.2f)\n", max_post->content, max_post->risk);

    // Pop all posts in order of risk
    post out;
    printf("\nProcessing posts (riskiest first):\n");
    while (!max_heap_is_empty(heap)) {
        max_heap_pop_max(heap, &out);
        printf("Post ID %d: %s (risk %.2f)\n", out.id, out.content, out.risk);
    }

    max_heap_free(heap);
    return 0;
}
