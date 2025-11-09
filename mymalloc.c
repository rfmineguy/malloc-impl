#include "mymalloc.h"
#include <string.h>
#include <stdio.h>

static uint8_t heap[HEAP_SIZE];
void heap_init() {
  memset(heap, 0, HEAP_SIZE);
  *(uint32_t*)(heap) = HEAP_SIZE - 9;
  *(uint32_t*)(heap + 4) = 0;
  *(uint32_t*)(heap + HEAP_SIZE - 5) = HEAP_SIZE - 9;
}

void heap_dump(const char* fn) {
  FILE* f = fopen(fn, "wb");
  if (!f) {
    fprintf(stderr, "Failed to open file\n");
    return;
  }
  
  size_t bytes = fwrite(heap, sizeof(uint8_t), HEAP_SIZE, f);
  printf("Wrote [%zu/%d] bytes\n", bytes, HEAP_SIZE);
  if (bytes != HEAP_SIZE) {
    fprintf(stderr, "Incomplete write [%zu/%d]\n", bytes, HEAP_SIZE);
  }

  fclose(f);
}
