#pragma once
//! \file
//! \brief  Event thread synchronization.

#include <stdbool.h>

//! \brief Handle to an event.
typedef struct event_t event_t;

//! \brief Creates a new event.
event_t* event_create();

//! \brief Destroys a previously created event.
void event_destroy(event_t* event);

//! \brief Signals an event.
//!
//! All threads waiting on this event will resume.
void event_signal(event_t* event);

//! \brief Waits for an event to be signaled.
void event_wait(event_t* event);

//! \brief Determines if an event is signaled.
bool event_is_raised(event_t* event);
