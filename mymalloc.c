#include "mymalloc.h"
#include <string.h>
#include <stdio.h>

// For information about how heap resizing works, see:
//   https://github.com/rfmineguy/malloc-impl/issues/1
static uint8_t heap[HEAP_MAX_SIZE];

// This is a very sensitive value that should not be exposed outside
//   of this translation unit
static uint32_t heap_max_addr = HEAP_START_SIZE;

/*
 * @Internal
 * @Desc:
 *     Find the heap offset of the next free region of memory.
 * @Param:
 *     size - a size representing the minimum size region to search for
 * @Return:
 *     32 bit offset into the heap datastructure where the next free offset is
 * @Notes:
 *   - if there are no free blocks in the current heap, it will return a number
 *      larger than the @heap_max_addr
 *   - this function is exposed, but not intended to be used directly
 */
static uint32_t heap_find_next_free_offset(uint32_t size) {
  int offset = 0;

  // while we see regions that are marked as used, go
  // further into the heap
  // skip regions that are too small as well
  while (offset < heap_max_addr) {
    uint32_t stride = ((uint32_t*)(offset + heap))[0];
    uint8_t  flags  = ((uint8_t* )(offset + heap + 4))[0];

    // Initialize the heap in this case
    if (stride == 0) {
      stride = heap_max_addr - 9;
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
  return offset;
}

void* mymalloc(uint32_t size) {
  int offset = heap_find_next_free_offset(size);

  // If offset is passed heap we didnt find a region
  // big enough, so we should trigger a heap resize
  if (offset >= heap_max_addr && heap_max_addr + HEAP_RESIZE_AMT <= HEAP_MAX_SIZE) {
    // This is overly simplified due to being in a hosted environment
    //   In reality, we want to 
    //   1. allocate a new frame using the physical memory allocator
    //   2. page that new frame to be contiguous with the previous frame
    uint32_t old_max_addr = heap_max_addr;
    heap_max_addr += HEAP_RESIZE_AMT;
    uint8_t* last_footer = heap + old_max_addr - 4;
    uint32_t last_stride = *(uint32_t*)last_footer;

    uint8_t* last_header = last_footer - 5 - last_stride;
    uint8_t* last_flags = last_header + 4;

    if (!(*last_flags & FLAG_USED)) {
      uint32_t new_stride = last_stride + HEAP_RESIZE_AMT;
      *(uint32_t*)last_header = new_stride;
      *(uint32_t*)(heap + heap_max_addr - 4) = new_stride;
    }
    return mymalloc(size);
  }

  // If offset puts us passed the max of the heap, we failed to find a sufficiently
  // sized region even after resizing the heap
  if (offset >= HEAP_MAX_SIZE) return NULL;

  // when we finally find an unused region
  // fill in the size, and mark it as used
  uint32_t oldsize = *(uint32_t*)(heap + offset);
  *(uint32_t*)(heap + offset) = size;
  *(uint8_t*)(heap + offset + 4) |= FLAG_USED;

  // fill in the trailing size as well
  *(uint32_t*)(heap + offset + 5 + size) = size;

  uint32_t newoff = offset + 5 + size + 4;
  uint32_t newsize = oldsize > (size + 9) ? oldsize - (size + 9) : 0;

  if (newsize > 0 && newoff + 9 <= heap_max_addr) {
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

void myfree(void *ptr) {
  if (ptr == 0) return;
  uint8_t* p = (uint8_t*)ptr;
  *p = 0xff;
  if (p < heap || p > heap + heap_max_addr)
    return;

  uint32_t curr_stride = *(uint32_t*)(p - 5);
  uint8_t* curr_flags = p - 1;
  *curr_flags = 0;

  // look at the region left of this allocation
  // 1. if there is no allocation before this one, we dont have to merge it
  if (p - 5 < heap) {
    p += curr_stride + 9;
    goto right_merge;
  }

  // 2. check the previous allocation to see if its free
  uint32_t prev_stride = *(uint32_t*)(p - 5 - 4);
  uint8_t* prev_hdr   = p - 9 - (5 + prev_stride);
  if (prev_hdr < heap) {
    p += curr_stride + 9;
    goto right_merge;
  }
  uint8_t* prev_flags = prev_hdr + 4;

  uint32_t new_stride = prev_stride + curr_stride + 9;
  if ((*prev_flags & FLAG_USED) == 0) {
    *(uint32_t*)prev_hdr = new_stride;                      // set header
    *(uint32_t*)(prev_hdr + 5 + new_stride) = new_stride;   // set trailer
    *prev_flags = 0;                                        // set flags
    memset(prev_hdr + 5, 0x0, new_stride);
  }

  // move p to the start of the next block
  p = prev_hdr + 9 + new_stride + 5;

right_merge:
  *(uint32_t*)p = 0x44;
  // 3. check the previous allocation to see if its free
  curr_stride = *(uint32_t*)(p - 5);
  prev_stride = *(uint32_t*)(p - 5 - 4);
  curr_flags  = p - 1;
  prev_hdr   = p - 9 - (5 + prev_stride);
  if (prev_hdr < heap) {
     return;
  }
  prev_flags = prev_hdr + 4;
  if ((*curr_flags & FLAG_USED) == FLAG_USED) {
    return;
  }

  new_stride = prev_stride + curr_stride + 9;
  if ((*prev_flags & FLAG_USED) == 0) {
    *(uint32_t*)prev_hdr = new_stride;                      // set header
    *(uint32_t*)(prev_hdr + 5 + new_stride) = new_stride;   // set trailer
    *prev_flags = 0;                                        // set flags
    memset(prev_hdr + 5, 0x0, new_stride);
  }
}

void heap_init() {
  memset(heap, 0, heap_max_addr);
  *(uint32_t*)(heap) = heap_max_addr - 9;
  *(uint32_t*)(heap + 4) = 0;
  *(uint32_t*)(heap + heap_max_addr - 4) = heap_max_addr - 9;
}

int heap_dump(const char* fn) {
  FILE* f = fopen(fn, "wb");
  if (!f) {
    fprintf(stderr, "Failed to open file\n");
    return 1;
  }
  
  size_t bytes = fwrite(heap, sizeof(uint8_t), heap_max_addr, f);
  if (bytes != heap_max_addr) {
    fprintf(stderr, "Incomplete write [%zu/%d]\n", bytes, heap_max_addr);
    fclose(f);
    return 2;
  }
  printf("Wrote [%zu/%d] bytes\n", bytes, heap_max_addr);

  fclose(f);
  return 0;
}

#define TEST
#ifdef TEST
uint8_t* heap_test_get() {
  return heap;
}

uint32_t heap_test_get_current_size() {
  return heap_max_addr;
}

int heap_check_validity() {
  int offset = 0;

  while (offset < heap_max_addr) {
    uint32_t stride = ((uint32_t*)(offset + heap))[0];
    // uint8_t  flags  = ((uint8_t* )(offset + heap + 4))[0];

    uint32_t trailer = ((uint32_t*)(offset + heap + 5 + stride))[0];
    if (trailer != stride) {
      return 1;
    }
    offset += 4 + 1 + stride + 4;
  }
  if (offset != heap_max_addr) return 2;
  return 0;
}
#endif
