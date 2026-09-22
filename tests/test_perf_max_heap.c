#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <check.h>
#include <stdio.h>

#include "../include/max_heap.h"
#include "../include/post.h"

//=============================================================================
// PERFORMANCE TEST CONSTANTS
//=============================================================================

#define BILLION 1000000000
#define TEN_MILLION 10000000
#define ONE_MILLION 1000000
#define STRESS_TEST_DURATION 600 // 10 minutes in seconds
#define INSERT TEST DURATION 0.05
//=============================================================================
// HELPER FUNCTIONS
//=============================================================================

// Generate a random risk value between 0 and 1 (exclusive of 1)
static float random_risk()
{
    return (float)rand() / (float)RAND_MAX * 0.99;
}

// Create a test post with given id and risk
static post *create_test_post(int id, float risk)
{
    post *p = malloc(sizeof(post));
    p->id = id;
    p->risk = risk;
    snprintf(p->content, MAX_CONTENT_LENGTH, "Test post %d", id);
    return p;
}

// Measure time in seconds (double precision)
static double get_time_seconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

//=============================================================================
// TEST 1: Insertion of a post with 1 Billion posts already stored
//=============================================================================

START_TEST(test_insertion_1billion_posts)
{
    // TODO: Test insertion performance with 1 billion posts
    // - Create heap with capacity for 1 billion posts
    max_heap *new_heap = max_heap_new(ONE_MILLION);
    int i;
    // - Fill heap with 1 billion posts
    for (i = 0; i < ONE_MILLION; i++)
    {
        post *new_post = create_test_post(i, random_risk());
        max_heap_insert(new_heap, new_post);
    }
    // - Measure time to insert one additional post
    double time_init = get_time_seconds();
    post *new_post = create_test_post(i, random_risk());
    double time_fin = get_time_seconds();
    // - Verify insertion completes in reasonable time
    ck_assert_msg((time_fin - time_init) < INSERT_TEST_DURATION, "too long, test takes %lf seconds", (time_fin - time_init));
    printf("============================== TEST 1 PASSED ================================")
    // - Expected: O(log n) behavior, should complete in microseconds
}
END_TEST

//=============================================================================
// TEST 2: Insertion of 10 posts up to 1 Billion (complexity curve)
//=============================================================================

START_TEST(test_insertion_complexity_growth)
{
    // TODO: Test insertion complexity growth from 10 to 1 billion posts
    // - Test insertion at various scales: 10, 100, 1K, 10K, 100K, 1M, 10M, 100M, 1B
    // - For each scale, measure average insertion time
    // - Plot/log the results to verify O(log n) growth
    // - Compare timing ratios against theoretical log(n) ratios
    // - Example: time(1M) / time(1K) should be ≈ log(1M)/log(1K) ≈ 2
}
END_TEST

//=============================================================================
// TEST 3: Eviction of min post with 1 Billion posts already stored
//=============================================================================

START_TEST(test_eviction_1billion_posts)
{
    // TODO: Test eviction performance with 1 billion posts
    // - Create heap with capacity for 1 billion posts
    // - Fill heap with 1 billion posts
    // - Measure time to evict the minimum post
    // - Verify eviction completes efficiently
    // - Expected: Should complete quickly with O(1) min access + O(n) rescan
}
END_TEST

//=============================================================================
// TEST 4: Successive evictions from 10 posts up to 1 Billion (complexity curve)
//=============================================================================

START_TEST(test_eviction_complexity_growth)
{
    // TODO: Test eviction complexity growth from 10 to 1 billion posts
    // - Test eviction at various scales: 10, 100, 1K, 10K, 100K, 1M, 10M, 100M, 1B
    // - For each scale, measure average eviction time
    // - Plot/log the results to analyze complexity
    // - With optimization, should show improved performance vs O(n) linear scan
    // - Verify that min_node_index optimization provides benefits
}
END_TEST

//=============================================================================
// TEST 5: Memory taken by structure after insertion of 1 Billion posts
//=============================================================================

START_TEST(test_memory_usage_1billion_posts)
{
    // TODO: Test memory efficiency with 1 billion posts
    // - Create heap with capacity for 1 billion posts
    // - Fill heap with 1 billion posts (varying risk levels)
    // - Calculate total memory used:
    //   * sizeof(max_heap)
    //   * nodes array: size * sizeof(heap_node*)
    //   * each heap_node: sizeof(heap_node)
    //   * posts arrays in each node: post_count * sizeof(post*)
    //   * actual posts: total_posts * sizeof(post)
    // - Verify memory usage is reasonable and matches expectations
    // - Check for memory efficiency (no excessive overhead)
}
END_TEST

//=============================================================================
// TEST 6: Stress testing the algorithm for 10 minutes (real-life simulation)
//=============================================================================

