#pragma once

//! \file
//! \brief Thread-Safe Queue Container

#include <stdbool.h>

//! \brief Handle to a thread-safe queue.
typedef struct queue_t queue_t;

//! \brief Handle to queue heap
typedef struct heap_t heap_t;

//! \brief Create a queue with the defined capacity.
queue_t* queue_create(heap_t* heap, int capacity);

//! \brief Destroy a previously created queue.
void queue_destroy(queue_t* queue);

//! \brief Push an item onto a queue.
//!
//! If the queue is full, blocks until space is available.
//! \remarks Safe for multiple threads to push at the same time.
void queue_push(queue_t* queue, void* item);

//! \brief Pop an item off a queue (FIFO order).
//! If the queue is empty, blocks until an item is avaiable.
//! \remarks Safe for multiple threads to pop at the same time.
void* queue_pop(queue_t* queue);

//! \brief Push an item onto a queue if space is available.
//! \returns false if queue is null.
//! \remarks Safe for multiple threads to push at the same time.
bool queue_try_push(queue_t* queue, void* item);

//! \brief Pop an item off a queue (FIFO order).
//! \returns NULL if queue is empty.
//! \remark for multiple threads to pop at the same time.
void* queue_try_pop(queue_t* queue);
