#pragma once

#include <pebble.h>

#define APP_TITLE "Pebble Mars"

#define APP_MESSAGE_BUF_IN 124
#define APP_MESSAGE_BUF_OUT 124

#define NUM_KEYS 6

#define KEY_APP_TITLE 0
#define KEY_TEMPERATURE 1
#define KEY_REL_TIME 2
#define KEY_INSTRUMENT 3
#define KEY_UTC 4
#define KEY_FILENAME 5

#define INFO_BUFFER_LEN 100

#define KEY_IMAGE_START 419
#define KEY_IMAGE_INDEX 420
#define KEY_IMAGE_DATA  421
#define KEY_IMAGE_COMPLETE 422
#define KEY_IMAGE_REQUEST_CHUNK 423
#define KEY_IMAGE_REQUEST_NEXT 424
#define KEY_TZ_OFFSET 425
	
#define PERSIST_TZ_KEY 42
#define PERSIST_TZ_TTL_KEY 43
#define PERSIST_IMG_CACHE_KEY_BASE 200

#define IMAGE_WIDTH 144
#define IMAGE_HEIGHT 144
#define IMAGE_COLS ((IMAGE_WIDTH - 1) / (8 * sizeof(uint32_t)) + 1)
#define IMAGE_ROWS (IMAGE_HEIGHT)

#define IMAGE_CHUNKS ((IMAGE_ROWS - 1) / 4 + 1)
#define IMAGE_CHUNK_SIZE (4 * IMAGE_COLS)
	
extern uint32_t image_buffer[IMAGE_ROWS][IMAGE_COLS];
extern GBitmap image_bitmap;
extern uint8_t image_next_chunk_id;
extern bool image_chunk_marks[IMAGE_CHUNKS];

typedef void (*SendCallback)(DictionaryIterator *iter, void *data);

void send_app_message(SendCallback callback, void *data);
void send_uint8(DictionaryIterator *iter, void *data);

void display_new_image();
void set_info_text(uint8_t key, const char *text);

void set_bitmap_byte(uint16_t index, uint8_t byte);
void clear_bitmap();

#if SHOW_METADATA
void set_metadata_text(const char* text);
#endif
