#pragma once

//! \file
//! \brief Recursive mutex thread synchronization

//! \brief Handle to a mutex.
typedef struct mutex_t mutex_t;

//! \brief Creates a new mutex.
mutex_t* mutex_create();

//! \brief Destroys a previously created mutex.
void mutex_destroy(mutex_t* mutex);

//! \brief Locks a mutex.
//!
//! May block if another thread unlocks it.
//! If a thread locks a mutex multiple times, it must be unlocked multiple times.
void mutex_lock(mutex_t* mutex);

//! \brief Unlocks a mutex.
void mutex_unlock(mutex_t* mutex);
