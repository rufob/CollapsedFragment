#include <stddef.h>

/*
Originally, this was going to be 3 separate classes each dedicated to a class / struct
	hashmap
	vector
	head_node and node
In which the:   Hashable map handles all the hashing and outside manipulation of the vector
				Vector is a robust structure capable of being repurposed outside of the needs of the hashmap 
					mirroring functionality of cpp vec
				Node is a element of a linked list, where head_node stores the length and start of each list of nodes
	I don't know the intricacies of C well enough to implement a vector that was robust enough so I gave up.
	since all of the structs were so intertwinned, I moved them all into this hashablemap class
	Perhaps in the future I will revist this and improve it
	
	*/

/*REGION: DEFINITIONS-------------------------------------------------------------------------------------------------------------*/
/*Hashmap for quick access of existing addresses to check for leaks or double free
NOTE: Exists before the first heap and is destroyed after the last heap
NOTE: Is directly alloced virtual memory; hueristically protected from running out of memory by being reliant on heap's available memory
*/
typedef struct hashmap_t hashmap_t;


/*Underlying head_node** - resized by its container as necessary(hashmap_t) */
typedef struct vector_t vector_t;;

/*Stores first element in linked_list<node_t>and length of list*/
typedef struct head_node head_node;

/*
Stores address of memory allocation and backtrace of what stack at time of allocation
NOTE: node_t will be added on as overhead to any allocations in heap_alloc and should never by created elsewhere
*/
typedef struct node_t node_t;

typedef struct node_t
{
	/*struct*/ node_t* next; // i think pointers are 8 bytes?
	void* address; //probably also 8 bytes, might be able to be 4 if i int it up
	void* backtrace[16]; /*symbol version of the backtrace*/
	int size;
}node_t;

typedef struct head_node
{
	node_t* first;
	int length;
} head_node;


typedef struct vector_t
{
	//use at(vec, index)->count length to identify number of nodes in a bucket
	head_node** arr;
	int size;
} vector_t;

typedef struct hashmap_t
{
	//use vector->size to identify the number of buckets
	//defaulyt buckets is 2. consider increasing
	vector_t* vector;
	int size; // size is the total number of nodes present
} hashmap_t;
/*ENDREGION: DEFINITIONS----------------------------------------------------------------------------------------------------------*/

/*REGION: HASHMAP-----------------------------------------------------------------------------------------------------------------*/


hashmap_t* hashmap_create();

void hashmap_destroy(hashmap_t* hashmap);

/*Hash function for struct hashmap_t - Makes assumptions about the form of void* address 
Hash function gotten from: https://azrael.digipen.edu/~mmead/www/Courses/CS280/HashFunctions-1.html
*/
int get_hash(void* address, int bucket_count);

/*Double the size of the vector and redistributes items to newly correct buckets*/
void hashmap_resize(hashmap_t* hashmap);

void hashmap_add(hashmap_t* hashmap, node_t* node);

void hashmap_remove(hashmap_t* hashmap, void* address);

int hashmap_contains(hashmap_t* hashmap, void* address);

node_t* hashmap_first_encounter(hashmap_t* hashmap);

/*ENDREGION HASHMAP---------------------------------------------------------------------------------------------------------------*/


/*REGION: VECTOR------------------------------------------------------------------------------------------------------------------*/



vector_t* vector_create();

void vector_destroy(vector_t* vector);

head_node** vector_underlying_create(size_t size);

void vector_underlying_destroy(head_node** arr, size_t size);

head_node* vector_at(vector_t* vector, int index);
/*ENDREGION: VECTOR---------------------------------------------------------------------------------------------------------------*/


/*REGION: NODE--------------------------------------------------------------------------------------------------------------------*/



head_node* head_node_create();

void head_node_destroy(head_node* head);

/*I should move this to header?; might not work there
need this function bc heap.c doesnt know node_t size for alloc purposes and i dont want to defin
struct in header since that would break convention established and i dont want to include.c
bc that break convention (but technically allowed by coding standard)*/
static size_t node_size()
{
	return sizeof(node_t);
}
/*
Append a node to the end of the linked_list<node_t>
*/
void list_add(head_node* head, node_t* new_node);

/*
Removes a node from a linked_list<node_t> by comparing address
NOTE: Makes assumption about the form of void* address
*/
int list_remove(head_node* head, void* address);


int list_contains(head_node* head, void* address);
/*ENDREGION: NODE-----------------------------------------------------------------------------------------------------------------*/
