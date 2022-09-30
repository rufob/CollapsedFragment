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

int get_hash(void* address, unsigned int bucket_count) 
{
	//this directly returns an int bc implicit conversion was causing inconsistencies i think. we just immediately convert to int in order to always know the state
	return (int)(intptr_t)address % bucket_count;
}
/* deprecated hash function because i didnt understand it and it was messy
static int get_hash(void* address, unsigned int bucket_count)
{
	//assumes address is of the form unsigned int*
	unsigned char* hashable_address = address;
	printf("\nha' a: %p; ha' v: %p; ha' pt: %d\na' a: %p a' v: %p; a' pt: %s\n", &hashable_address, hashable_address, *(hashable_address), &address, address, ((unsigned char*)address));
	unsigned hash = 0;       // Initial value of hash
	unsigned rand1 = 31415; // "Random" 1
	unsigned rand2 = 27183; // "Random" 2
	int i = 0;
	// Process each char in string
	while (i<16)
	{
		// Multiply hash by random
		hash = hash * rand1;
		// Add in current char, keep within TableSize
		hash = (hash + hashable_address[i]);
		// Update rand1 for next "random" number
		rand1 = (rand1 * rand2);
		// Next char
		i++;
	}
	// Hash value is within 0 - TableSize - 1
	printf("For address %p produced hashable address %p to index %d with %d buckets\n\n", address, hashable_address, hash%bucket_count, bucket_count);
	return hash % bucket_count;
}
*/


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
	if (list_remove(vector_at(hashmap->vector, index), address)) {
		hashmap->size--;
	}
	//hashmap_remove will not resize hashmap to reduce redudant compression at time of destruction
}

// resize should occur at head_node size >= 3 or when all headnodes size >= 2, dont know how to check secon
void hashmap_resize(hashmap_t* hashmap)
{
	//create new vector
	int old_size = hashmap->vector->size;
	int new_size = old_size * 3;
	head_node** new_head = vector_underlying_create(new_size);

	head_node** old_head = hashmap->vector->arr;
	
	//move all old entries into new vector
	for (int i = 0; i < old_size; i++)
	{
		head_node* temp = old_head[i];
		node_t* current = temp->first;
		node_t* hold;
		while (current)
		{
			hold = current->next;
			list_add(new_head[get_hash(current->address, new_size)], current);
			current= hold;
		}
	}
	//free old vector and set pointer to new vector
	vector_underlying_destroy(old_head, hashmap->vector->size);
	hashmap->vector->size = new_size;
	hashmap->vector->arr = new_head;
}

int hashmap_contains(hashmap_t* hashmap, void* address)
{
	int index = get_hash(address, hashmap->vector->size);
	return list_contains(vector_at(hashmap->vector, index), address);
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
	vector->size = 3; //this should be odd bc otherwise the modulo will be 0 most of the time
	vector->arr = vector_underlying_create(vector->size);
	
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
		new_node->next = NULL;
	}
	head->length++;
	//consider making this return current length for resize checking
}

int list_contains(head_node* head, void* address) 
{
	node_t* current  = head->first;
	//assumes void* address is actually of the form unsigned char*
	
	if (current && ((unsigned char*)(current->address)) == ((unsigned char*)(address)))
	{
		return 1;
		
	}
	while (current) {
		if ( (unsigned char*)current->address == (unsigned char*)address )
		{
			return 1;
		}
		current = current->next;
		
	}
	return 0;
	
}

int list_remove(head_node* head, void* address)
{
	int index = 0;
	node_t* current = head->first;
	node_t* hold;
	//assumes address is of the form unsigned char; casts and compares
	if (!current) 
	{
		return 0;
	}
	if ( (unsigned char*)current->address == (unsigned char*)address )
	{
		//printf("current address %p\n", current->address);
		head->first = current->next;
		head->length--;
		return 1;//consider making this the new length ? -1 for fail, 0 for empty
	}
			//i dont think i need to do the null check here due to the flow, but it gave me a warning
	hold = current;
	current = current->next;
	index++;
	while (current)
	{
		if ((unsigned char*)current->address == (unsigned char*)address ) 
		{
			hold->next = current->next ? current->next->next : NULL;
			head->length--;
			return 1; //consider making this the new length ? -1 for fail, 0 for empty
		}
		hold = current;
		current = current->next;
		index++;
	}
	return 0;
}
/*ENDREGION: NODE-----------------------------------------------------------------------------------------------------------------*/
