#include "wm.h"
#include "heap.h"
#include "debug.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

typedef struct wm_window_t
{
	HWND hwnd;
	bool quit;
	bool has_focus;
	uint32_t mouse_mask; // 0 = LMB, 1 = RMB, 2 = MMB
	uint32_t key_mask;
	int mouse_x;
	int mouse_y;
	heap_t* heap;
} wm_window_t;

const struct
{
	int virtual_key;
	int cf_key;
}
k_key_map[] =
{
	{.virtual_key = VK_UP, .cf_key = k_key_up, },
	{.virtual_key = VK_DOWN, .cf_key = k_key_down, },
	{.virtual_key = VK_LEFT, .cf_key = k_key_left,},
	{.virtual_key = VK_RIGHT, .cf_key = k_key_right, },
};

static LRESULT CALLBACK _window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	wm_window_t* win = (wm_window_t*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_KEYDOWN:
		for (int i = 0; i < _countof(k_key_map); ++i) 
		{
			if (k_key_map[i].virtual_key == wParam)
			{
				win->key_mask |= k_key_map[i].cf_key;
				break;
			}
		}
		break;
	case WM_KEYUP:
		for (int i = 0; i < _countof(k_key_map); ++i)
		{
			if (k_key_map[i].virtual_key == wParam)
			{
				win->key_mask &= ~k_key_map[i].cf_key;
				break;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		win->mouse_mask |= k_mouse_button_left;
		break;
	case WM_LBUTTONUP:
		win->mouse_mask &= ~k_mouse_button_left;
		break;
	case WM_RBUTTONDOWN:
		win->mouse_mask |= k_mouse_button_right;
		break;
	case WM_RBUTTONUP:
		win->mouse_mask &= ~k_mouse_button_right;
		break;
	case WM_MBUTTONDOWN:
		win->mouse_mask |= k_mouse_button_middle;
		break;
	case WM_MBUTTONUP:
		win->mouse_mask &= ~k_mouse_button_middle;
		break;
	case WM_MOUSEMOVE:
		if (win->has_focus)
		{
			//Relative mouse movement
			//Start with current pos
			POINT old_cursor;
			GetCursorPos(&old_cursor);
			RECT window_rect;
			GetWindowRect(hwnd, &window_rect);
			//Move mouse to center screen
			SetCursorPos(
				(window_rect.left + window_rect.right) / 2, 
				(window_rect.top + window_rect.bottom) / 2);
			//Get Current Mouse Pos
			POINT new_cursor;
			GetCursorPos(&new_cursor);
			//compute relative mouse
			
			win -> mouse_x = old_cursor.x - new_cursor.x;
			win->mouse_y = old_cursor.y - new_cursor.y;
			
		}

		break;
	case WM_ACTIVATEAPP:
		ShowCursor(!wParam);
		win->has_focus = wParam;
		break;
	case WM_CLOSE:
		win->quit = true;
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


wm_window_t* wm_create(heap_t* heap)
{
	WNDCLASS wc =
	{
		.lpfnWndProc = _window_proc,
		.hInstance = GetModuleHandle(NULL),
		.lpszClassName = L"ga2022 window class",
	};
	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		L"GA 2022",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		wc.hInstance,
		NULL);

	if (!hwnd) {
		debug_print(
			k_print_warning,
			"Failed to create window!\n");
		
		return NULL;
	}
	

	wm_window_t* win = heap_alloc(heap, sizeof(wm_window_t), 8);
	win->has_focus = false;
	win->hwnd = hwnd;
	win->key_mask = 0;
	win->mouse_mask = 0;
	win->quit = false;
	win->heap = heap;

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)win);

	//Windows are hidden by default
	ShowWindow(hwnd, TRUE);

	return win;
}

bool wm_pump(wm_window_t* window)
{
	MSG msg = { 0 };
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return window->quit;
}

uint32_t wm_get_mouse_mask(wm_window_t* window)
{
	return window->key_mask;
}

uint32_t wm_get_key_mask(wm_window_t* window)

{
	return window->key_mask;
}

void wm_get_mouse_move(wm_window_t* window, int* x, int* y)
{
	*x = window->mouse_x;
	*y = window->mouse_y;
}

void wm_destroy(wm_window_t* window)
{
	DestroyWindow(window->hwnd);
	heap_free(window->heap, window);
}
