#include "trace.h"
#include "heap.h"
#include "timer.h"
#include "fs.h"
#include "debug.h"
#include "mutex.h"
#include "atomic.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int digits_in_int (uint64_t n);

typedef struct tracelet_t
{
	char* name;
	DWORD tid;
	struct tracelet_t* next;
}tracelet_t;

typedef struct trace_t
{
	heap_t* heap;
	mutex_t* mutex;
	tracelet_t* first; //pointer to first element of tracelet dll
	char* path; //path to write for eventual exit
	char** time_stamps; //all begin and end mesages thus far
	int length; // number of messages in time_stamps
	int capacity; // max number of events
	int size; // cumulative size (in bytes) of trace's report
	BOOL capture; //should an event be added to list
} trace_t;

void* pop_list_threadbased(trace_t* trace, DWORD tid)
{
	tracelet_t* root = trace->first;
	if(root->tid == tid)
	{
		trace->first = root->next;
		return root;
	}
	tracelet_t* prev = root;
	root = root->next;

	//at this point prev is first and root is first->next
	while(root)
	{
		if(root->tid == tid)
		{
			tracelet_t* popped = root;
			if(prev)
			{
				prev->next = root->next;
			}
			return popped;
		}
		prev = root;
		root = root->next;
	}
	return NULL;
}

trace_t* trace_create(heap_t* heap, int event_capacity)
{
	trace_t* trace = heap_alloc(heap, sizeof(trace_t), 8);
	trace->heap = heap;
	trace->mutex = mutex_create();
	trace->first = NULL;
	trace->capacity = event_capacity;
	trace->time_stamps = NULL;
	trace->length = 0;
	trace->size = snprintf(NULL,0, "{\n\t\"displaytime\": \"ns\", \"traceEvents\": [\n\t]\n}");
	trace->capture = 0;
	return trace;
}

void trace_destroy(trace_t* trace)
{
	mutex_lock(trace->mutex);
	tracelet_t* root = trace->first;
	while(root)
	{
		tracelet_t* current = root;
		root = root->next;
		if(current)
		{
			heap_free(trace->heap, current);
		}
	}
	for(int j = 0; j < trace->length; j++)
	{
		if(trace->time_stamps[j])
		{
			heap_free(trace->heap, trace->time_stamps[j]);
		}
	}
	if( trace->time_stamps)
	{
		heap_free(trace->heap, trace->time_stamps);
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
		int length = 45; //default chars + null term
		DWORD tid = GetCurrentThreadId();
		DWORD pid = GetCurrentProcessId();
		uint64_t time =  timer_ticks_to_us(timer_get_ticks());
		length += (int) strlen(name) + digits_in_int(tid) +  digits_in_int(pid) +  digits_in_int(time);
		tracelet_t* tracelet = heap_alloc(trace->heap, sizeof(tracelet_t), 8);
		tracelet->name = (char*)name;
		tracelet->tid = tid;
		int index = atomic_increment(&trace->length);
		trace->time_stamps[index] = heap_alloc(trace->heap, length * sizeof(char), 8);
		tracelet->next = trace->first;
		trace->first = tracelet;
		trace->size += snprintf(trace->time_stamps[index], length,
			"{\"name\":\"%s\",\"ph\":\"B\",\"pid\":%ld,\"tid\":\"%ld\",\"ts\":\"%llu\"}", name,pid, tid, time) +3;
	}
	
}

void trace_duration_pop(trace_t* trace)
{
	
	if (trace->capture) 
	{
		int length = 45;
		DWORD tid = GetCurrentThreadId();
		DWORD pid = GetCurrentProcessId();
		
		tracelet_t* op = pop_list_threadbased(trace, tid);
		int index = atomic_increment(&trace->length);
		uint64_t time =  timer_ticks_to_us(timer_get_ticks());
		length += (int) strlen(op->name) + digits_in_int(tid) +  (int)digits_in_int(pid) +  digits_in_int(time);
		trace->time_stamps[index] = heap_alloc(trace->heap, length * sizeof(char), 8);
		trace->size += snprintf(trace->time_stamps[index], length,
			"{\"name\":\"%s\",\"ph\":\"E\",\"pid\":%ld,\"tid\":\"%ld\",\"ts\":\"%llu\"}", op->name,pid, tid, time)+3;
		heap_free(trace->heap, op);
	}
}

void trace_capture_start(trace_t* trace, const char* path)
{
	if(trace->capture != TRUE)
	{
		trace->time_stamps = heap_alloc(trace->heap, trace->capacity*sizeof(char*), 8);
		trace->capture = TRUE;
		trace->path = (char*)path;
	}
}

void trace_capture_stop(trace_t* trace)
{
	mutex_lock(trace->mutex);
	fs_t* fs = fs_create(trace->heap, trace->length);
	trace->size += trace->length; //account for commas
	
	char* print_buffer = heap_alloc(trace->heap, trace->size, 8);
	strcpy_s(print_buffer, trace->size, "{\n\t\"displaytime\": \"ns\", \"traceEvents\": [\0");
	for (int i = 0; i < trace->length-1; i++) 
	{
		strcat_s(print_buffer, trace->size, "\n\t\t");
		strcat_s(print_buffer, trace->size, trace->time_stamps[i]);
		strcat_s(print_buffer, trace->size, ",");
	}
	strcat_s(print_buffer, trace->size, "\n\t\t");
	strcat_s(print_buffer,trace->size, trace->time_stamps[trace->length-1]);
	strcat_s(print_buffer, trace->size, "\n\t]\n}\0");

	fs_work_t* work = fs_write(fs, trace->path,print_buffer,trace->size-1,false);
	
	fs_work_wait(work);
	fs_work_destroy(work);
	fs_destroy(fs);
	for(int j = 0; j < trace->length; j++)
	{
		heap_free(trace->heap, trace->time_stamps[j]);
	}
	heap_free(trace->heap, trace->time_stamps);
	trace->time_stamps = NULL;
	heap_free(trace->heap, print_buffer);
	trace->length = 0;
	trace->capture = FALSE;
	mutex_unlock(trace->mutex);
}

/*https://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-digits-of-an-integer-in-c*/
int digits_in_int (uint64_t n) {
	//if (n < 0) return 0;//n = (n == INT_MIN) ? INT_MAX : -n;
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	/*      2147483647 is 2^31-1 - add more ifs as needed
	   and adjust this final return as well. */
	return 10;
}