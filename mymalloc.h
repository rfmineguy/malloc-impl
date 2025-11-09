#ifndef MYMALLOC_H
#define MYMALLOC_H
#include <stdint.h>

#define HEAP_SIZE 0x1000
#define FLAG_USED 0x1

void* mymalloc(uint32_t);
void* mycalloc(uint32_t, uint32_t);
void  myfree(void*);

void  heap_init();
void  heap_dump(const char*);

#endif
