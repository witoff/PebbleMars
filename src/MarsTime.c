#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "util.h"


#define MY_UUID { 0xE2, 0xB9, 0x14, 0x46, 0x44, 0x2F, 0x45, 0xC6, 0xA8, 0x00, 0x4F, 0xA0, 0x83, 0xA0, 0xF8, 0x25 }
PBL_APP_INFO(MY_UUID,
             "JPL Mars Time", "JPL",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

static Window *window;
static TextLayer *txt_time_layer;
static TextLayer *txt_title;

static char text[30];

static void updateTimeSinceLanding() {
  getTimeSinceLandingString( text,  30);
  text_layer_set_text(txt_time_layer, text);
  text_layer_set_text(txt_title, "MSL Duration");
}
static void handle_minute_tick_TimeSinceLanding(struct tm *tick_time, TimeUnits units_changed) {
  updateTimeSinceLanding();
}
void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  tick_timer_service_unsubscribe(); 
  updateTimeSinceLanding();
  tick_timer_service_subscribe(SECOND_UNIT, &handle_minute_tick_TimeSinceLanding);
}


static void updateCurrentTimeString() {
  getCurrentTimeString( text,  30);
  text_layer_set_text(txt_time_layer, text);
  text_layer_set_text(txt_title, "Current Time");
}
static void handle_minute_tick_CurrentTime(struct tm *tick_time, TimeUnits units_changed) {
  updateCurrentTimeString();
}
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  tick_timer_service_unsubscribe();
  updateCurrentTimeString();
  tick_timer_service_subscribe(SECOND_UNIT, &handle_minute_tick_CurrentTime);
  
}

void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  tick_timer_service_unsubscribe();
  getMslEpochString( text,  30);
  text_layer_set_text(txt_title, "Landing Time");
  text_layer_set_text(txt_time_layer, text);
}

void config_provider(ClickConfig **config, Window *window) {
  config[BUTTON_ID_SELECT]->click.handler = select_click_handler;
  config[BUTTON_ID_UP]->click.handler = up_click_handler;
  config[BUTTON_ID_DOWN]->click.handler = down_click_handler;
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);

  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);

  txt_time_layer = text_layer_create(GRect(/* x: */ 0, /* y: */ 74,
                                       /* width: */ 144, /* height: */ 20));
  txt_title = text_layer_create(GRect(/* x: */ 0, /* y: */ 20,
                                       /* width: */ 144, /* height: */ 40));
  text_layer_set_font(txt_title, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_title));

  text_layer_set_text(txt_title, "JPL Mars Time!");
}

void handle_deinit(void) {
  text_layer_destroy(txt_time_layer);
  text_layer_destroy(txt_title);
  window_destroy(window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
  return 0;
}
