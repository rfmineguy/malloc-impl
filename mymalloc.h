#ifndef MYMALLOC_H
#define MYMALLOC_H
#include <stdint.h>

#define HEAP_MAX_SIZE 0x8000            // the heap is allowed to grow up to this size
#define HEAP_START_SIZE 0x1000          // the heap starts at this size
#define HEAP_RESIZE_AMT 0x1000          // how much memory gets added to the heap when it resizes
#define FLAG_USED 0x1

/*
 * @Desc:
 *     Allocate a new region of memory in the heap
 * @Param:
 *     size - the size(in bytes) of the allocation
 * @Return:
 *     pointer to the beginning of the newly allocated region
 *     null if allocation failed
 * @Notes:
 *   - there is no protection against writing outside the bounds of your allocation
 *      so if you do write outside of the allocation, there is a high chance that this
 *      will invalidate the heap metadata, and make it non traversable (aka corrupted)
 *   - it is up to the caller to respect the size that they requested
 */
void* mymalloc(uint32_t size);

/*
 * @Desc:
 *     Allocate a new region of memory in the heap
 *     This will zero out the memory region as well
 * @Param:
 *     count - the number of elements to allocate
 *     size  - the size(in bytes) of each element expected to be stored
 * @Return:
 *     pointer to the beginning of the newly allocated memory region
 *     null if allocation failed
 * @Notes:
 *   - see notes for @mymalloc
 */
void* mycalloc(uint32_t count, uint32_t size);

/*
 * @Desc:
 *     Free an allocation made previously by @mymalloc, or @mycalloc
 * @Param:
 *     ptr - a pointer that has been previously allocated
 * @Return: none
 * @Notes:
 *   - if ptr is not within the bounds of the heap, no operations are performed
 */
void  myfree(void* ptr);

/*
 * @Desc:
 *     Initialize the heap to be 4kb in a known, and valid state
 * @Param: none
 * @Return: none
 * @Notes:
 *   - for information on validity, see #heap_check_validity
 */
void  heap_init(void);

/*
 * @Desc:
 *     Take the current heap and dump its binary data to a file
 * @Param:
 *     filename - what file we should dump into
 * @Return:
 *     0 - success
 *     1 - failed to open 'filename'
 *     2 - incomplete write to 'filename'
 * @Notes:
 *   - this is most useful when opened with a hex editor
 */
int   heap_dump(const char* filename);

#ifdef TEST
/*
 * @Desc:
 *     Retrieve a mutable pointer to the internal heap array
 * @Param: none
 * @Return:
 *   - mutabile pointer to the internal heap array
 * @Notes:
 *   - this mutability of the returned pointer is intentional to allow the user
 *      of this function to do what they want with it. if they break the heap by
 *      modifying it, its their fault.
 */
uint8_t* heap_test_get(void);

/*
 * @Desc:
 *     Retrieve the current size of this heap.
 * @Param: none
 * @Return:
 *   - the current size of the heap
 * @Notes:
 *   - the current size of the heap is a different concept to the max size of the heap
 *   - because the heap can grow, this size may change given enough allocations
 */
uint32_t heap_test_get_current_size(void);

/*
 * @Desc:
 *     Determine whether the heap is in a valid state
 * @Param: none
 * @Return:
 *     0 - heap is in a valid state
 *     1 - a trailer somewhere doesn't match the header
 * @Notes:
 *   - being a valid state means that the heap is fully traversable
 *   - if the heap is fully traversable we can assume the metadata is correct
 */
int      heap_check_validity(void);
#endif

#endif
