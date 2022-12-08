#pragma once

//! \file
//! \brief High resolution timer support.

#include <stdint.h>

//! \brief Perform one-time initialization of the timer.
void timer_startup();

//! \brief Get the number of OS-defined ticks that have elapsed since startup.
uint64_t timer_get_ticks();

//! \brief Get the OS-defined tick frequency.
uint64_t timer_get_ticks_per_second();

//! \brief Convert a number of OS-defined ticks to nanoseconds.
uint64_t timer_ticks_to_ns(uint64_t t);

//! \brief Convert a number of OS-defined ticks to microseconds.
uint64_t timer_ticks_to_us(uint64_t t);

//! \brief Convert a number of OS-defined ticks to milliseconds.
uint32_t timer_ticks_to_ms(uint64_t t);
