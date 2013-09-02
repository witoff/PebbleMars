#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "PebbleMars.h"
#include "base64.h"

#define MY_UUID { 0x75, 0xA4, 0x95, 0x6D, 0xED, 0xD0, 0x48, 0xB1, 0x89, 0xF8, 0x68, 0xB8, 0x88, 0x13, 0xBB, 0x22 }

PBL_APP_INFO(MY_UUID,
             APP_TITLE, "MakeAwesomeHappen",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

static Window *window;
static BitmapLayer *image_layer_large;
static BitmapLayer *progress_separator;
static Layer *time_layer;
static BitmapLayer *separator;
static TextLayer *info_layer;
static char info_text[NUM_KEYS][INFO_BUFFER_LEN];
static uint32_t swap_delay;

uint32_t image_buffer[IMAGE_ROWS][IMAGE_COLS];
GBitmap image_bitmap;
uint8_t image_next_chunk_id;
bool image_chunk_marks[IMAGE_CHUNKS];
bool image_receiving;

static void image_update();
static void image_set_uint32(uint16_t index, uint32_t uint32);
static void image_mark_chunk(uint8_t chunk_id);
static bool image_check_chunks();
static void image_start_transfer();
static void image_complete_transfer();
static void image_check_chunks_timer_callback(void *data);

typedef void (*SendCallback)(DictionaryIterator *iter, void *data);

static void send_app_message(SendCallback callback, void *data) {
  DictionaryIterator *iter;
  app_message_out_get(&iter);

  callback(iter, data);
  dict_write_end(iter);

  app_message_out_send();
  app_message_out_release();
}

static void send_uint8(DictionaryIterator *iter, void *data) {
  Tuplet *tuplet = (Tuplet*) data;
  dict_write_uint8(iter, tuplet->key, tuplet->integer.storage);
}

void slide_separator(uint8_t to_row) {
  layer_set_frame(bitmap_layer_get_layer(progress_separator), GRect(0, to_row + 27, 144, 1));
  layer_mark_dirty(bitmap_layer_get_layer(progress_separator));
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
    image_set_uint32(chunk_offset + i, image_chunk.bmp[i]); 
  }

  image_mark_chunk(image_chunk.id);

  uint8_t next_row = image_next_chunk_id * IMAGE_CHUNK_SIZE / IMAGE_COLS;
  slide_separator(next_row); 
  image_update();

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

    if (image_check_chunks()) {
      image_complete_transfer();
    }
  }
  else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Not a remote-image message");
  }
}

static void image_update() {
  layer_mark_dirty(bitmap_layer_get_layer(image_layer_large));
}

static void image_set_uint32(uint16_t index, uint32_t uint32) {
  //Translate the index into a byte address in our bitbuffer
  uint8_t row = index / IMAGE_COLS;
  uint8_t col = index % IMAGE_COLS;
    image_buffer[row][col] = uint32;
}

static void image_clear() {
  for (uint16_t j = 0; j < IMAGE_ROWS; j++) {
    for (uint16_t i = 0; i < IMAGE_COLS; i++) {
      image_buffer[j][i] = 0x00000000;
    }
  }
}

static void image_start_transfer() {
  image_receiving = false;
  image_next_chunk_id = 0;
  memset(image_chunk_marks, 0, sizeof(image_chunk_marks));
  Tuplet tuplet = TupletInteger(KEY_IMAGE_START, 0);
  send_app_message(send_uint8, &tuplet);
  //image_check_chunks_timer_callback(NULL);
}

static void image_complete_transfer() {
  if (image_receiving) {
   return;
  }
  image_receiving = true;
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

static void image_init() {
  image_bitmap = (GBitmap) {
    .addr = image_buffer,
    .row_size_bytes = 20,
    .info_flags = 0x01,
    .bounds = GRect(0, 0, 144, 144),
  };
  bitmap_layer_set_bitmap(image_layer_large, &image_bitmap);

  const ResHandle image_handle = resource_get_handle(RESOURCE_ID_IMAGE_DEFAULT);
  const size_t image_res_size = resource_size(image_handle);
  const size_t image_header_size = sizeof(GBitmap) - sizeof(void*);
  resource_load_byte_range(image_handle, image_header_size, (uint8_t*) image_buffer, image_res_size - image_header_size);

  image_update();
}

void set_info_text(uint8_t key, const char *text) {
  snprintf(info_text[key], 100, "%s", text);

  text_layer_set_text(info_layer, info_text[key]);
}




void app_message_out_sent(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "out_sent");
}

void app_message_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "out_failed reason=%d", reason);
}

void app_message_in_received(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received");

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
}

void app_message_in_dropped(void *context, AppMessageResult reason) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "in_dropped reason=%d", reason);
}

