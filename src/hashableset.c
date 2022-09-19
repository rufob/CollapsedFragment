#include "hashableset.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

//rename everything to be clear that it handles specifically memory and isnt generally functioning

/*REGION: HASHMAP-----------------------------------------------------------------------------------------------------------------*/

typedef struct hashmap_t
{
	//use vector->size to identify the number of buckets
	//defaulyt buckets is 2. consider increasing
	vector_t* vector;
	int size; // size is the total number of nodes present
} hashmap_t;

//need a way to iterate through all the elements


//this would make more sense as an iterator, rn its pretty expensive since it needs to be dne a shit ton of times
//iter stores node_t* and bucket position. iter ++ goes to next node, if next is null, goes to next bucket. if no buckets left, point to NULL
	//if i want to go crazy i can go typedef NULL EndOfMap

//overall not THAT expensive since its near constant time, just would be more efficent to not have to re start each time
//implement the iter after the rest of the homework is done		
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
}



hashmap_t* hashmap_create()
{
	hashmap_t* hashmap = VirtualAlloc(NULL, sizeof(hashmap_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	hashmap->vector = vector_create();
	hashmap->size = 0;
	return hashmap;
}

BOOL hashmap_destroy(hashmap_t* hashmap)
{
	if (!vector_destroy(hashmap->vector))
	{
		return FALSE;
	}
	return VirtualFree(hashmap, 0, MEM_RELEASE);
}

int get_hash(void* address, int bucket_count)
{
	//assumes address is of the form unsigned char* 
	unsigned char* hashable_address = (unsigned char*)address;

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
	int index = get_hash(node->address, hashmap->vector->size);
	list_add(vector_at(hashmap->vector,index), node);
	hashmap->size++;

	if ((double)hashmap->size / (double)hashmap->vector->size > 2)
	{
		hashmap_resize(hashmap->vector);
	}
}

void hashmap_remove(hashmap_t* hashmap, node_t* node)
{
	int index = get_hash(node->address, hashmap->vector->size);
	list_remove(vector_at(hashmap->vector, index), node->address);
	hashmap->size--;
	//hashmap_remove will not resize hashmap to reduce redudant compression at time of destruction
}

// resize should occur at head_node size >= 3 or when all headnodes size >= 2, dont know how to check secon
void hashmap_resize(vector_t* vector)
{
	//create new vector
	int size = vector->size * 2;
	head_node** new_head = vector_underlying_create(size);
	//move all old entries into new vector
	for (int i = 0; i < vector->size; i++)
	{
		head_node* temp = vector->arr[i];
		node_t* current = temp->first;
		while (current && current->next)
		{
			list_add(new_head[get_hash(current->address, size)], current);
		}
	}
	//free old vector
	vector_underlying_destroy(vector->arr, vector->size);
	//set pointer to new vector
	vector->arr = new_head;
}

BOOL hashmap_contains(hashmap_t* hashmap, void* address)
{
	return list_contains(vector_at(hashmap->vector, get_hash(address, hashmap->vector->size)), address);
}

/*ENDREGION: HASHMAP--------------------------------------------------------------------------------------------------------------*/

/*REGION: VECTOR------------------------------------------------------------------------------------------------------------------*/

typedef struct vector_t
{
	//use at(vec, index)->count length to identify number of nodes in a bucket
	head_node** arr;
	int size; 
} vector_t;

vector_t* vector_create() 
{
	vector_t* vector = VirtualAlloc(NULL, sizeof(vector_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	vector->arr = vector_underlying_create(2);
	vector->size = 2;
	return vector;
}

head_node** vector_underlying_create(int size)
{
	head_node** arr = VirtualAlloc(NULL, sizeof(head_node*) * size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	for (int i = 0; i < size; i++) 
	{
		arr[i] = VirtualAlloc(NULL, sizeof(head_node), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		arr[i]->first = NULL;
		arr[i]->length = 0;
	}
	return arr;
}

BOOL vector_underlying_destroy(head_node** arr, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (!head_node_destroy(arr[i]))
		{
			return FALSE;
		}
	}
	return VirtualFree(arr, 0, MEM_RELEASE);
}

BOOL vector_destroy(vector_t* vector)
{
	//clear any remaining node_t
	//there should not be any remaining node_t since those would be reported as leaks from heap_destroy and then cleared

	//clear array of head_node*
	if (!vector_underlying_destroy(vector->arr, vector->size))
	{
		return FALSE;
	}
	return VirtualFree(vector, 0, MEM_RELEASE);
}

head_node* vector_at(vector_t* vector, int index)
{
	return vector->arr[index];
}
/*ENDREGION: VECTOR---------------------------------------------------------------------------------------------------------------*/

/*REGION: NODE--------------------------------------------------------------------------------------------------------------------*/

typedef struct head_node
{
	node_t* first; 
	int length;
} head_node;

typedef struct node_t
{
	node_t* next; // i think pointers are 8 bytes?
	void* address; //probably also 8 bytes, might be able to be 4 if i int it up
	void** backtrace; /*symbol version of the backtrace*/
	int size;
}node_t;

head_node* head_node_create()
{
	head_node* head = VirtualAlloc(NULL, sizeof(head_node), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

BOOL head_node_destroy(head_node* head_node) 
{
	return VirtualFree(head_node, 0, MEM_RELEASE);
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


//needs header comments
BOOL list_contains(head_node* head, void* address) 
{
	node_t* current = head->first;
	//assumes void* address is actually of the form unsigned char*
	if (current && *((unsigned char*)(current->address)) == *((unsigned char*)(address)))
	{
		return TRUE;
	}
	while (current->next) {
		if ((int)(current->next->address) == *((int*)(address)))
		{
			return TRUE;
		}
		current = current->next;
	}
	return FALSE;
}

int list_remove(head_node* head, void* address)
{
	node_t* current = head->first;

	//assumes address is of the form unsigned char; casts and compares
	if (current && *((unsigned char*)(current->address)) == *((unsigned char*)(address)))
	{
		head->first = NULL;
		head->length--;
		return 1; //consider making this the new length ? -1 for fail, 0 for empty
	}
			//i dont think i need to do the null check here due to the flow, but it gave me a warning
	while (current && current->next)
	{
		if ((int)(current->next->address) == *((int*)(address)))
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
