
typedef struct thread_t thread_t;

thread_t* thread_create(int (* function)(void*), void* data);

int thread_destroy(thread);