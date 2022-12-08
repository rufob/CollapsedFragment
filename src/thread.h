#pragma once

//! \file
//! \brief Threading support.

#include <stdint.h>

//! \brief Handle to a thread.
typedef struct thread_t thread_t;

//! \brief Creates a new thread.
//!
//! Thread begins running function with data on return.
thread_t* thread_create(int (*function)(void*), void* data);

//! \brief Waits for a thread to complete and destroys it.
//! \returns thread's exit code.
int thread_destroy(thread);

//! \brief Puts the calling thread to sleep for the specified number of milliseconds.
//! \remarks Thread will sleep for *approximately* the specified time.
void thread_sleep(uint32_t ms);
