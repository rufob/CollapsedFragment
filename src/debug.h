#pragma once
//! \file
//! \brief Debugging Support.
//! \remarks Similar robustness to printf

#include <stdint.h>

//! \brief Info Flags.
//! \sa debug_print()
typedef enum debug_print_t
{
	k_print_info = 1 << 0,
	k_print_warning = 1 << 1,
	k_print_error = 1 << 2,
}debug_print_t;

//! \brief Install unhandled exception handler.
//! \remarks When unhandled exceptions are caught, will log an error and capture a memory dump.
void debug_install_exception_handler();

//! \brief Set mask of which types of prints will actually fire.
//! \see debug_print()
void debug_set_print_mask(uint32_t mask);

//! \briefLog a message to the console.
//! \sa debug_set_print_mask()
//! \remarks Message may be dropped if type is not in the active mask.
void debug_print(uint32_t type, _Printf_format_string_ const char* format, ...);

//! \brief Capture a list of addresses that make up the current function callstack.
//! \return The number of addresses captured is the return value, stack contains at most stack_capacity addresses.
int debug_backtrace(void** stack, int stack_capacity);

//! \brief Formats and prints backtrace info for reporting callstack
/// \param Backtrace generated at time of heap_alloc by CaptureBacktrace
void print_alloc_backtrace(void** backtrace);
