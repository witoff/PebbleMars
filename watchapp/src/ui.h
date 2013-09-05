#pragma once
#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "PebbleMars.h"

/*Initialize and destroy the UI elements for our application*/
void ui_init();
void ui_deinit();

/* Slides the progress separator down to to_row */
void slide_progress_separator(uint8_t to_row);

/* Sets dword at index / IMAGE_COLS, index % IMAGE_COLS */
void mars_image_set_dword(uint16_t index, uint32_t dword);

/*Mark the the mars image layer dirty to get Pebble to redraw*/
void mars_image_mark_dirty();

/* Set the martian image to blank white */
void mars_image_clear();

/*Set information text at the top of the screen. */
/*TODO: Add more documentation here */
void set_info_text(uint8_t key, const char *text);

