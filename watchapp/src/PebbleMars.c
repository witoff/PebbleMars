#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "PebbleMars.h"
#include "base64.h"

//#define MY_UUID { 0x75, 0xA4, 0x95, 0x6D, 0xED, 0xD0, 0x48, 0xB1, 0x89, 0xF8, 0x68, 0xB8, 0x88, 0x13, 0xBB, 0x22 }
//#define MY_UUID { 0xB0, 0x3D, 0x50, 0x95, 0x94, 0xA1, 0x4A, 0x9E, 0xA6, 0xE8, 0xEB, 0x12, 0x79, 0x13, 0xDA, 0x64 }
#define MY_UUID { 0xB0, 0x3D, 0x50, 0x95, 0x94, 0xA1, 0x4A, 0x9E, 0xA6, 0xE8, 0xEB, 0x12, 0x79, 0x13, 0xDA, 0x65 }
#define KEY_IMG_INDEX 420
#define KEY_IMG_DATA  421

PBL_APP_INFO(MY_UUID,
             APP_TITLE, "MakeAwesomeHappen",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

static Window *window;
static BitmapLayer *image_layer_large;
static BitmapLayer *progress_separator;
static TextLayer *time_layer;
static BitmapLayer *separator;
static TextLayer *info_layer;

static void image_update();
static void image_set_uint32(uint16_t index, uint32_t uint32);

void slide_separator(uint8_t to_row) {
  layer_set_frame(bitmap_layer_get_layer(progress_separator), GRect(0, to_row + 27, 144, 1));
  layer_mark_dirty(bitmap_layer_get_layer(progress_separator));
}

size_t process_string(char *str, uint16_t index_start) {
  uint16_t input_len_bytes = strlen(str);
  uint16_t output_len_bytes = 4 * (input_len_bytes/3);

  output_len_bytes += output_len_bytes % 4;

  uint32_t decoded_image[4 * IMAGE_COLS];

  decode_base64(decoded_image, (uint8_t *) str, input_len_bytes);

  for(uint16_t i = 0; i < ARRAY_LENGTH(decoded_image); ++i) {
    image_set_uint32(index_start + i, decoded_image[i]); 
  }

  uint8_t next_row = index_start / IMAGE_COLS + 1;
  slide_separator(next_row); 
  image_update();

  return ARRAY_LENGTH(decoded_image);
}

static uint16_t imgIndex = 0;

void remote_image_data(DictionaryIterator *received) {
  //Tuple *imgIndexTuple;
  Tuple *imgDataTuple;

  //imgIndexTuple = dict_find(received, KEY_IMG_INDEX);
  imgDataTuple = dict_find(received, KEY_IMG_DATA);

  if (imgDataTuple) {
    //int32_t imgIndex = imgIndexTuple->value->int32;
    char *data = imgDataTuple->value->cstring;
    uint16_t length = imgDataTuple->length;

    // APP_LOG(APP_LOG_LEVEL_INFO, "Received image[%li] - %d bytes", imgIndex, length);
    APP_LOG(APP_LOG_LEVEL_INFO, "Index[%i] ==> %s (%d bytes)", imgIndex, data, length);
    imgIndex += process_string(data, imgIndex);
  }
  else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Not a remote-image message");
  }
}

static void image_update() {
  layer_mark_dirty(bitmap_layer_get_layer(image_layer_large));
}

void image_set_uint32(uint16_t index, uint32_t uint32) {
  //Translate the index into a byte address in our bitbuffer
  uint8_t row = index / IMAGE_COLS;
  uint8_t col = index % IMAGE_COLS;
    image_buffer[row][col] = uint32;
}

void image_clear() {
  for (uint16_t j = 0; j < IMAGE_ROWS; j++) {
    for (uint16_t i = 0; i < IMAGE_COLS; i++) {
      image_buffer[j][i] = 0x00000000;
    }
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

void set_info_text(const char *text) {
  static char text_buffer[100];
  snprintf(text_buffer, 100, "%s", text);

  text_layer_set_text(info_layer, text_buffer);
 }

#if SHOW_METADATA

  static Window *more_info_window;
  //static BitmapLayer *image_layer_small;
  static TextLayer *metadata_layer;
  static bool have_metadata;


  static void accel_tap_callback(AccelAxisType axis) {
    have_metadata = true;
    if(axis == ACCEL_AXIS_X && have_metadata) {
      if(more_info_window == window_stack_get_top_window()) {
        window_stack_pop(true);
      } else {
        window_stack_push(more_info_window, true);
      }
    }
  }

  void set_metadata_text(const char* text) {
    text_layer_set_text(metadata_layer, text);
  }
#endif


void app_message_out_sent(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "out_sent");
}

void app_message_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "out_failed reason=%d", reason);
}

void app_message_in_received(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received");

  Tuple *t;
  if ((t = dict_find(received, KEY_TITLE))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Title %s", t->value->cstring);
    set_info_text(t->value->cstring);
    
  }
  if ((t = dict_find(received, KEY_INSTRUMENT))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Instrument %s", t->value->cstring);

  }
  if ((t = dict_find(received, KEY_UTC))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "UTC %s", t->value->cstring);
    // Start a new image when receiving UTC
    imgIndex = 0;
  }
  if ((dict_find(received, KEY_IMG_DATA))) {
    remote_image_data(received);
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

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00";
  char *time_format;

  if (clock_is_24h_style()) 
    time_format = "%R";
  else
    time_format = "%I:%M";

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  text_layer_set_text(time_layer, time_text);
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
  set_info_text(APP_TITLE);


  separator = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 23,
                                          /* width: */ 144, /* height: */ 1));
  bitmap_layer_set_background_color(separator, GColorWhite);




  image_layer_large = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 24,
                                              /* width: */ 144, /* height: */ 144));
  bitmap_layer_set_background_color(image_layer_large, GColorBlack);


  time_layer = text_layer_create(GRect(/* x: */ 0, /* y: */ 134,
                                              /* width: */ 144, /* height: */ 30));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_large));
  layer_add_child(window_layer, bitmap_layer_get_layer(progress_separator));
  layer_add_child(window_layer, text_layer_get_layer(info_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(separator));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  
  image_init();

#if SHOW_METADATA
  more_info_window = window_create();

  Layer *more_info_window_layer = window_get_root_layer(more_info_window);

  metadata_layer = text_layer_create(GRect(/* x: */ 0, /* y: */ 0,
                                            /* width: */ 144, /* height: */ 168));
  text_layer_set_background_color(metadata_layer, GColorBlack);
  text_layer_set_text_color(metadata_layer, GColorWhite);
  text_layer_set_text_alignment(metadata_layer, GTextAlignmentLeft);
  text_layer_set_font(metadata_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  layer_add_child(more_info_window_layer, text_layer_get_layer(metadata_layer));

  accel_tap_service_subscribe(&accel_tap_callback);
#endif

  app_message_register_callbacks(&callbacks);
  app_message_open(124, 124);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Application Started");

  DictionaryIterator *iter;
  app_message_out_get(&iter);
  dict_write_cstring(iter, KEY_HELLO, "Hey dude!");
  dict_write_end(iter);

  app_message_out_send();
  app_message_out_release();
}

void handle_deinit(void) {
#if SHOW_METADATA
  text_layer_destroy(metadata_layer);
  //bitmap_layer_destroy(image_layer_small);
  window_destroy(more_info_window);
#endif

  tick_timer_service_unsubscribe();
  text_layer_destroy(time_layer);
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
