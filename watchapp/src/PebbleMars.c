#include <pebble.h>
#include "PebbleMars.h"
#include "base64.h"
#include "ui.h"
#include "marstime.h"

uint8_t image_next_chunk_id;
bool image_chunk_marks[IMAGE_CHUNKS];
bool image_receiving;

static void image_mark_chunk(uint8_t);
static void image_cache_chunk(uint8_t chunk_id, uint32_t *bmp, uint16_t len);
static bool image_check_chunks();
static void image_start_transfer();
static void image_complete_transfer();
static void image_check_chunks_timer_callback(void *);

static uint32_t lmst_update_interval;

void send_app_message(SendCallback callback, void *data) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  callback(iter, data);
  dict_write_end(iter);

  app_message_outbox_send();
}

void send_uint8(DictionaryIterator *iter, void *data) {
  Tuplet *tuplet = (Tuplet*) data;
  dict_write_uint8(iter, tuplet->key, tuplet->integer.storage);
}

size_t process_string(char *str) {
  uint16_t input_len_bytes = strlen(str);
  uint16_t output_len_bytes = 4 * (input_len_bytes/3);

  output_len_bytes += output_len_bytes % 4;

  struct {
    uint8_t id;
    uint32_t bmp[IMAGE_CHUNK_SIZE];
  } __attribute__((__packed__)) image_chunk;

  decode_base64((uint8_t *) &image_chunk, (uint8_t *) str, input_len_bytes);

  if (image_chunk.id >= IMAGE_CHUNKS) {
    return 0;
  }

  size_t chunk_offset = image_chunk.id * IMAGE_CHUNK_SIZE;
  for (uint16_t i = 0; i < ARRAY_LENGTH(image_chunk.bmp); i++) {
    mars_image_set_dword(chunk_offset + i, image_chunk.bmp[i]);
  }

  image_mark_chunk(image_chunk.id);
	
  image_cache_chunk(image_chunk.id, image_chunk.bmp, ARRAY_LENGTH(image_chunk.bmp));

  uint8_t next_row = image_next_chunk_id * IMAGE_CHUNK_SIZE / IMAGE_COLS;
  slide_progress_separator(next_row);
  mars_image_mark_dirty();

  return ARRAY_LENGTH(image_chunk.bmp);
}

void remote_image_data(DictionaryIterator *received) {
  Tuple *image_data_tuple;
  image_data_tuple = dict_find(received, KEY_IMAGE_DATA);
  if (image_data_tuple) {
    char *data = image_data_tuple->value->cstring;

    //uint16_t length = image_data_tuple->length;
    //APP_LOG(APP_LOG_LEVEL_INFO, "Index[%i] ==> %s (%d bytes)", imgIndex, data, length);
    process_string(data);

    image_check_chunks();
  }
  else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Not a remote-image message");
  }
}

static void image_start_transfer() {
  image_receiving = true;
  image_next_chunk_id = 0;
  memset(image_chunk_marks, 0, sizeof(image_chunk_marks));
  Tuplet tuplet = TupletInteger(KEY_IMAGE_START, 0);
  send_app_message(send_uint8, &tuplet);
  //image_check_chunks_timer_callback(NULL);
}

static void image_complete_transfer() {
  if (!image_receiving) {
   return;
  }
  image_receiving = false;
  Tuplet tuplet = TupletInteger(KEY_IMAGE_COMPLETE, 0);
  send_app_message(send_uint8, &tuplet);
}

static void image_mark_chunk(uint8_t chunk_id) {
  if (chunk_id >= image_next_chunk_id) {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "mark next %d %d", (int) chunk_id, (int) image_next_chunk_id);
    image_next_chunk_id = chunk_id+1;
  }
  image_chunk_marks[chunk_id] = true;
}

static void image_cache_chunk(uint8_t chunk_id, uint32_t *bmp, uint16_t len) {
  //persist_delete(PERSIST_IMG_CACHE_KEY_BASE+chunk_id);
  //persist_write_data(PERSIST_IMG_CACHE_KEY_BASE+chunk_id, bmp, len);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Cached chunk ID: %d (%d bytes) with return status: %d", (int)chunk_id, len, status);
}

