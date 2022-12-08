#pragma once

//! \file
//! \brief Asynchronous read/write file system.

#include <stdbool.h>
#include <stddef.h>

//! \brief Handle to file system.
typedef struct fs_t fs_t;

//! \brief Handle to file work.
typedef struct fs_work_t fs_work_t;

//! \brief Handle to file system heap.
typedef struct heap_t heap_t;

//! \brief Create a new file system.
//! \remarks Provided heap will be used to allocate space for queue and work buffers.\n Provided queue size defines number of in-flight file operations.
fs_t* fs_create(heap_t* heap, int queue_capacity);

//! \brief Destroy a previously created file system.
void fs_destroy(fs_t* fs);

//! \brief Queue a file read.
//!
//! File at the specified path will be read in full.
//! \return Work object.
//! \remarks Memory for the file will be allocated out of the provided heap.\n It is the calls responsibility to free the memory allocated!
fs_work_t* fs_read(fs_t* fs, const char* path, heap_t* heap, bool null_terminate, bool use_compression);

//! \brief Queue a file write.
//!
//! File at the specified path will be written in full.
//! \return Work object.
fs_work_t* fs_write(fs_t* fs, const char* path, const void* buffer, size_t size, bool use_compression);

//! \brief If true, the file work is complete.
bool fs_work_is_done(fs_work_t* work);

//! \brief Block for the file work to complete.
void fs_work_wait(fs_work_t* work);

//! \brief Get the error code for the file work.
//! \returns Zero for success, generally.
int fs_work_get_result(fs_work_t* work);

//! \brief Get the buffer associated with the file operation.
void* fs_work_get_buffer(fs_work_t* work);

//! \brief Get the size associated with the file operation.
size_t fs_work_get_size(fs_work_t* work);

//! \brief Free a file work object.
void fs_work_destroy(fs_work_t* work);
