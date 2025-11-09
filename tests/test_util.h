#ifndef TEST_UTIL_H
#define TEST_UTIL_H

typedef struct {
  int size;
  void *ptr;
} test_util_allocation;

void test_util_shuffle_array(test_util_allocation array[], int count);

#endif
