#include "mymalloc.h"
#include <string.h>
#include <stdio.h>

static uint8_t heap[HEAP_SIZE];

void* mymalloc(uint32_t size) {
  int offset = 0;

  // while we see regions that are marked as used, go
  // further into the heap
  // skip regions that are too small as well
  while (offset < HEAP_SIZE) {
    uint32_t stride = ((uint32_t*)(offset + heap))[0];
    uint8_t  flags  = ((uint8_t* )(offset + heap + 4))[0];

    // Initialize the heap in this case
    if (stride == 0) {
      stride = HEAP_SIZE - 9;
      *(uint32_t*)(heap + offset) = stride;
      *(uint8_t*)(heap + offset + 4) = 0x0;
      *(uint32_t*)(heap + offset + stride + 5) = stride;
    }

    if ((flags & FLAG_USED) == FLAG_USED || stride < size) {
      offset += 4 + 1 + stride + 4; // skip stride, flags, bytes, size
      continue;
    }
    break;
  }

  // If offset is passed heap we didnt find a region
  // big enough
  if (offset >= HEAP_SIZE) return NULL;

  // when we finally find an unused region
  // fill in the size, and mark it as used
  uint32_t oldsize = *(uint32_t*)(heap + offset);
  *(uint32_t*)(heap + offset) = size;
  *(uint8_t*)(heap + offset + 4) |= FLAG_USED;

  // fill in the trailing size as well
  *(uint32_t*)(heap + offset + 5 + size) = size;

  uint32_t newoff = offset + 5 + size + 4;
  uint32_t newsize = oldsize > (size + 9) ? oldsize - (size + 9) : 0;

  if (newsize > 0 && newoff + 9 <= HEAP_SIZE) {
    *(uint32_t*)(heap + newoff) = newsize;
    *(uint32_t*)(heap + newoff + 5 + newsize) = newsize;
    *(uint8_t*)(heap + newoff + 4) = 0;
  }

  return heap + offset + 5;
}

void* mycalloc(uint32_t count, uint32_t size) {
  void* addr = mymalloc(count * size);
  memset(addr, 0, size);
  return addr;
}
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
