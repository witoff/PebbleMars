#pragma once

#include "pebble_os.h"

#define APP_MESSAGE_BUF_IN 100
#define APP_MESSAGE_BUF_OUT 100
#define KEY_HELLO 42

#define KEY_NAME 2
#define KEY_INSTRUMENT 3
#define KEY_UTC 4

#define IMAGE_WIDTH 144
#define IMAGE_HEIGHT 144
#define IMAGE_COLS ((IMAGE_WIDTH - 1) / (8 * sizeof(gbword_t)) + 1)
#define IMAGE_ROWS (IMAGE_HEIGHT)

// GBitmap Word Type
typedef uint32_t gbword_t;

// Pebble Graphics Int Type
typedef uint16_t gint_t;

gbword_t (*image_buffer)[IMAGE_ROWS][IMAGE_COLS];
GBitmap image_bitmap;

void display_new_image();
void set_footer_text(const char *text);

void set_bitmap_byte(uint16_t index, uint8_t byte);
void clear_bitmap();

#if SHOW_METADATA
void set_metadata_text(const char* text);
#endif
