#include "munit.h"
#define TEST
#include "../mymalloc.h"

MunitResult test_heap_init(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_single(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_multiple(const MunitParameter params[], void* user_data_or_fixture);

MunitTest tests[] = {
  { "/heap_init", test_heap_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/malloc_single", test_malloc_single, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/malloc_multiple", test_malloc_multiple, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

MunitSuite suite = {
  "malloc_tests", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char** argv) {
  return munit_suite_main(&suite, NULL, argc, argv);
}

MunitResult test_heap_init(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  uint8_t* heap = heap_test_get();

  // 1. ensure the entire heap is zeroed other than the:
  //       header
  //       trailer
  //       flags
  for (int i = 5; i < HEAP_SIZE - 5; i++)
    munit_assert_int(heap[i], ==, 0);

  // 2. check the header is appropriate for the HEAP_SIZE
  munit_assert_int(*(uint32_t*)heap, ==, HEAP_SIZE - 9);

  // 3. check the flags are not set
  munit_assert_int(*(uint8_t*)(heap + 4), ==, 0x0);

  // 4. check the trailer is equal to the header
  munit_assert_int(*(uint32_t*)(heap + HEAP_SIZE - 5), ==, HEAP_SIZE - 9);
  return MUNIT_OK;
}

MunitResult test_malloc_single(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  void* p = mymalloc(100);

  // check the header of this allocation
  munit_assert_int(*(uint32_t*)(p - 5), ==, 100);
  munit_assert_int(*(uint8_t*)(p - 1), ==, 0x1);

  // check the trailer of this allocation
  munit_assert_int(*(uint32_t*)(p + 100), ==, 100);

  // check the header of the region after
  munit_assert_int(*(uint32_t*)(p + 100 + 4), ==, HEAP_SIZE - 9 - 5 - 4 - 100);

  // check the trailer of the region after
  uint32_t header = *(uint32_t*)(p + 100 + 4);
  munit_assert_int(header, ==, HEAP_SIZE - 9 - 5 - 4 - 100);

  return MUNIT_OK;
}

MunitResult test_malloc_multiple(const MunitParameter params[], void* user_data_or_fixture) {
  heap_init();
  void* p = mymalloc(100);
  // check the header of this allocation
  munit_assert_int(*(uint32_t*)(p - 5), ==, 100);
  munit_assert_int(*(uint8_t*)(p - 1), ==, 0x1);

  // check the trailer of this allocation
  munit_assert_int(*(uint32_t*)(p + 100), ==, 100);


  void* p2 = mymalloc(20);
  // check the header of this allocation
  munit_assert_int(*(uint32_t*)(p2 - 5), ==, 20);
  munit_assert_int(*(uint8_t*)(p2 - 1), ==, 0x1);

  // check the trailer of this allocation
  munit_assert_int(*(uint32_t*)(p2 + 20), ==, 20);

  // check the header of the region after
  munit_assert_int(*(uint32_t*)(p2 + 20 + 4), ==, HEAP_SIZE - 9 - 9 - 5 - 4 - 100 - 20);

  // check the trailer of the region after
  uint32_t header = *(uint32_t*)(p2 + 20 + 4);
  munit_assert_int(header, ==, HEAP_SIZE - 9 - 9 - 5 - 4 - 100 - 20);

  return MUNIT_OK;
}
