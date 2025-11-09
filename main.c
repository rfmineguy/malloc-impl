#include "mymalloc.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#define TEST_RNG_SEEDS_COUNT 5

int test_rng_seeds[TEST_RNG_SEEDS_COUNT] = {
  0x8247134,
  0x13497ab,
  0xbfd31f4,
  0xca518f9,
  0x4b82f5b
};

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
  char output_file[100] = {0};
  for (int i = 0; i < TEST_RNG_SEEDS_COUNT; i++) {
    srand(test_rng_seeds[i]);
    snprintf(output_file, 100, "heap_%d.dump", i);
    for (int j = 0; j < 0xffff; j++) {
      test_malloc_free();
    }
    heap_dump(output_file);
  }
}
