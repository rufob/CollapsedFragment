#include <stddef.h>
/*This is the short version. The bottom of the header file will host the long version*/

/*
Hashmap Container Class header for storing memory information
Use of the hashmap allows for detection and prevention of doublefrees in O(1) time and is as efficient at detecting leaks as any other container ( O(n) )
This class contains all the structs and functions necessary to operate the hashmap class that only serves to handle memory and cannot be expanded to other situations

See regions for specifics
Definions:16
Hashmap: 71
Vector: 112
Node: 134
*/

/*REGION: DEFINITIONS-------------------------------------------------------------------------------------------------------------*/
/*Hashmap for quick access of existing addresses to check for leaks or double free
NOTE: Exists on each heap and is destroyed just before the heap
NOTE: Is directly alloced memory; hueristically protected from running out of memory by being reliant on heap's available memory
*/
typedef struct hashmap_t hashmap_t;

/*
Expanding container with O(1) indexing. 
NOTE: Underlying head_node** - resized by its container as necessary(hashmap_t) 
*/
typedef struct vector_t vector_t;;

/*Stores first element in linked_list<node_t>and length of list*/
typedef struct head_node head_node;



/*
Stores address of memory allocation and backtrace of what stack at time of allocation
NOTE: node_t will be added on as overhead to any allocations in heap_alloc and should never by created elsewhere
*/
typedef struct node_t node_t;

/*Struct Defs are done in header due to privacy restrictions with other classes --------------------------------------------------*/
typedef struct node_t
{
	node_t* next; //forms linked list chain
	void* address; //address returned by tlsf_memalign and used to identify node
	void* backtrace[4]; /*symbol version of the backtrace*/
	int size; //number of bytes allocated to address
}node_t;

typedef struct head_node
{
	node_t* first;//first element in linked list
	int length; //number of nodes in list
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
	//default buckets is 2
	vector_t* vector;
	int size; // size is the total number of nodes present in map
} hashmap_t;
/*ENDREGION: DEFINITIONS----------------------------------------------------------------------------------------------------------*/

/*REGION: HASHMAP-----------------------------------------------------------------------------------------------------------------*/

/*Uses calloc and calls all levels of create from hashmap -> head_node. does not alloc nodes*/
hashmap_t* hashmap_create();

/*Cleanup method for hashmap, calls freeing of all alloc'd memory from head_node -> hashmap*/
void hashmap_destroy(hashmap_t* hashmap);

/*directly converts address into hash value with modulo
*/
int get_hash(const void* address, unsigned int bucket_count);

/*Double the size of the vector and redistributes items to newly correct buckets*/
void hashmap_resize(hashmap_t* hashmap);

/*Adds node to the hashmap by delegating down the line to list add
NOTE: Can call to hashmap_resize*/
void hashmap_add(hashmap_t* hashmap, node_t* node);

/*Removes target node from hashmap; only does so immediately before freeing
NOTE: makes assumptions about the form of void* address*/
void hashmap_remove(hashmap_t* hashmap, void* address);

/*Verify if node exists in the hashmap by address
Return 1 if found; 0 otherwise
NOTE: makes assumptions about the form of void* address*/
int hashmap_contains(hashmap_t* hashmap, void* address);

/*returns the first node encountered in the hashmap for the purposes of reporting and preventing leaks
NOTE: Treat this as an O(n) traversal or foreach; its a little more costly due to loop overhead but since it removes nodes its fine*/
node_t* hashmap_first_encounter(hashmap_t* hashmap);

/*NOTES for future upkeep*/
//this would probably make more sense as an iterator, rn its pretty expensive since it needs to be dne a shit ton of times
//iter stores node_t* and bucket position. iter ++ goes to next node, if next is null, goes to next bucket. if no buckets left, point to NULL
	//if i want to go crazy i can go typedef NULL EndOfMap

//overall not THAT expensive since its near constant time, just would be more efficent to not have to re start each time
//implement the iter after the rest of the homework is done 	
/*ENDREGION HASHMAP---------------------------------------------------------------------------------------------------------------*/

/*REGION: VECTOR------------------------------------------------------------------------------------------------------------------*/

/*Uses calloc to create vector and calls head_node**->head_node* create
NOTE: called from hashmap_create and calls vector_underlying_create*/
vector_t* vector_create();

/*Frees vector_t struct space and head_node*->head_node 
NOTE: called from hashmap_destroy and calls vector_underlying_destroy*/
void vector_destroy(vector_t* vector);

/*Uses calloc to create head_node** for storing list heads
Notes: called from vector_create and calls head_node_create*/
head_node** vector_underlying_create(size_t size);

/*Frees head_node** calls freeing of head_node* memory 
NOTE: called from vector_destroy and calls head_node_destroy*/
void vector_underlying_destroy(head_node** arr, size_t size);

/*Returns pointer to list head stored at vector index*/
head_node* vector_at(vector_t* vector, int index);
/*ENDREGION: VECTOR---------------------------------------------------------------------------------------------------------------*/

/*REGION: NODE--------------------------------------------------------------------------------------------------------------------*/

/* Uses calloc to create head_node
NOTE: called from vector_underlying_create*/
head_node* head_node_create();

//There is no node_create as it is allocated as overhead on heap_alloc's 

/*Frees memory alloc'd to head_node*s */
void head_node_destroy(head_node* head);

/*Append a node to the end of the linked_list<node_t>*/
void list_add(head_node* head, node_t* new_node);

/*
Removes a node from a linked_list<node_t> by comparing address
NOTE: Makes assumption about the form of void* address
*/
int list_remove(head_node* head, void* address);

/*Verify if node exists in the list by address. 
Return 1 if found; 0 otherwise
NOTE: makes assumptions about the form of void* address*/
int list_contains(head_node* head, void* address);
/*ENDREGION: NODE-----------------------------------------------------------------------------------------------------------------*/

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

	Currently, all of hashmap_t, vector_t, head_node are dynamically allocated by calloc, and node_t is added as overhead to the heap_allocs to minimize directly
	allocated overhead

Some backstory, should you want a read:
	I watched a talk over the summer by some CS person who had fairly strong opinions regarding design and I wanted to try to follow some of the principles;
		Im confident I probably did not do them justice but one thing I feel that was stressed was preventing unnecessary human error when it could be prevented.
		To that end, I decided to simply handle and remove the possibility of memory leaks and double frees (someone just earlier today told me that this was 
		maybe not the best design and i had for a brief moment hope of submitting 2 branches, one that was this, one that handled the homework directly and without
		extra capability or overhead. I was not able to do that), so I hope you don't actually mind that I chose to do more than was asked of me at a potential cost.

		I believe I was able to adhere to almost all of the Coding standard (I had wanted to avoid calloc but VirtualAlloc full page allocation seemed undesirable 
		for the piecemeal storage system I wanted to employ.) I think I successfully avoided win specific includes in header files but I may be mistaken about the
		agnosticism of stddef or stdlib

		I think im sorry?

		It shouldnt take me too long to change the implementation if the hashmap approach ends of being mutex with any future homeworks or planned capabilities
*/