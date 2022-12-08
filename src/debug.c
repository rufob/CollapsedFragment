//! \file
//! \brief Debug C File
//
//! this is also kinda a test

#include "debug.h"
#include <stdarg.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <DbgHelp.h>

static uint32_t s_mask = 0xffffffff;

const static unsigned int FRAME_MAX = 4;

static LONG debug_exception_handler(LPEXCEPTION_POINTERS ExceptionInfo)
{
	// XXX: MS uses 0xE06D7363 to indicate C++ language exception.
	// We're just to going to ignore them. Sometimes Vulkan throws them on startup?
	// https://devblogs.microsoft.com/oldnewthing/20100730-00/?p=13273
	if (ExceptionInfo->ExceptionRecord->ExceptionCode != 0xE06D7363) // changed this bc otherwise it breaks
	{
		return 0;
	}

	debug_print(k_print_error, "Caught exception!\n");
	
	HANDLE file = CreateFile(L"ga2022-crash.dmp", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION mini_exception = { 0 };
		mini_exception.ThreadId = GetCurrentThreadId();
		mini_exception.ExceptionPointers = ExceptionInfo;
		mini_exception.ClientPointers = FALSE;

		MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			file,
			MiniDumpWithThreadInfo,
			&mini_exception,
			NULL,
			NULL);

		CloseHandle(file);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void debug_install_exception_handler()
{
	AddVectoredExceptionHandler(TRUE, debug_exception_handler);
}

void debug_set_print_mask(uint32_t mask)
{
	s_mask = mask;
}

void debug_print(uint32_t type, _Printf_format_string_ const char* format, ...)
{
	if ((s_mask & type) == 0)
	{
		return;
	}
	va_list args;
	va_start(args, format);
	char buffer[256];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	OutputDebugStringA(buffer);
	DWORD bytes = (DWORD)strlen(buffer);
	DWORD written = 0;
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(out, buffer, bytes, &written, NULL);
}

int debug_backtrace(void** stack, int stack_capacity) 
{
	return CaptureStackBackTrace(2, stack_capacity, stack, NULL);
}

void print_alloc_backtrace(void** address)
{
	unsigned int   i;
	PIMAGEHLP_SYMBOL symbol;
	HANDLE         process;

	process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);
	SymSetOptions(SYMOPT_LOAD_LINES);	
	symbol = (PIMAGEHLP_SYMBOL)calloc(sizeof(PIMAGEHLP_SYMBOL) + 256 * sizeof(char), 1);
	if (symbol)
	{
		symbol->MaxNameLength = 255;
		symbol->SizeOfStruct = sizeof(PIMAGEHLP_SYMBOL);

		for (i = 0; i < FRAME_MAX; i++)
		{
			SymGetSymFromAddr64(process, (DWORD64)(address[i]), 0, symbol);

			//printf();
			debug_print(k_print_warning, "[%i] %s\n", FRAME_MAX - i - 1, symbol->Name);
		}
		free(symbol);
	}
	

	
	SymCleanup(process);
}
