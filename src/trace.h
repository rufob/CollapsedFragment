#pragma once

//! \file
//! \brief Chrome Tracing support.

typedef struct heap_t heap_t;

typedef struct stacked_trace_t stacked_trace_t;

typedef struct event_trace_t event_trace_t;

typedef struct trace_t trace_t;

typedef struct trace_node_t trace_node_t;

//! \brief Creates a CPU performance tracing system.
//! \remark Event capacity is the maximum number of durations that can be traced.
trace_t* trace_create(heap_t* heap, int event_capacity);

//! \brief Destroys a CPU performance tracing system.
void trace_destroy(trace_t* trace);

//! \brief Begin tracing a named duration on the current thread.
//! \remarks It is okay to nest multiple durations at once.
void trace_duration_push(trace_t* trace, const char* name);

//! \brief End tracing the currently active duration on the current thread.
void trace_duration_pop(trace_t* trace);

//! \brief Start recording trace events.
//!
//! A Chrome trace file will be written to path.
void trace_capture_start(trace_t* trace, const char* path);

//! \brief Stop recording trace events.
void trace_capture_stop(trace_t* trace);

