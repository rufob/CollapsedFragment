#include <stdbool.h>
#include <stdint.h>

//Window Manager
//
// Main Object is wm_window_t to represent one OS-level window
// Window should pump every frame
// After pump can be queried for user input

// Handle to a window
typedef struct wm_window_t wm_window_t;

typedef struct heap_t heap_t;
//mouse buttons
enum
{
	k_mouse_button_left = 1 << 0,
	k_mouse_button_right = 1 << 1,
	k_mouse_button_middle = 1 << 2,
};
//keyboard buttons
enum
{
	k_key_up = 1 << 0,
	k_key_down = 1 << 1,
	k_key_left = 1 << 2,
	k_key_right = 1 << 3
};

// Creates a window. Must be destroyed with wm_destroy()
wm_window_t* wm_create(heap_t* heap);
void wm_destroy(wm_window_t* window);

//pump the message for a window
//refreshes mouse and key state of the window
bool wm_pump(wm_window_t* window);
//returns mask of all mouse/key buttons currently held
//see enums for usage
uint32_t wm_get_mouse_mask(wm_window_t* window);
uint32_t wm_get_key_mask(wm_window_t* window);
// gets relative mouse movement in x and y
void wm_get_mouse_move(wm_window_t* window, int* x, int* y);