static int16_t image_get_next_chunk_id() {
  for (uint8_t i = 0; i < image_next_chunk_id && i < IMAGE_CHUNKS; i++) {
    if (!image_chunk_marks[i]) {
      return i;
    }
  }
  if (image_next_chunk_id < IMAGE_CHUNKS) {
    return image_next_chunk_id;
  }
  return -1;
}

static bool image_check_chunks() {
  int16_t next_chunk_id = image_get_next_chunk_id();
  if (next_chunk_id < 0) {
    image_complete_transfer();
    return true;
  }

  if (next_chunk_id == image_next_chunk_id) {
    return false;
  }

  Tuplet tuplet = TupletInteger(KEY_IMAGE_REQUEST_CHUNK, next_chunk_id);
  send_app_message(send_uint8, &tuplet);
  return false;
}

static void image_check_chunks_timer_callback(void *data) {
  if (!image_check_chunks()) {
    //light_enable_interaction();
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "timer next: %d", (int) image_next_chunk_id);
    app_timer_register(50, image_check_chunks_timer_callback, NULL);
  } else {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "timer done");
    image_complete_transfer();
  }
}

void app_message_out_sent(DictionaryIterator *sent, void *context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "out_sent");
}

void app_message_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "out_failed reason=%d", reason);
}

void app_message_in_received(DictionaryIterator *received, void *context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received");

  Tuple *t;
  if ((t = dict_find(received, KEY_TEMPERATURE))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature %s", t->value->cstring);
    set_info_text(KEY_TEMPERATURE, t->value->cstring);
  }
  if ((t = dict_find(received, KEY_REL_TIME))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Relative Time %s", t->value->cstring);
    set_info_text(KEY_REL_TIME, t->value->cstring);
  }
  if ((t = dict_find(received, KEY_INSTRUMENT))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Instrument %s", t->value->cstring);
    set_info_text(KEY_INSTRUMENT, t->value->cstring);

  }
  if ((t = dict_find(received, KEY_UTC))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "UTC %s", t->value->cstring);
    set_info_text(KEY_UTC, t->value->cstring);
    // Start a new image when receiving UTC
    image_start_transfer();
  }
  if ((t = dict_find(received, KEY_FILENAME))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Filename %s", t->value->cstring);
    set_info_text(KEY_FILENAME, t->value->cstring);
  }
  if ((t = dict_find(received, KEY_IMAGE_DATA))) {
    remote_image_data(received);
    app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  }
  if ((t = dict_find(received, KEY_IMAGE_COMPLETE))) {
    // This is a query, but we'll stop for now
    image_complete_transfer();
  }
  if ((t = dict_find(received, KEY_TZ_OFFSET))) {
	int32_t value = t->value->int32;
	
	// Save value to persistent storage.
	int tz_key_status = persist_write_int(PERSIST_TZ_KEY, value);
	int tz_ttl_key_status = persist_write_int(PERSIST_TZ_TTL_KEY, time(NULL));

    APP_LOG(APP_LOG_LEVEL_DEBUG, "TZ Offset from JS: %i, persist write status: %d,%d", (int)value, tz_key_status, tz_ttl_key_status);
  }
}

void app_message_in_dropped(void *context, AppMessageResult reason) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "in_dropped reason=%d", reason);
}

static void handle_accel_tap(AccelAxisType axis, int32_t direction) {
  image_complete_transfer();
  Tuplet tuplet = TupletInteger(KEY_IMAGE_REQUEST_NEXT, 0);
  send_app_message(send_uint8, &tuplet);
}

static void update_lmst(uint32_t *ms) {
  static char lmst_string[20];
  getMarsTimeString(lmst_string, 20, getMslEpoch(), 0);
  text_layer_set_text(lmst_layer, lmst_string);
  app_timer_register(*ms, (AppTimerCallback) update_lmst, ms);
}

void handle_init(void) {
  ui_init();

  app_message_register_inbox_received(app_message_in_received);
  app_message_register_outbox_sent(app_message_out_sent);
  app_message_register_outbox_failed(app_message_out_failed);

  app_message_open(124, 124);

  accel_tap_service_subscribe(handle_accel_tap);
	
  lmst_update_interval = 10274;
  update_lmst(&lmst_update_interval);
	
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Application Started");
}

void handle_deinit(void) {
  ui_deinit();
  app_message_deregister_callbacks();
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
