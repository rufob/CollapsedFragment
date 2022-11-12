#include "atomic.h"
#include "fs.h"
#include "heap.h"
#include "net.h"
#include "render.h"
#include "simple_game.h"
#include "frogger_game.h"
#include "timer.h"
#include "wm.h"
#include "debug.h"
#include "hashablemap.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <excpt.h>

#include "frogger_game.h"
#include "queue.h"


int main(int argc, const char* argv[])
{
	debug_set_print_mask(k_print_info | k_print_warning | k_print_error);
	debug_install_exception_handler();

	timer_startup();

	heap_t* heap = heap_create(2 * 1024 * 1024);
	fs_t* fs = fs_create(heap, 8);
	wm_window_t* window = wm_create(heap);
	render_t* render = render_create(heap, window);

<<<<<<< HEAD
	//simple_game_t* game = simple_game_create(heap, fs, window, render);
	frogger_game_t* game = frogger_game_create(heap,fs,window,render);
	while (!wm_pump(window))
	{
		//simple_game_update(game);
		frogger_game_update(game);
=======
	int port = 12345;
	if (argc >= 2)
	{
		port = atoi(argv[1]);
	}
	net_t* net = net_create(heap, port);

	simple_game_t* game = simple_game_create(heap, fs, window, render);

	while (!wm_pump(window))
	{
		net_update(net);
		simple_game_update(game);
>>>>>>> upstream/main
	}
	

	/* XXX: Shutdown render before the game. Render uses game resources. */
	render_destroy(render);

<<<<<<< HEAD
	//simple_game_destroy(game);
	frogger_game_destroy(game);
	
=======
	simple_game_destroy(game);

	net_destroy(net);
>>>>>>> upstream/main
	wm_destroy(window);
	fs_destroy(fs);
	heap_destroy(heap);
	return 0;
}

