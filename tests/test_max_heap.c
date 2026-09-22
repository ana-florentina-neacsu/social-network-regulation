#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>

#include "../include/max_heap.h"
#include "../include/post.h"

//=============================================================================
// HEAP CREATION & DESTRUCTION TESTS
//=============================================================================

START_TEST(test_create_heap_valid)
{
    // TODO: Test creating heap with valid capacity
    // - Check heap is not NULL
    // - Check initial size is 0
    // - Check capacity is set correctly
    // - Check heap is empty
    // - Check heap is not full
}
END_TEST

START_TEST(test_create_heap_invalid)
{
    // TODO: Test creating heap with invalid capacity
    // - Test with capacity <= 0
    // - Check that NULL is returned
}
END_TEST

//=============================================================================
// INSERTION TESTS
//=============================================================================

START_TEST(test_insert_single_post)
{
    // TODO: Test inserting a single post
    // - Create heap
    // - Insert one post with risk 0.5
    // - Check size is 1
    // - Check peek_max returns the inserted post
    // - Check heap is not empty
}
END_TEST

//=============================================================================
// PEEK & POP TESTS
//=============================================================================

START_TEST(test_peek_max_returns_highest_risk)
{
    // TODO: Test that peek_max returns highest risk post
    // - Insert posts with risks: 0.3, 0.8, 0.5
    // - Check peek_max returns the 0.8 post
    // - Check size unchanged after peek
}
END_TEST

START_TEST(test_pop_max_removes_highest_risk)
{
    // TODO: Test that pop_max removes and returns highest risk
    // - Insert posts with risks: 0.4, 0.9, 0.6
    // - Pop max, check it's the 0.9 post
    // - Check size decreased by 1
    // - Peek max again, check it's now 0.6
}
END_TEST

//=============================================================================
// MIN OPERATIONS TESTS
//=============================================================================

START_TEST(test_peek_min_returns_lowest_risk)
{
    // TODO: Test that peek_min returns lowest risk post
    // - Insert posts with risks: 0.7, 0.3, 0.5
    // - Check peek_min returns the 0.3 post
    // - Check size unchanged after peek
}
END_TEST

START_TEST(test_evict_min_removes_lowest_risk)
{
    // TODO: Test that evict_min removes lowest risk post
    // - Insert posts with risks: 0.8, 0.3, 0.6
    // - Evict min, check it's the 0.3 post
    // - Check size decreased by 1
    // - Peek min again, check it's now 0.6
}
END_TEST

//=============================================================================
// HEAP PROPERTY & EDGE CASES
//=============================================================================

START_TEST(test_heap_maintains_max_property)
{
    // TODO: Test that max-heap property is maintained after operations
    // - Insert random sequence of posts
    // - After each insert, verify peek_max is actually maximum
    // - Pop some posts
    // - Verify heap property still holds
}
END_TEST

START_TEST(test_heap_at_capacity_eviction)
{
    // TODO: Test heap behavior when at full capacity
    // - Create heap with capacity 5
    // - Fill with posts: risks 0.3, 0.5, 0.7, 0.8, 0.9
    // - Verify heap is full
    // - Insert post with risk 0.6 (higher than min 0.3)
    // - Verify min post (0.3) was evicted
    // - Verify new post (0.6) was inserted
    // - Verify size remains 5
    // - Insert post with risk 0.2 (lower than current min)
    // - Verify post was rejected (not inserted)
    // - Verify size still 5
}
END_TEST

START_TEST(test_posts_with_same_risk_level)
{
    // TODO: Test handling multiple posts with identical risk levels
    // - Create heap
    // - Insert 3 posts all with risk 0.5
    // - Insert 2 posts all with risk 0.7
    // - Insert 2 posts all with risk 0.3
    // - Verify total size is 7
    // - Verify peek_max returns one of the 0.7 posts
    // - Pop max twice, verify both are 0.7 risk posts
    // - Verify size is now 5
    // - Verify peek_min returns one of the 0.3 posts
    // - Evict min twice, verify both are 0.3 risk posts
    // - Verify size is now 3 (all 0.5 posts remaining)
}
END_TEST

//=============================================================================
// TEST SUITE SETUP
//=============================================================================

Suite *max_heap_suite(void)
{
    Suite *s;
    TCase *tc_creation;
    TCase *tc_insertion;
    TCase *tc_peek_pop;
    TCase *tc_min_ops;
    TCase *tc_properties;

    s = suite_create("MaxHeap");

    // Test case for creation and destruction
    tc_creation = tcase_create("Creation");
    tcase_add_test(tc_creation, test_create_heap_valid);
    tcase_add_test(tc_creation, test_create_heap_invalid);
    suite_add_tcase(s, tc_creation);

    // Test case for insertion operations
    tc_insertion = tcase_create("Insertion");
    tcase_add_test(tc_insertion, test_insert_single_post);
    suite_add_tcase(s, tc_insertion);

    // Test case for peek and pop operations
    tc_peek_pop = tcase_create("PeekAndPop");
    tcase_add_test(tc_peek_pop, test_peek_max_returns_highest_risk);
    tcase_add_test(tc_peek_pop, test_pop_max_removes_highest_risk);
    suite_add_tcase(s, tc_peek_pop);

    // Test case for minimum operations
    tc_min_ops = tcase_create("MinOperations");
    tcase_add_test(tc_min_ops, test_peek_min_returns_lowest_risk);
    tcase_add_test(tc_min_ops, test_evict_min_removes_lowest_risk);
    suite_add_tcase(s, tc_min_ops);

    // Test case for heap properties and edge cases
    tc_properties = tcase_create("Properties");
    tcase_add_test(tc_properties, test_heap_maintains_max_property);
    tcase_add_test(tc_properties, test_heap_at_capacity_eviction);
    tcase_add_test(tc_properties, test_posts_with_same_risk_level);
    suite_add_tcase(s, tc_properties);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s = max_heap_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}