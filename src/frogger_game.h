#pragma once

//! \file
//! \brief Frogger Test Game.
//
//! Brings together major engine systems to make a frogger game for homework 4

//! \brief Handle to the Frogger Game.
typedef struct frogger_game_t frogger_game_t;

//! \brief Handle to the file system.
typedef struct fs_t fs_t;
//! \brief Handle to the reap.
typedef struct heap_t heap_t;
//! \brief Handle to the render.
typedef struct render_t render_t;

//! \brief Handle to the window.
typedef struct wm_window_t wm_window_t;

//! \brief Create an instance of frogger game.
frogger_game_t* frogger_game_create(heap_t* heap, fs_t* fs, wm_window_t* window, render_t* render);

//! \brief Destroy an instance of frogger game.
void frogger_game_destroy(frogger_game_t* game);

//! \brief Per-frame update for our simple test game.
void frogger_game_update(frogger_game_t* game);
