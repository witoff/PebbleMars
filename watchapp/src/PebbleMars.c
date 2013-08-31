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
static Window *more_info_window;
//static BitmapLayer *image_layer_small;
static TextLayer *metadata_layer;
static bool have_metadata;


void remote_image_data(DictionaryIterator *received) {
  Tuple *imgIndexTuple, *imgDataTuple;

  imgIndexTuple = dict_find(received, KEY_IMG_INDEX);
  imgDataTuple = dict_find(received, KEY_IMG_DATA);

  if (imgIndexTuple && imgDataTuple) {
    int32_t imgIndex = imgIndexTuple->value->int32;
    uint8_t *data = imgDataTuple->value->data;
    uint16_t length = imgDataTuple->length;

    // APP_LOG(APP_LOG_LEVEL_INFO, "Received image[%li] - %d bytes", imgIndex, length);
    APP_LOG(APP_LOG_LEVEL_INFO, "Index[%li] ==> %u (%d bytes)", imgIndex, data[0], length);
    for (int i = 0; i < length; i++) {
      APP_LOG(APP_LOG_LEVEL_INFO, "==> %li %i", imgIndex + i, data[i]);


    }
  }
  else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Not a remote-image message");
  }
}


static void accel_tap_callback(AccelAxisType axis) {
  if(axis == ACCEL_AXIS_X && have_metadata) {
    if(more_info_window == window_stack_get_top_window()) {
      window_stack_pop(true);
    } else {
      window_stack_push(more_info_window, true);
    }
  }
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
  if ((t = dict_find(received, KEY_NAME))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Name %s", t->value->cstring);
  }
  if ((t = dict_find(received, KEY_INSTRUMENT))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Instrument %s", t->value->cstring);
  }
  if ((t = dict_find(received, KEY_UTC))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "UTC %s", t->value->cstring);
  }

  if ((dict_find(received, KEY_IMG_INDEX))) {
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

  separator = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 144,
                                          /* width: */ 144, /* height: */ 1));
  bitmap_layer_set_background_color(separator, GColorWhite);

  footer_layer = text_layer_create(GRect(/* x: */ 0, /* y: */ 145,
                                           /* width: */ 144, /* height: */ 23));


  more_info_window = window_create();

  Layer *more_info_window_layer = window_get_root_layer(more_info_window);

  metadata_layer = text_layer_create(GRect(/* x: */ 0, /* y: */ 0,
                                            /* width: */ 144, /* height: */ 168));


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
  text_layer_destroy(metadata_layer);
  //bitmap_layer_destroy(image_layer_small);
  window_destroy(more_info_window);

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
