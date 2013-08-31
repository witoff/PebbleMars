#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "PebbleMars.h"

//#define MY_UUID { 0x75, 0xA4, 0x95, 0x6D, 0xED, 0xD0, 0x48, 0xB1, 0x89, 0xF8, 0x68, 0xB8, 0x88, 0x13, 0xBB, 0x22 }
//#define MY_UUID { 0xB0, 0x3D, 0x50, 0x95, 0x94, 0xA1, 0x4A, 0x9E, 0xA6, 0xE8, 0xEB, 0x12, 0x79, 0x13, 0xDA, 0x64 }
#define MY_UUID { 0xB0, 0x3D, 0x50, 0x95, 0x94, 0xA1, 0x4A, 0x9E, 0xA6, 0xE8, 0xEB, 0x12, 0x79, 0x13, 0xDA, 0x65 }
#define KEY_IMG_INDEX 420
#define KEY_IMG_DATA  421

PBL_APP_INFO(MY_UUID,
             "Pebble Mars", "MakeAwesomeHappen",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

static Window *window;
static BitmapLayer *image_layer_large;
static BitmapLayer *separator;
static TextLayer *footer_layer;


int get_char(char c) {
    if (c >= '0' && c <= '9')
        return c-'0';
    if (c >= 'a' && c <= 'f')
      return c-'a' + 10;
    return 0;
}

void process_string(char* str, uint16_t index_start) {
  // hack: skip first char which is an = sign to force the type to be a string...
    for(uint16_t i=1;i<strlen(str);i++) {
        //current byte
        uint8_t b = 0;
        b = get_char(str[i++]) << 4;
        b += get_char(str[i]);

        uint16_t index = index_start + (i-1) / 2;

        set_bitmap_byte(index, b);
    }
    display_new_image();
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
    process_string(data, imgIndex);
    imgIndex += 20;
  }
  else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Not a remote-image message");
  }
}

void display_new_image() {
  static GBitmap bitmap;
    bitmap.bounds = GRect(0, 0, 144, 144);
    bitmap.info_flags = 0x01;
    bitmap.row_size_bytes = 20;
    bitmap.addr = &byte_buffer;

  bitmap_layer_set_bitmap(image_layer_large, &bitmap);
  layer_mark_dirty((Layer*) image_layer_large);
  //clear_bitmap();
}

void set_bitmap_byte(uint16_t index, uint8_t byte) {
  //Translate the index into a byte address in our bitbuffer
  uint8_t row_addr = index / 20;
  uint8_t col_addr = index % 20;

  byte_buffer[row_addr][col_addr] = byte;
}

void clear_bitmap() {
  for(int i = 0; i < 144; ++i) {
    for(int b = 0; b < 20; ++b) {
      byte_buffer[i][b] = 0x00;
    }
  }
}

void set_footer_text(const char *text) {
  static char text_buffer[100];
  snprintf(text_buffer, 100, "%s", text);

  text_layer_set_text(footer_layer, text_buffer);
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
  if ((t = dict_find(received, KEY_NAME))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Name %s", t->value->cstring);
  }
  if ((t = dict_find(received, KEY_INSTRUMENT))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Instrument %s", t->value->cstring);
  }
  if ((t = dict_find(received, KEY_UTC))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "UTC %s", t->value->cstring);
    set_footer_text(t->value->cstring);
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

void handle_init(void) {
   window  = window_create();
  //Pushes window on top of navigation stack, on top of the current top-most window of the app
    //Second arg of window_stack_push indicates whether or not to slide the app window
    //into place over the top of the other apps
  window_stack_push(window, true /* Animated */);
  Layer *window_layer = window_get_root_layer(window);

  image_layer_large = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 0,
                                              /* width: */ 144, /* height: */ 144));
  bitmap_layer_set_background_color(image_layer_large, GColorBlack);

  separator = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 144,
                                          /* width: */ 144, /* height: */ 1));
  bitmap_layer_set_background_color(separator, GColorWhite);

  footer_layer = text_layer_create(GRect(/* x: */ 0, /* y: */ 145,
                                           /* width: */ 144, /* height: */ 23));
  text_layer_set_background_color(footer_layer, GColorBlack);
  text_layer_set_text_color(footer_layer, GColorWhite);
  text_layer_set_text_alignment(footer_layer, GTextAlignmentCenter);
  text_layer_set_font(footer_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));

  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_large));
  layer_add_child(window_layer, bitmap_layer_get_layer(separator));
  layer_add_child(window_layer, text_layer_get_layer(footer_layer));

  ResHandle image_handle = resource_get_handle(RESOURCE_ID_IMAGE_DEFAULT);
  size_t image_res_size = resource_size(image_handle);
  size_t image_header_size = sizeof(GBitmap) - sizeof(void*);
  size_t bytes_copied = resource_load_byte_range(image_handle, image_header_size, (uint8_t*) &byte_buffer, image_res_size - image_header_size);

  display_new_image();

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
  app_message_open(128, 128);

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

  text_layer_destroy(footer_layer);
  bitmap_layer_destroy(separator);
  bitmap_layer_destroy(image_layer_large);

  window_destroy(window);
  app_message_deregister_callbacks(&callbacks);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
