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
  return MUNIT_OK;
}

MunitResult test_malloc_single(const MunitParameter params[], void* user_data_or_fixture) {
  return MUNIT_SKIP;
}
