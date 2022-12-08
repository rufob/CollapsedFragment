#pragma once

//! \file
//! \brief Main object heap_t.
//!
//! Represents dynamic memory heap. Memory can be allocated to and from heap

#include <stdlib.h>


//! \brief Handle to a heap
typedef struct heap_t heap_t;

//! \brief Creates a new mem heap.
//!
//! The grow increment is the default size with which the heap grows
//! \remark should be a multiple of OS page size
heap_t* heap_create(size_t grow_increment);

//! \brief Destroy a previously created heap
void heap_destroy(heap_t* heap);

//! \brief Allocate memory from a heap
void* heap_alloc(heap_t* heap, size_t size, size_t alignment);

//! \brief Generic heap_free, called by end user. 
//!
//! Free memory previously allocated from a heap
//! \remarks verifies address is in hashmap then calls heap_free_checked
void heap_free(heap_t*, void* address);

//! \brief Frees memory allocated from heap.
//!
//! Occurs after it has been verified to not have been freed already
//! \remarks Should only be called from heap_free or directly from heap_destroy
void heap_free_checked(heap_t* heap, void* address);
