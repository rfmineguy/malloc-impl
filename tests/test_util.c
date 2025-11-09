#include "test_util.h"
#include <stdlib.h>

void test_util_shuffle_array(test_util_allocation array[], int count) {
  for (int i = 0; i < count; i++) {
    int i = rand() % count;
    int j = rand() % count;

    test_util_allocation a = array[i];
    array[i] = array[j];
    array[j] = a;
  }
}