static AppMessageCallbacksNode callbacks = {
  .context = NULL,
  .callbacks = {
    .out_sent = app_message_out_sent,
    .out_failed = app_message_out_failed,
    .in_received = app_message_in_received,
    .in_dropped = app_message_in_dropped
  }
};

static char time_text[6];
static void update_time_display_callback(Layer *layer, GContext *ctx) {
  graphics_context_set_text_color(ctx, GColorBlack);
  GRect bounds = GRect(0, 0, 144, 32);
  //bounds.size.h -= 2;
  //graphics_context_set_text_color(ctx, GColorWhite);
  
  //GRect bounds = layer_get_frame(layer);
  GRect back_bounds[4];
  for(uint8_t i = 0; i < 4; i++) {
    back_bounds[i] = bounds;
  }

  back_bounds[0].origin.x++; back_bounds[0].origin.y++;
  back_bounds[1].origin.x++; back_bounds[1].origin.y--;
  back_bounds[2].origin.x--; back_bounds[2].origin.y++;
  back_bounds[3].origin.x--; back_bounds[3].origin.y--;

  for(uint8_t i = 0; i < 4; i++) {
    graphics_text_draw(ctx,
      time_text,
      fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
      back_bounds[i],
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL);
  }
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_text_draw(ctx,
      time_text,
      fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
      bounds,
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL);
}

static void swap_info(uint32_t *ms) {
  static uint8_t t = KEY_APP_TITLE;
  if(t > KEY_INSTRUMENT) t = KEY_APP_TITLE;
  text_layer_set_text(info_layer, info_text[t++]);

  app_timer_register(*ms, (AppTimerCallback) swap_info, ms);
}

static void handle_accel_tap(AccelAxisType axis) {
  if (!image_receiving) {
    return;
  }
  Tuplet tuplet = TupletInteger(KEY_IMAGE_REQUEST_NEXT, 0);
  send_app_message(send_uint8, &tuplet);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  char *time_format;

  if (clock_is_24h_style()) 
    time_format = "%R";
  else
    time_format = "%I:%M";

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  layer_mark_dirty(time_layer);
}

void handle_init(void) {
  window  = window_create();
  //Pushes window on top of navigation stack, on top of the current top-most window of the app
    //Second arg of window_stack_push indicates whether or not to slide the app window
    //into place over the top of the other apps
  window_stack_push(window, true /* Animated */);
  Layer *window_layer = window_get_root_layer(window);

  progress_separator = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 0,
                                          /* width: */ 144, /* height: */ 1));
  bitmap_layer_set_background_color(progress_separator, GColorBlack);

  info_layer = text_layer_create(GRect(/* x: */ 0, /* y: */ 0,
                                           /* width: */ 144, /* height: */ 23));
  text_layer_set_background_color(info_layer, GColorBlack);
  text_layer_set_text_color(info_layer, GColorWhite);
  text_layer_set_text_alignment(info_layer, GTextAlignmentCenter);
  text_layer_set_font(info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  set_info_text(KEY_APP_TITLE, APP_TITLE);
  set_info_text(1, "Curiosity Rover:");
  set_info_text(2, "Twist Your Wrist");
  set_info_text(3, "To Get the Latest");
  //unreachable at this time
  set_info_text(4, "And Greatest");
  set_info_text(5, "Burma Shave");


  separator = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 23,
                                          /* width: */ 144, /* height: */ 1));
  bitmap_layer_set_background_color(separator, GColorWhite);


  image_layer_large = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 24,
                                              /* width: */ 144, /* height: */ 144));
  bitmap_layer_set_background_color(image_layer_large, GColorBlack);


  time_layer = layer_create(GRect(/* x: */ 0, /* y: */ 134,
                                              /* width: */ 144, /* height: */ 32));
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  layer_set_update_proc(time_layer, update_time_display_callback);

  swap_delay = 5500;
  swap_info(&swap_delay);

  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_large));
  layer_add_child(window_layer, bitmap_layer_get_layer(progress_separator));
  layer_add_child(window_layer, text_layer_get_layer(info_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(separator));
  layer_add_child(window_layer, time_layer);

  app_message_register_callbacks(&callbacks);
  app_message_open(124, 124);

  image_init();

  accel_tap_service_subscribe(handle_accel_tap);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Application Started");
}

void handle_deinit(void) {
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  layer_destroy(time_layer); 
  bitmap_layer_destroy(progress_separator);
  bitmap_layer_destroy(image_layer_large);
  bitmap_layer_destroy(separator);
  text_layer_destroy(info_layer);

  window_destroy(window);
  app_message_deregister_callbacks(&callbacks);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
