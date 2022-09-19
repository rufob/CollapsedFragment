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

/*REGION: HASHMAP-----------------------------------------------------------------------------------------------------------------*/

/*Hashmap for quick access of existing addresses to check for leaks or double free
NOTE: Exists before the first heap and is destroyed after the last heap
NOTE: Is directly alloced virtual memory; hueristically protected from running out of memory by being reliant on heap's available memory
*/
typedef struct hashmap_t hashmap_t;

hashmap_t* hashmap_create();

/*Hash function for struct hashmap_t - Makes assumptions about the form of void* address 
Hash function gotten from: https://azrael.digipen.edu/~mmead/www/Courses/CS280/HashFunctions-1.html
*/
int get_hash(void* address, int bucket_count);

/*Double the size of the vector and redistributes items to newly correct buckets*/
void hashmap_resize(vector_t* vector);
/*ENDREGION HASHMAP---------------------------------------------------------------------------------------------------------------*/


/*REGION: VECTOR------------------------------------------------------------------------------------------------------------------*/

/*Underlying head_node** - resized by its container as necessary(hashmap_t) */
typedef struct vector_t vector_t;

vector_t* vector_create();

vector_t* vector_at(vector_t* vector, int index);
/*ENDREGION: VECTOR---------------------------------------------------------------------------------------------------------------*/


/*REGION: NODE--------------------------------------------------------------------------------------------------------------------*/

/*Stores first element in linked_list<node_t> and length of list*/
typedef struct head_node head_node;

/*
Stores address of memory allocation and backtrace of what stack at time of allocation
NOTE: node_t will be added on as overhead to any allocations in heap_alloc and should never by created elsewhere
*/
typedef struct node_t node_t;


/*
Append a node to the end of the linked_list<node_t>
*/
void list_add(head_node* head, node_t* new_node);

/*
Removes a node from a linked_list<node_t> by comparing address
NOTE: Makes assumption about the form of void* address
*/
int list_remove(head_node* head, void* address);
/*ENDREGION: NODE-----------------------------------------------------------------------------------------------------------------*/
