#pragma once

#include <stdlib.h>

//Heap Memory Manager
// Main object, heap_t, represents dynamic mem heap
// Once created, memory can be allocated and free from the heap

//Handle to a heap
typedef struct heap_t heap_t;

//Creates a new mem heap
// The grow increment is the defauly size with which the heap grows
// should be a multiple of OS page size
heap_t* heap_create(size_t grow_increment);

//Destroy a previously created heap
void heap_destroy(heap_t* heap);

//Allocate memory from a heap
void* heap_alloc(heap_t* heap, size_t size, size_t alignment);

/*Generic heap_free, called by end user. 
Free memory previously allocated from a heap
NOTE: verifies address is in hashmap then calls heap_free_checked*/
void heap_free(heap_t*, void* address);

/*Frees memory allocated from heap after it has been verified to not have been freed
NOTE: Should only be called from heap_free or directly from heap_destroy*/
void heap_free_checked(heap_t* heap, void* address);
