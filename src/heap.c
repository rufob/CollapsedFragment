#include "heap.h"
#include "debug.h"
#include "tlsf/tlsf.h"
#include "thread.h"
#include "hashablemap.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <stddef.h>
#include <stdio.h>

typedef struct arena_t
{
	pool_t pool;
	struct arena_t* next;
} arena_t;

typedef struct heap_t
{
	tlsf_t tlsf;
	size_t grow_increment;
	arena_t* arena;
	hashmap_t* memory_map;
} heap_t;

heap_t* heap_create(size_t grow_increment)
{
	heap_t* heap = VirtualAlloc(NULL, sizeof(heap_t) + tlsf_size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!heap) {
		debug_print(
			k_print_error,
			"OUT OF MEMORY!!!\n");
		return NULL;
	}
	heap->grow_increment = grow_increment;
	heap->tlsf = tlsf_create(heap + 1);
	heap->arena = NULL;
	heap->memory_map = hashmap_create();
	
	return heap;
}

//should i name memory map after fuction of black box it and just use comments
void* heap_alloc(heap_t* heap, size_t size, size_t alignment)
{
	size_t size_with_overhead = size + node_size();
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
	//alloc space for node on the pool? on smth.
	//node is just appended to the alloc, and then assign info using pointer arithmetic
	if (address) {
		//DECIDE WHERE TO MOVE NODE DEFINITION TO, GIVEN ALL ITS SHIT IS BROKE AS HELL IN THE C FILE.
		node_t* node = (node_t*)((unsigned char*)address + size);
		node->address = address;
		node->size = (int)size;
		debug_backtrace(node->backtrace, 16);
		//hashmap stores nodes but is composed 
		hashmap_add(heap->memory_map, node);
		heap_t* heap = VirtualAlloc(NULL, sizeof(heap_t) + tlsf_size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!heap)
		{
			debug_print(k_print_error, "OUT OF MEMORY!\n");
			return NULL;
		}

		heap->grow_increment = grow_increment;
		heap->tlsf = tlsf_create(heap + 1);
		heap->arena = NULL;
	}

	return heap;
}

//be worrie dabout function issues
void heap_free(heap_t* heap, void* address)
{
	if (hashmap_contains(heap->memory_map, address))
	{
		heap_free_checked(heap, address);
	}
}

void heap_free_checked(heap_t* heap, void* address)
{
	hashmap_remove(heap->memory_map, address);
	//verify that address is in hashmap to prevent double free
	tlsf_free(heap->tlsf, address);
}

void heap_destroy(heap_t* heap)
{
	tlsf_destroy(heap->tlsf);
	//step through hashmap to find leaks
	node_t* current;
	while (current = hashmap_first_encounter(heap->memory_map))
	{
		//report leak
		print_alloc_backtrace(current->backtrace);

		heap_free_checked(heap, current->address);
	}
	//report leak, free, and then remove from hashmap

	arena_t* arena = heap->arena;
	while (arena) {
	arena_t* arena = heap->arena;
	while (arena)
	{
		arena_t* next = arena->next;
		VirtualFree(arena, 0, MEM_RELEASE);
		arena = next;
	}
	VirtualFree(heap, 0, MEM_RELEASE);
}
