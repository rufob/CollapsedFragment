#pragma once

//! \file
//! \brief Counting semaphore thread synchronization

#include <stdbool.h>

//! \brief Handle to a semaphore.
typedef struct semaphore_t semaphore_t;

//! \brief Creates a new semaphore.
semaphore_t* semaphore_create(int initial_count, int max_count);

//! \brief Destroys a previously created semaphore.
void semaphore_destroy(semaphore_t* semaphore);

//! \brief Lowers the semaphore count by one.
//!
//! If the semaphore count is zero, blocks until another thread releases.
void semaphore_acquire(semaphore_t* semaphore);

//! \brief Attempts to lower the semaphore count by one.
//!
//! If the semaphore count is zero, returns false. Otherwise true.
bool semaphore_try_acquire(semaphore_t* semaphore);

//! \brief Raises the semaphore count by one.
void semaphore_release(semaphore_t* semaphore);
