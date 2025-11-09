#include "munit.h"
#define TEST
#include "../mymalloc.h"

MunitResult test_heap_init(const MunitParameter params[], void* user_data_or_fixture);
MunitResult test_malloc_single(const MunitParameter params[], void* user_data_or_fixture);

MunitTest tests[] = {
  { "/heap_init", test_heap_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { "/malloc_single", test_malloc_single, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
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
  return MUNIT_SKIP;
}
