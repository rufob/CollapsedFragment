#include "heap.h"
#include "debug.h"
#include "mutex.h"
#include "tlsf/tlsf.h"
#include "thread.h"
#include "hashablemap.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <stddef.h>
#include <stdio.h>
#include "mutex.h"


typedef struct arena_t
{
	pool_t pool;
	struct arena_t* next;
	//mutex_t mutex;
} arena_t;

typedef struct heap_t
{
	tlsf_t tlsf;
	size_t grow_increment;
	arena_t* arena;
	mutex_t* mutex;
	hashmap_t* memory_map; //tracks allocated memory to prevent leaks and double frees
} heap_t;

heap_t* heap_create(size_t grow_increment)
{
	heap_t* heap = VirtualAlloc(NULL, sizeof(heap_t) + tlsf_size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
	//mutex_create();

	if (!heap) {
		debug_print(
			k_print_error,
			"OUT OF MEMORY!!!\n");
		return NULL;
	}
	heap->mutex = mutex_create();
	heap->grow_increment = grow_increment;
	heap->tlsf = tlsf_create(heap + 1);
	heap->arena = NULL;
	heap->memory_map = hashmap_create();
	
	return heap;
}

void* heap_alloc(heap_t* heap, size_t size, size_t alignment)
{
	mutex_lock(heap->mutex);
	size_t size_with_overhead = size + sizeof(node_t);
	void* address = tlsf_memalign(heap->tlsf, alignment, size_with_overhead);
	if (!address)
	{
		size_t arena_size = __max(heap->grow_increment, size *2) + sizeof(arena_t);
		arena_t* arena = VirtualAlloc(NULL, arena_size+tlsf_pool_overhead(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!arena)
		{
			printf("OUT OF MEM\n");
			return NULL;
		}
		arena->pool = tlsf_add_pool(heap->tlsf, arena+1, arena_size);
		arena->next = heap->arena;
		heap->arena = arena;
		address = tlsf_memalign(heap->tlsf, alignment, size_with_overhead);
	}
	//node is just appended to the alloc, and then assign info using pointer arithmetic
	if (address) {
		node_t* node = (node_t*)((unsigned char*)address + size);
		node->address = address;
		node->size = (int)size;
		debug_backtrace(node->backtrace, 4);
		hashmap_add(heap->memory_map, node);
	}
	debug_print(k_print_warning, "heap_alloc successfull on address %p\n", address);
	mutex_unlock(heap->mutex);
	return address;
}

//multi level locking is allowed as long as we unlock equivalent number of times
void heap_free(heap_t* heap, void* address)
{
	mutex_lock(heap->mutex);
	printf("started heap_free on address %p\n", address);
	//verify that address is in hashmap to prevent double free
	if (hashmap_contains(heap->memory_map, address))
	{
		heap_free_checked(heap, address);
	}
	else {
		debug_print(k_print_warning, "Double free attempt detected for address %p, free attempt denied.\n", address);
		void* backtrace[4];
		debug_backtrace(backtrace, 4);
		print_alloc_backtrace(backtrace);
	}
	mutex_unlock(heap->mutex);
}

void heap_free_checked(heap_t* heap, void* address)
{
	mutex_lock(heap->mutex);
	hashmap_remove(heap->memory_map, address);
	tlsf_free(heap->tlsf, address);
	mutex_unlock(heap->mutex);
}

void heap_destroy(heap_t* heap)
{
	tlsf_destroy(heap->tlsf);
	//step through hashmap to find leaks
	mutex_lock(heap->mutex);
	node_t* current;
	while (current = hashmap_first_encounter(heap->memory_map))
	{
		//report leak
		debug_print(k_print_warning, "Memory leak of size: %d bytes(plus overhead of %d bytes) for address(%p) detected with callstack:\n",current->size, (int)sizeof(node_t), current->address);
		print_alloc_backtrace(current->backtrace);
		//prevent memory leak after reporting the error :)
		heap_free_checked(heap, current->address);
	}
	arena_t* arena = heap->arena;
	while (arena)
	{
		arena_t* next = arena->next;
		VirtualFree(arena, 0, MEM_RELEASE);
		arena = next;
	}
	mutex_unlock(heap->mutex);
	mutex_destroy(heap->mutex);
	VirtualFree(heap, 0, MEM_RELEASE);
}
