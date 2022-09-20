#include <stdint.h>

typedef enum debug_print_t
{
	k_print_info = 1 << 0,
	k_print_warning = 1 << 1,
	k_print_error = 1 << 2,
} debug_print_t;

//Install unhandled exception handler.
// When unhanded exceptions are cought, will log an error and apture a mem dump
void debug_install_exception_handler();

//Set mask of which types of prints will actuall fire
// See the debug_print()
void debug_set_print_mask(uint32_t mask);

//Log a message to the console
// Message may be dropped if they type is not in the active mask
// See debug_set_print_mask
void debug_print(uint32_t type, _Printf_format_string_ const char* format, ...);

//Capture a list of addresses that make up the current function callstack
// on return, stack contains at most stack_capacity addresses
// the number of addresses captured is the return value
int debug_backtrace(void** stack, int stack_capacity);


void print_alloc_backtrace(void** backtrace);

