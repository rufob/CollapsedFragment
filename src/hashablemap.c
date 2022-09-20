#include "hashablemap.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*REGION: HASHMAP-----------------------------------------------------------------------------------------------------------------*/
	
node_t* hashmap_first_encounter(hashmap_t* hashmap)
{
	for (int i = 0; i < hashmap->vector->size; i++)
	{
		node_t* node = hashmap->vector->arr[i]->first;
		if (node)
		{
			return node;
		}
	}
	return NULL;
}

hashmap_t* hashmap_create()
{
	//hashmap_t* hashmap = VirtualAlloc(NULL, sizeof(hashmap_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	hashmap_t* hashmap = calloc(1, sizeof(hashmap_t) );
	if (!hashmap) 
	{
		return NULL;
	}
	hashmap->vector = vector_create();
	hashmap->size = 0;
	return hashmap;
}

void hashmap_destroy(hashmap_t* hashmap)
{
	vector_destroy(hashmap->vector);
	free(hashmap);
}

int get_hash(void* address, int bucket_count)
{
	//assumes address is of the form unsigned char* 
	int* hashable_address = (int*)address;

	unsigned hash = 0;       // Initial value of hash
	unsigned rand1 = 31415; // "Random" 1
	unsigned rand2 = 27183; // "Random" 2

	// Process each char in string
	while (*hashable_address)
	{
		// Multiply hash by random
		hash = hash * rand1;
		// Add in current char, keep within TableSize
		hash = (hash + *hashable_address);
		// Update rand1 for next "random" number
		rand1 = (rand1 * rand2);
		// Next char
		hashable_address++;
	}
	// Hash value is within 0 - TableSize - 1
	return hash % bucket_count;
}

void hashmap_add(hashmap_t* hashmap, node_t* node)
{
	if (!hashmap_contains(hashmap, node->address)) {
		int index = get_hash(node->address, hashmap->vector->size);
		list_add(vector_at(hashmap->vector, index), node);
		hashmap->size++;

		if ((double)hashmap->size / (double)hashmap->vector->size > 2)
		{
			hashmap_resize(hashmap);
		}
	}
}

void hashmap_remove(hashmap_t* hashmap, void* address)
{
	int index = get_hash(address, hashmap->vector->size);
	list_remove(vector_at(hashmap->vector, index), address);
	hashmap->size--;
	//hashmap_remove will not resize hashmap to reduce redudant compression at time of destruction
}

// resize should occur at head_node size >= 3 or when all headnodes size >= 2, dont know how to check secon
void hashmap_resize(hashmap_t* hashmap)
{
	//create new vector
	int size = hashmap->vector->size * 2;
	head_node** new_head = vector_underlying_create(size);
	//move all old entries into new vector
	for (int i = 0; i < hashmap->vector->size; i++)
	{
		head_node* temp = hashmap->vector->arr[i];
		node_t* current = temp->first;
		while (current && current->next)
		{
			list_add(new_head[get_hash(current->address, size)], current);
		}
	}
	//free old vector and set pointer to new vector
	vector_underlying_destroy(hashmap->vector->arr, hashmap->vector->size);
	hashmap->vector->arr = new_head;
}

int hashmap_contains(hashmap_t* hashmap, void* address)
{
	return list_contains(vector_at(hashmap->vector, get_hash(address, hashmap->vector->size)), address);
}

/*ENDREGION: HASHMAP--------------------------------------------------------------------------------------------------------------*/

/*REGION: VECTOR------------------------------------------------------------------------------------------------------------------*/

vector_t* vector_create() 
{
	vector_t* vector = calloc(1, sizeof(vector_t));
	if (!vector)
	{
		return NULL;
	}
	vector->arr = vector_underlying_create(2);
	vector->size = 2;
	return vector;
}

head_node** vector_underlying_create(size_t size)
{
	head_node** arr = calloc(size, sizeof(head_node*));
	if (!arr)
	{
		return NULL;
	}
	for (int i = 0; i < size; i++) 
	{
		arr[i] = head_node_create();
		arr[i]->first = NULL;
		arr[i]->length = 0;
	}
	return arr;
}

void vector_underlying_destroy(head_node** arr, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		head_node_destroy(arr[i]);
	}
	free(arr);
}

void vector_destroy(vector_t* vector)
{
	vector_underlying_destroy(vector->arr, vector->size);
	free(vector);
}

head_node* vector_at(vector_t* vector, int index)
{
	return vector->arr[index];
}
/*ENDREGION: VECTOR---------------------------------------------------------------------------------------------------------------*/

/*REGION: NODE--------------------------------------------------------------------------------------------------------------------*/

head_node* head_node_create()
{
	 head_node* head = calloc(1, sizeof(head_node));
	 return head;
}

void head_node_destroy(head_node* head_node) 
{
	free(head_node);
}

void list_add(head_node* head, node_t* new_node)
{
	node_t* current = head->first;
	if (!current)
	{
		head->first = new_node;
		new_node->next = NULL;
	}
	else {
		while (current->next)
		{
			current = current->next;
		}
		current->next = new_node;
	}
	head->length++;
	//consider making this return current length for resize checking
}

int list_contains(head_node* head, void* address) 
{
	node_t* current = head->first;
	//assumes void* address is actually of the form unsigned char*
	if (current && ((unsigned char*)(current->address)) == ((unsigned char*)(address)))
	{
		return 1;
	}
	while (current && current->next) {
		if ((unsigned char*)(current->address) == ((unsigned char*)(address)) )
		{
			return 1;
		}
		current = current->next;
	}
	return 0;
}

int list_remove(head_node* head, void* address)
{
	node_t* current = head->first;
	//assumes address is of the form unsigned char; casts and compares
	if (current && *((unsigned char*)(current->address)) == *((unsigned char*)(address)))
	{
		head->first = current->next;
		head->length--;
		return 1;//consider making this the new length ? -1 for fail, 0 for empty
	}
			//i dont think i need to do the null check here due to the flow, but it gave me a warning
	while (current && current->next)
	{
		if ((unsigned char)(current->next->address) == *((unsigned char*)(address)) ) 
		{
			current->next = current->next->next;
			head->length--;
			return 1; //consider making this the new length ? -1 for fail, 0 for empty
		}
		current = current->next;
	}
	return 0;
}
/*ENDREGION: NODE-----------------------------------------------------------------------------------------------------------------*/
