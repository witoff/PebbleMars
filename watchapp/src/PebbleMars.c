#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "remote-image.h"
#include "PebbleMars.h"

//#define MY_UUID { 0x75, 0xA4, 0x95, 0x6D, 0xED, 0xD0, 0x48, 0xB1, 0x89, 0xF8, 0x68, 0xB8, 0x88, 0x13, 0xBB, 0x22 }
//#define MY_UUID { 0xB0, 0x3D, 0x50, 0x95, 0x94, 0xA1, 0x4A, 0x9E, 0xA6, 0xE8, 0xEB, 0x12, 0x79, 0x13, 0xDA, 0x64 }
#define MY_UUID { 0xB0, 0x3D, 0x50, 0x95, 0x94, 0xA1, 0x4A, 0x9E, 0xA6, 0xE8, 0xEB, 0x12, 0x79, 0x13, 0xDA, 0x65 }


PBL_APP_INFO(MY_UUID,
             "Pebble Mars", "MakeAwesomeHappen",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

static Window *window;
static TextLayer *text_layer;

void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Select pressed - sending message");
  text_layer_set_text(text_layer, "Select");

  DictionaryIterator *iter;
  app_message_out_get(&iter);
  dict_write_cstring(iter, KEY_HELLO, "Hey dude!");
  dict_write_end(iter);

  app_message_out_send();
  app_message_out_release();
}

void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up");
}

void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Down");
}

void config_provider(ClickConfig **config, Window *window) {
  config[BUTTON_ID_SELECT]->click.handler = select_click_handler;
  config[BUTTON_ID_UP]->click.handler = up_click_handler;
  config[BUTTON_ID_DOWN]->click.handler = down_click_handler;
}

void app_message_out_sent(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "out_sent");
}

void app_message_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "out_failed reason=%d", reason);
}

void app_message_in_received(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received");

  if (dict_find(received, KEY_IMG_INDEX)) {
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
  window = window_create();
  window_stack_push(window, true /* Animated */);

  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);

  text_layer = text_layer_create(GRect(/* x: */ 0, /* y: */ 74,
                                       /* width: */ 144, /* height: */ 20));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));

  text_layer_set_text(text_layer, "Press a button");

  app_message_register_callbacks(&callbacks);
  app_message_open(100, 100);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Application Started");
}

void handle_deinit(void) {
  text_layer_destroy(text_layer);
  window_destroy(window);
  app_message_deregister_callbacks(&callbacks);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
