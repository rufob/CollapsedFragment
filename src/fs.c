#include "fs.h"
#include "heap.h";
#include "queue.h"
#include "thread.h"



typedef struct fs_t
{
	heap_t* heap;
	queue_t* file_queue;
	thread_t* file_thread;
}fs_t;

typedef enum fs_work_op_t
{
	k_fs_work_op_read,
	k_fs_work_op_write
}fs_work_op_t;
typedef struct fs_work_t
{
	fs_work_op_t op;
	char* path[1024];
	void* buffer;

}fs_work_t;;

fs_t* fs_create(heap_t* heap, int queue_capacity)
{

	fs_t* fs = heap_alloc(heap, sizeof(fs_t), 8);
	fs->heap = heap;
	fs->file_thread = thread_create(file_thread_func, fs);
	fs->file_queue = queue_create(heap, queue_capacity);
	return NULL;
}

void fs_destroy(fs_t* fs)
{
	queue_push(fs->file_queue, NULL);
	thread_destroy(fs->file_thread);
	queue_destroy(fs->file_queue);
	heap_free(fs->heap, fs);
}

fs_work_t* fs_read(fs_t* fs, const char* path)
{
	fs_work_t* item = heap_alloc(fs->heap, sizeof(fs_work_t), 8);
	item->op = k_fs_work_op_read;
	strcpy_s(item->path, sizeof(item->path), path);
	queue_push(fs->file_queue, item);
}


static int file_thread_func(void* user)
{
	fs_t* fs = user;
	while (1)
	{
		fs_work_t* item = queue_pop(fs->file_queue);
		if (item == NULL)
		{
			break;
		}
	}
	return 0;
}