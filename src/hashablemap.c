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
	//assumes address is of the form unsigned int* 
	int* hashable_address = (int*)address;

	unsigned hash = 0;       // Initial value of hash
	unsigned rand1 = 31415; // "Random" 1
	unsigned rand2 = 27183; // "Random" 2
	int i = 0;
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
	printf("For address %p produced hashable address %p to index %d\n", address, hashable_address, hash%bucket_count);
	return hash % bucket_count;
}

void hashmap_add(hashmap_t* hashmap, node_t* node)
{
	if (!hashmap_contains(hashmap, node->address)) {
		int index = get_hash(node->address, hashmap->vector->size);
		list_add(vector_at(hashmap->vector, index), node);
		printf("adding node with address %p to index %d\n", node->address, index);
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
	printf("begin resize with %d nodes\n", hashmap->size);
	//create new vector
	int old_size = hashmap->vector->size;
	int new_size = old_size * 2;
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
			printf("adding node with address %p to index %d\n", current->address, get_hash(current->address, new_size));
			current= hold;
		}
	}
	//free old vector and set pointer to new vector
	vector_underlying_destroy(old_head, hashmap->vector->size);
	hashmap->vector->size = new_size;
	hashmap->vector->arr = new_head;
	printf("end resize\n");
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
	printf("checking bucket of index %d\n", index);
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
	printf("started list_contains on address %p\n", address);
	node_t* current = head->first;
	//assumes void* address is actually of the form unsigned char*
	
	if (current && ((unsigned char*)(current->address)) == ((unsigned char*)(address)))
	{
		printf("current address %p\n", current->address);
		return 1;
		printf("completed list_contains on address %p\n", address);
	}
	while (current) {
		printf("current address %p\n", current->address);
		if ((unsigned char*)(current->address) == ((unsigned char*)(address)) )
		{
			printf("completed list_contains on address %p\n", address);
			return 1;
		}
		current = current->next;
		
	}
	printf("failed list_contains on address %p\n", address);
	return 0;
	
}

int list_remove(head_node* head, void* address)
{
	printf("started list_remove on address %p\n", address);
	node_t* current = head->first;
	//assumes address is of the form unsigned char; casts and compares
	if (current && *((unsigned char*)(current->address)) == *((unsigned char*)(address)))
	{
		head->first = current->next;
		head->length--;
		printf("completed list_remove on address %p\n", address);
		return 1;//consider making this the new length ? -1 for fail, 0 for empty
	}
			//i dont think i need to do the null check here due to the flow, but it gave me a warning
	while (current && current->next)
	{
		if ((unsigned char)(current->next->address) == *((unsigned char*)(address)) ) 
		{
			current->next = current->next->next;
			head->length--;
			printf("completed list_remove on address %p\n", address);
			return 1; //consider making this the new length ? -1 for fail, 0 for empty
		}
		current = current->next;
	}
	printf("failed list_remove on address %p\n", address);
	return 0;
}
/*ENDREGION: NODE-----------------------------------------------------------------------------------------------------------------*/
