#include "munit.h"
#include <stdbool.h>
#include "test_util.h"
#ifndef TEST
#define TEST
#endif
#include "../mymalloc.h"

MunitResult test_heap_init(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_single(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_multiple(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_too_big(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_free_single(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_free_multiple(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_free_multiple_interleaved(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_free_many_in_order(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_free_many_out_of_order(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_multiple_too_big_trigger_resize(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_multiple_too_big_passed_max(const MunitParameter params[], void* user_data_or_fixture);

MunitTest tests[] = {
  { "/heap_init", test_heap_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/malloc_single", test_malloc_single, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/malloc_multiple", test_malloc_multiple, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_malloc_too_big", test_malloc_too_big, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/malloc_free_single", test_malloc_free_single, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/malloc_free_multiple", test_malloc_free_multiple, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_malloc_free_multiple_interleaved", test_malloc_free_multiple_interleaved, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_malloc_free_many_in_order", test_malloc_free_many_in_order, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_malloc_free_many_out_of_order", test_malloc_free_many_out_of_order, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_malloc_multiple_too_big_trigger_resize", test_malloc_multiple_too_big_trigger_resize, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/test_malloc_multiple_too_big_passed_max", test_malloc_multiple_too_big_passed_max, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

MunitSuite suite = {
  "malloc_tests", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char** argv) {
  return munit_suite_main(&suite, NULL, argc, argv);
}

/*
 * What are we testing?
 *   - we want to make sure that the heap starts in a valid state
 * 
 * Covers:
 *   - heap_init()
 *   - heap_test_get()
 *
 * Expected:
 *   - heap_init should provide the minimal metadata required for creating a single
 *        large free region of memory
 */
MunitResult test_heap_init(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  uint8_t* heap = heap_test_get();
  uint32_t size = heap_test_get_current_size();

  // 1. ensure the entire heap is zeroed other than the:
  //       header
  //       trailer
  //       flags
  for (int i = 5; i < size - 5; i++)
    munit_assert_int(heap[i], ==, 0);

  // 2. check the header is appropriate for the HEAP_SIZE
  munit_assert_int(*(uint32_t*)heap, ==, size - 9);

  // 3. check the flags are not set
  munit_assert_int(*(uint8_t*)(heap + 4), ==, 0x0);

  // 4. check the trailer is equal to the header
  munit_assert_int(*(uint32_t*)(heap + size - 4), ==, size - 9);
  return MUNIT_OK;
}

/*
 * What are we testing?
 *   - we want to make sure the most basic task (allocation) can be done once without problems
 *   - this is a standalone test that tests heap region splitting
 *
 * Covers:
 *   - mymalloc()
 *
 * Expected:
 *   - mymalloc should not break the heap metadata
 *   - mymalloc should modify heap metadata in a way that keeps the heap traversable
 *   - there should be two valid regions (p, rest)
 */
MunitResult test_malloc_single(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  uint8_t* p = mymalloc(100);
  uint32_t size = heap_test_get_current_size();

  // check the header of this allocation
  munit_assert_int(*(uint32_t*)(p - 5), ==, 100);
  munit_assert_int(*(uint8_t*)(p - 1), ==, 0x1);

  // check the trailer of this allocation
  munit_assert_int(*(uint32_t*)(p + 100), ==, 100);

  // check the header of the region after
  munit_assert_int(*(uint32_t*)(p + 100 + 4), ==, size - 9 - 5 - 4 - 100);

  // check the trailer of the region after
  uint32_t header = *(uint32_t*)(p + 100 + 4);
  munit_assert_int(header, ==, size - 9 - 5 - 4 - 100);

  munit_assert_int(heap_check_validity(), ==, 0);

  return MUNIT_OK;
}

/*
 * What are we testing?
 *   - we want to make sure that subsequent malloc calls get unique, nonoverlapping regions of memory
 *
 * Covers:
 *   - mymalloc()
 *
 * Expected:
 *   - mymalloc should not break the heap metadata
 *   - mymalloc should modify heap metadata in a way that keeps the heap traversable
 *   - there should be three valid regions (p, p2, rest)
 */
MunitResult test_malloc_multiple(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  uint8_t* p = mymalloc(100);
  uint32_t size = heap_test_get_current_size();
  // check the header of this allocation
  munit_assert_int(*(uint32_t*)(p - 5), ==, 100);
  munit_assert_int(*(uint8_t*)(p - 1), ==, 0x1);

  // check the trailer of this allocation
  munit_assert_int(*(uint32_t*)(p + 100), ==, 100);


  uint8_t* p2 = mymalloc(20);
  // check the header of this allocation
  munit_assert_int(*(uint32_t*)(p2 - 5), ==, 20);
  munit_assert_int(*(uint8_t*)(p2 - 1), ==, 0x1);

  // check the trailer of this allocation
  munit_assert_int(*(uint32_t*)(p2 + 20), ==, 20);

  // check the header of the region after
  munit_assert_int(*(uint32_t*)(p2 + 20 + 4), ==, size - 9 - 9 - 5 - 4 - 100 - 20);

  // check the trailer of the region after
  uint32_t header = *(uint32_t*)(p2 + 20 + 4);
  munit_assert_int(header, ==, size - 9 - 9 - 5 - 4 - 100 - 20);

  munit_assert_int(heap_check_validity(), ==, 0);

  return MUNIT_OK;
}

MunitResult test_malloc_too_big(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  uint32_t size = heap_test_get_current_size();
  void* p = mymalloc(size);

  // This result of this malloc should succeed due to heap resizing
  munit_assert_not_null(p);
  munit_assert_int(heap_check_validity(), ==, 0);

  return MUNIT_OK;
}

/*
 * What are we testing?
 *   - we want to test malloc and free in conjunction with each other
 *
 * Covers:
 *   - mymalloc()
 *   - myfree()
 *
 * Expected:
 *   - the resultant heap should be indistinguishable from a freshly created heap
 *   - this would demonstrate that region joining is functional for a single region
 *      allocation
 */
MunitResult test_malloc_free_single(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  void* p = mymalloc(100);
  myfree(p);
  munit_assert_int(heap_check_validity(), ==, 0);

  return MUNIT_OK;
}

/*
 * What are we testing?
 *   - we want to now test multiple mallocs and frees in conjunction with each other
 *
 * Covers:
 *   - mymalloc()
 *   - myfree()
 *
 * Expected:
 *   - given that we have an equal number of frees and mallocs we should expect the resultant heap
 *      to be indistinguishable from a freshly created heap
 */
MunitResult test_malloc_free_multiple(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  void* p = mymalloc(100);
  void* p2 = mymalloc(50);

  myfree(p);
  myfree(p2);
  munit_assert_int(heap_check_validity(), ==, 0);

  return MUNIT_OK;
}

/* * What are we testing?
 *   - we want to now test multiple mallocs and frees in conjunction with each other
 *   - this time however we free the first allocation before allocating the second
 *
 * Covers:
 *   - mymalloc()
 *   - myfree()
 *
 * Expected:
 *   - given that we have an equal number of frees and mallocs we should expect the resultant heap
 *      to be indistinguishable from a freshly created heap
 */
MunitResult test_malloc_free_multiple_interleaved(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  uint8_t* heap = heap_test_get();

  void* p = mymalloc(100);
  myfree(p);

  void* p2 = mymalloc(50);

  // check the header to see if it is 50 bytes long before freeing the allocation
  //   this means that the first 100 byte freed allocation is now being occupied by
  //   the new 50 byte one
  munit_assert_int(*(uint32_t*)heap, ==, 50);

  myfree(p2);
  munit_assert_int(heap_check_validity(), ==, 0);

  return MUNIT_OK;
}

/* * What are we testing?
 *   - we want to now test many random mallocs and frees in conjunction with each other
 *   - the frees will be freed in the same order they were allocated
 *
 * Covers:
 *   - mymalloc()
 *   - myfree()
 *
 * Expected:
 *   - given that we have an equal number of frees and mallocs we should expect the resultant heap
 *      to be indistinguishable from a freshly created heap
 */
MunitResult test_malloc_free_many_in_order(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  test_util_allocation allocations[50] = {0};

  for (int i = 0; i < 50; i++) {
    allocations[i].size = (munit_rand_int_range(10, 50));
    allocations[i].ptr = mymalloc(allocations[i].size);
  }

  for (int i = 0; i < 50; i++) {
    myfree(allocations[i].ptr);
  }
  munit_assert_int(heap_check_validity(), ==, 0);
  return MUNIT_OK;
}

/* * What are we testing?
 *   - we want to now test many random mallocs and frees in conjunction with each other
 *   - the frees will be freed in a random order to how were allocated
 *
 * Covers:
 *   - mymalloc()
 *   - myfree()
 *
 * Expected:
 *   - given that we have an equal number of frees and mallocs we should expect the resultant heap
 *      to be indistinguishable from a freshly created heap
 */
MunitResult test_malloc_free_many_out_of_order(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  test_util_allocation allocations[50] = {0};

  for (int i = 0; i < 50; i++) {
    allocations[i].size = (munit_rand_int_range(10, 50));
    allocations[i].ptr = mymalloc(allocations[i].size);
  }

  test_util_shuffle_array(allocations, 50);

  for (int i = 0; i < 50; i++) {
    myfree(allocations[i].ptr);
  }
  munit_assert_int(heap_check_validity(), ==, 0);
  return MUNIT_OK;
}

/* * What are we testing?
 *   - test what happens when you try to allocate a region that is too big to fit in the
 *       heap
 *   - additionally test to make sure that if you try to free a NULL pointer, nothing happens
 *
 * Covers:
 *   - mymalloc()
 *   - myfree()
 *
 * Expected:
 *   - the internal heap structure should not change in any way
 *   - the pointer returned from a failed allocation should be NULL
 */
MunitResult test_malloc_multiple_too_big_trigger_resize(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  void *a = mymalloc(0x400);
  munit_assert_not_null(a);
  void *b = mymalloc(0x400);
  munit_assert_not_null(b);
  void *c = mymalloc(0x400);
  munit_assert_not_null(c);
  void *d = mymalloc(0x400);

  // even though the heap starts with 0x1000 memory
  //   when the last malloc happens it gets resized
  munit_assert_not_null(d);
  myfree(d);
  d = mymalloc(0x30);
  munit_assert_not_null(d);


  munit_assert_int(heap_check_validity(), ==, 0);
  return MUNIT_OK;
}

/* * What are we testing?
 *   - test what happens when you try to allocate passed the max heap size
 *   - even though the heap can resize, it also has a max size where it will no longer resize
 *
 * Covers:
 *   - mymalloc()
 *   - myfree()
 *
 * Expected:
 *   - mymalloc() should return null for any allocations that can't fit even after being resized
 */
MunitResult test_malloc_multiple_too_big_passed_max(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  int allocation_size = 0x1000;
  int allocation_count = HEAP_MAX_SIZE / allocation_size;
  for (int i = 0; i < allocation_count - 1; i++) {
    munit_assert_int(heap_test_get_current_size(), ==, allocation_size * (i + 1));
    void* p = mymalloc(allocation_size);
    munit_assert_not_null(p);
  }
  munit_assert_int(heap_check_validity(), ==, 0);
  munit_assert_int(heap_test_get_current_size(), ==, HEAP_MAX_SIZE);

  void* p = mymalloc(allocation_size);
  munit_assert_null(p);

  return MUNIT_OK;
}
