#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define HEAP_SIZE 0x8000

#define FLAG_USED 0x1

uint8_t heap[HEAP_SIZE];

void* mymalloc(uint32_t size);
void* mycalloc(uint32_t count, uint32_t size);
void  myfree(void*);

void heap_dump();

int main() {
  memset(heap, 0, HEAP_SIZE);
  *(uint32_t*)(heap) = HEAP_SIZE - 5;
  *(uint32_t*)(heap + 4) = 0;

  void* x = mymalloc(0x5);
  *(uint32_t*)x = 0x56f39b1a;
  *(uint8_t*)(x+4) = 0x44;

  void* y = mymalloc(0x4);
  *(uint32_t*)y = 0x13f4b5a3;

  void* z = mymalloc(0x6);
  *(uint16_t*)z = 0x13f4;
  *(uint16_t*)(z + 1) = 0x2405;

  myfree(y);
  y = mymalloc(0x5);
  *(uint32_t*)y = 0x8523fd13;

  void* d = mymalloc(0x4);
  *(uint32_t*)d = 0xdddddddd;

  // NOTE:
  // in the current state, allocation y ends up overwriting memory in z
  // due to not checking if there is enough space for y in the newly freed region
  // it was 4 bytes long, but we ask for 10 bytes. 10 bytes cant fit in 4 bytes

  heap_dump();
}

void* mymalloc(uint32_t size) {
  printf("=========== mymalloc ===========\n");
  int offset = 0;
  // uint32_t stride = ((uint32_t*)(offset + heap))[0];
  // uint8_t  flags  = ((uint8_t* )(offset + heap + 4))[0];

  // while we see regions that are marked as used, go
  // further into the heap
  // skip regions that are too small as well
  while (offset < HEAP_SIZE) {
    uint32_t stride = ((uint32_t*)(offset + heap))[0];
    uint8_t flags   = ((uint8_t* )(offset + heap + 4))[0];

    if ((flags & FLAG_USED) == FLAG_USED) {
      offset += 5 + stride;
      continue;
    }
    if (stride < size) {
      offset += 5 + stride;
      continue;
    }
    break;
  }

  if (offset >= HEAP_SIZE) return NULL;
  // when we finally find an unused region
  // mark it as used, and fill in the size
  uint32_t oldsize = *(uint32_t*)(heap + offset);
  *(uint32_t*)(heap + offset) = size;
  *(uint8_t*)(heap + offset + 4) |= FLAG_USED;

  uint32_t newoff = offset + 5 + size;
  uint32_t newsize = oldsize - size - 5;
  if (newoff + 5 <= HEAP_SIZE) {
    *(uint32_t*)(heap + newoff) = newsize;
    *(uint8_t*)(heap + newoff + 4) = 0;
  }

  return heap + offset + 5;
}

void* mycalloc(uint32_t count, uint32_t size) {
  void* addr = mymalloc(count * size);
  memset(addr, 0, size);
  return addr;
}

void myfree(void* ptr) {
  int heap_index = ptr - (void*)heap - 5; // does this work??
  if (heap_index < 0 || heap_index > HEAP_SIZE)
    return;

  uint32_t stride = *(uint32_t*)(heap + heap_index);
  uint8_t* flags  = (uint8_t*)(heap + heap_index + 4);
  *flags = 0x0;
}

void heap_dump() {
  FILE* f = fopen("heap_dump.bin", "wb");
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
