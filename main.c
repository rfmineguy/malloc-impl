#include "mymalloc.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

typedef struct {
  int size;
  void *ptr;
} test_allocation;

void shuffle_array(test_allocation array[], size_t count) {
  for (int i = 0; i < count; i++) {
    int i = rand() % count;
    int j = rand() % count;

    test_allocation a = array[i];
    array[i] = array[j];
    array[j] = a;
  }
}

void test_malloc_free() {
  test_allocation allocations[50] = {0};
  heap_init();

  for (int i = 0; i < 50; i++) {
    while (allocations[i].size == 0)
      allocations[i].size = (rand() % 50);

    allocations[i].ptr = mymalloc(allocations[i].size);
  }

  for (int i = 0; i < 50; i++) {
    myfree(allocations[i].ptr);
  }
}

int main() {
  srand(0x453975);

  for (int i = 0; i < 0xffff; i++) {
    test_malloc_free();
  }

  heap_dump("heap.dump");
}