START_TEST(test_stress_reallife_10min)
{
    // Real-world simulation: 2 billion posts/day = ~1,388,889 posts/minute
    // Running for 10 minutes = ~13,888,890 total posts to process

    const int HEAP_CAPACITY = 1000000;   // 1M posts in review queue
    const double TARGET_DURATION = 10.0; // 10 minutes in seconds (10 seconds only for dev period)

    max_heap *heap = max_heap_new(HEAP_CAPACITY);
    ck_assert_ptr_nonnull(heap);

    srand(time(NULL));

    // Metrics tracking
    long total_insertions = 0;
    long pops = 0;
    long evictions = 0;
    long posts_created = 0;

    double total_insert_time = 0.0;
    double total_pop_time = 0.0;
    double total_evict_time = 0.0;

    double start_time = get_time_seconds();
    double elapsed;

    // Run for 10 minutes
    while ((elapsed = get_time_seconds() - start_time) < TARGET_DURATION)
    {
        // Weighted operation selection (realistic moderation workflow)
        // 70% insertions, 20% pops (human review), 10% manual evictions
        int op = rand() % 100;

        if (op < 70)
        { // 70% - Insert new post
            post *p = create_test_post(posts_created++, random_risk());

            double t1 = get_time_seconds();
            bool inserted = max_heap_insert(heap, p);
            double t2 = get_time_seconds();

            total_insert_time += (t2 - t1);
            total_insertions++;

            if (!inserted)
            {
                free(p); // Post was rejected or deleted (risk=1)
            }
        }
        else if (op < 90)
        { // 20% - Pop max (human reviewer processes post)
            if (!max_heap_is_empty(heap))
            {
                post out;
                double t1 = get_time_seconds();
                bool success = max_heap_pop_max(heap, &out);
                double t2 = get_time_seconds();

                total_pop_time += (t2 - t1);
                if (success)
                    pops++;
            }
        }
        else
        { // 10% - Evict min (remove low-risk posts)
            if (!max_heap_is_empty(heap))
            {
                post out;
                double t1 = get_time_seconds();
                bool success = max_heap_evict_min(heap, &out);
                double t2 = get_time_seconds();

                total_evict_time += (t2 - t1);
                if (success)
                    evictions++;
            }
        }
    }

    // Verify heap integrity
    ck_assert_int_ge(max_heap_size(heap), 0);
    ck_assert_int_le(max_heap_size(heap), HEAP_CAPACITY);

    // Verify no performance degradation (all operations should remain fast)
    double avg_insert_time = total_insert_time / total_insertions;
    ck_assert_msg(avg_insert_time < 0.001,
                  "Insert time too slow: %.6f seconds", avg_insert_time);

    // Verify heap maintains max property
    if (!max_heap_is_empty(heap))
    {
        post *max_post = max_heap_peek_max(heap);
        ck_assert_ptr_nonnull(max_post);
        post *min_post = max_heap_peek_min(heap);
        ck_assert_ptr_nonnull(min_post);
        ck_assert_msg(max_post->risk >= min_post->risk,
                      "Heap property violated: max=%.2f < min=%.2f",
                      max_post->risk, min_post->risk);
    }

    max_heap_free(heap);
}
END_TEST

//=============================================================================
// TEST 7: Realloc stress testing (dynamic array growth overhead)
//=============================================================================

START_TEST(test_realloc_stress_overhead)
{
    // TODO: Test memory reallocation overhead
    // - Create heap with small initial capacity
    // - Create 5 nodes with different risk levels
    // - Fill each node to its post_capacity (initially 2 posts per node)
    // - Insert one additional post to each node to trigger realloc
    // - Measure time taken for each realloc operation
    // - Verify realloc operations complete efficiently
    // - Test with varying node sizes to measure realloc overhead at different scales
    // - Expected: realloc overhead should be minimal but measurable
    // - Verify geometric growth (capacity doubling) works correctly
}
END_TEST

//=============================================================================
// TEST SUITE SETUP
//=============================================================================

Suite *max_heap_performance_suite(void)
{
    Suite *s;
    TCase *tc_insertion_perf;
    TCase *tc_eviction_perf;
    TCase *tc_memory;
    TCase *tc_stress;
    TCase *tc_realloc;

    s = suite_create("MaxHeap_Performance");

    // Test case for insertion performance
    tc_insertion_perf = tcase_create("InsertionPerformance");
    tcase_set_timeout(tc_insertion_perf, 3600); // 1 hour timeout for large tests
    tcase_add_test(tc_insertion_perf, test_insertion_1billion_posts);
    tcase_add_test(tc_insertion_perf, test_insertion_complexity_growth);
    suite_add_tcase(s, tc_insertion_perf);

    // Test case for eviction performance
    tc_eviction_perf = tcase_create("EvictionPerformance");
    tcase_set_timeout(tc_eviction_perf, 3600); // 1 hour timeout for large tests
    tcase_add_test(tc_eviction_perf, test_eviction_1billion_posts);
    tcase_add_test(tc_eviction_perf, test_eviction_complexity_growth);
    suite_add_tcase(s, tc_eviction_perf);

    // Test case for memory usage
    tc_memory = tcase_create("MemoryUsage");
    tcase_set_timeout(tc_memory, 3600); // 1 hour timeout for large tests
    tcase_add_test(tc_memory, test_memory_usage_1billion_posts);
    suite_add_tcase(s, tc_memory);

    // Test case for stress testing (10 minutes)
    tc_stress = tcase_create("StressTesting");
    tcase_set_timeout(tc_stress, 720); // 12 minutes timeout (10 min test + buffer)
    tcase_add_test(tc_stress, test_stress_reallife_10min);
    suite_add_tcase(s, tc_stress);

    // Test case for realloc overhead
    tc_realloc = tcase_create("ReallocStress");
    tcase_add_test(tc_realloc, test_realloc_stress_overhead);
    suite_add_tcase(s, tc_realloc);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s = max_heap_performance_suite();
    SRunner *sr = srunner_create(s);

    // Run tests with verbose output
    srunner_run_all(sr, CK_VERBOSE);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}