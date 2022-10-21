#include "trace.h"
#include "heap.h"
#include "timer.h"
#include "fs.h"
#include "debug.h"
#include "mutex.h"
#include "atomic.h"
#include "queue.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

typedef struct stacked_trace_t
{
	char* name;
	DWORD tid;
	struct stacked_trace_t* next;
}stacked_trace_t;

typedef struct event_trace_t
{
	char* name;
	DWORD pid;
	DWORD tid;
	uint64_t time;
	char op;
}event_trace_t;

typedef struct trace_t
{
	heap_t* heap;
	mutex_t* mutex;
	stacked_trace_t* first; //pointer to first element of tracelet dll
	char* path; //path to write for eventual exit
	queue_t* events; // queue of event information structs
	int length; // number of messages in time_stamps
	int capacity; // max number of events
	int size; // cumulative size (in bytes) of trace's report
	BOOL capture; //should an event be added to list
} trace_t;

//not declared in header to keep coding standard
//pops the most recent node sourced in this list from this thread
void* pop_list_thread_based(trace_t* trace, DWORD tid)
{
	stacked_trace_t* root = trace->first;
	if(root->tid == tid)
	{
		trace->first = root->next;
		return root;
	}
	stacked_trace_t* prev = root;
	root = root->next;
	//at this point prev is first and root is first->next
	while(root)
	{
		if(root->tid == tid)
		{
			stacked_trace_t* popped = root;
			if(prev)
			{
				prev->next = root->next;
			}
			return popped;
		}
		prev = root;
		root = root->next;
	}
	//should never hit this
	return NULL;
}

trace_t* trace_create(heap_t* heap, int event_capacity)
{
	trace_t* trace = heap_alloc(heap, sizeof(trace_t), 8);
	trace->heap = heap;
	trace->mutex = mutex_create();
	trace->first = NULL;
	trace->capacity = event_capacity;
	trace->events = NULL;
	trace->length = 0;
	trace->size = snprintf(NULL,0, "{\n\t\"displaytime\": \"ns\", \"traceEvents\": [\n\t]\n}");
	trace->capture = 0;
	return trace;
}

void trace_destroy(trace_t* trace)
{
	mutex_lock(trace->mutex);
	stacked_trace_t* root = trace->first;
	while(root)
	{
		stacked_trace_t* current = root;
		root = root->next;
		if(current)
		{
			heap_free(trace->heap, current);
		}
	}
	mutex_unlock(trace->mutex);
	mutex_destroy(trace->mutex);
	if(trace)
	{
		heap_free(trace->heap, trace);
	}
}

void trace_duration_push(trace_t* trace, const char* name)
{
	if (trace->capture == TRUE)
	{
		DWORD tid = GetCurrentThreadId();
		DWORD pid = GetCurrentProcessId();
		stacked_trace_t* stacked_trace = heap_alloc(trace->heap, sizeof(stacked_trace_t), 8);
		stacked_trace->name = (char*)name;
		stacked_trace->tid = tid;

		//add trace to stack
		mutex_lock(trace->mutex);
		stacked_trace->next = trace->first;
		trace->first = stacked_trace;
		mutex_unlock(trace->mutex);
		//record relevant info
		event_trace_t* event = heap_alloc(trace->heap, sizeof(event_trace_t), 8);
		event->name = (char*)name;
		event->pid = pid;
		event->tid = tid;
		event->op = 'B';
		queue_push(trace->events, event);
		uint64_t time =  timer_ticks_to_us(timer_get_ticks());
		event->time = time;
		atomic_increment(&trace->length);
	}
}

void trace_duration_pop(trace_t* trace)
{
	if (trace->capture) 
	{
		uint64_t time =  timer_ticks_to_us(timer_get_ticks());
		DWORD tid = GetCurrentThreadId();
		DWORD pid = GetCurrentProcessId();

		//pop from stack
		mutex_lock(trace->mutex);
		stacked_trace_t* op = pop_list_thread_based(trace, tid);
		mutex_unlock(trace->mutex);
		//record relevant event info
		event_trace_t* event = heap_alloc(trace->heap, sizeof(event_trace_t), 8);
		event->name = op->name;
		event->pid = pid;
		event->tid = tid;
		event->op = 'E';
		queue_push(trace->events, event);
		heap_free(trace->heap, op);
		atomic_increment(&trace->length);
		event->time = time;
	}
}

void trace_capture_start(trace_t* trace, const char* path)
{
	if(trace->capture != TRUE)
	{
		trace->events = queue_create(trace->heap, trace->capacity);
		trace->capture = TRUE;
		trace->path = (char*)path;
	}
}

void trace_capture_stop(trace_t* trace)
{
	mutex_lock(trace->mutex);
	fs_t* fs = fs_create(trace->heap, trace->length);
	event_trace_t* event_arr = heap_alloc(trace->heap, trace->length * sizeof(event_trace_t),8);
	//collect size and reorganize data 
	int buf_size = 0;
	buf_size += snprintf(NULL, 0,"{\n\t\"displaytime\": \"ns\", \"traceEvents\": [ ");
	for(int i =0; i < trace->length; i++)
	{
		event_trace_t* temp = queue_pop(trace->events);
		buf_size += snprintf(NULL,0,"\n\t\t{\"name\":\"%s\",\"ph\":\"%c\",\"pid\":%lu,\"tid\":\"%lu\",\"ts\":\"%llu\"}", temp->name, temp->op, temp->pid, temp->tid, temp ->time);
		if(i != trace->length-1) {buf_size++;}
		event_arr[i] = *(temp);
		heap_free(trace->heap,temp);
	}
	queue_destroy(trace->events);
	buf_size += snprintf(NULL, 0,"\n\t]\n}");

	//create buffer and add info to it
	char* print_buffer = heap_alloc(trace->heap, buf_size, 8);
	int index = 0;
	index += snprintf(print_buffer, buf_size,"{\n\t\"displaytime\": \"ns\", \"traceEvents\": [ ");
	for(int i =0; i < trace->length; i++)
	{
		event_trace_t temp = event_arr[i];
		index += snprintf((print_buffer + index),buf_size,"\n\t\t{\"name\":\"%s\",\"ph\":\"%c\",\"pid\":%lu,\"tid\":\"%lu\",\"ts\":\"%llu\"}", temp.name, temp.op, temp.pid, temp.tid, temp.time);
		if(i != trace->length-1) {index += snprintf((print_buffer + index),buf_size,",");}
	}
	snprintf((print_buffer + index),buf_size,"\n\t]\n}");
	fs_work_t* work = fs_write(fs, trace->path,print_buffer,buf_size,false);
	//cleanup
	fs_work_wait(work);
	fs_work_destroy(work);
	fs_destroy(fs);
	heap_free(trace->heap, event_arr);
	heap_free(trace->heap, print_buffer);
	trace->length = 0;
	trace->capture = FALSE;
	mutex_unlock(trace->mutex);
}
