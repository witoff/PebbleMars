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
static TextLayer *txt_title;
static TextLayer *txt_time_top;
static TextLayer *txt_time_bottom;

static char text[35];
static char textTop[35];
static char textBottom[35];

// SELECT BUTTON ACTION
static void updateSelect() {
  getDurationString(textTop,  textBottom, 35, getMslEpoch());
  text_layer_set_text(txt_time_top, textTop);
  text_layer_set_text(txt_time_bottom, textBottom);
  text_layer_set_text(txt_title, "MSL Duration");
}
static void handle_minute_tick_TimeSinceLanding(struct tm *tick_time, TimeUnits units_changed) {
  updateSelect();
}
void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "select_click_handler");
  tick_timer_service_unsubscribe(); 
  updateSelect();
  tick_timer_service_subscribe(SECOND_UNIT, &handle_minute_tick_TimeSinceLanding);
}

// UP BUTTON ACTION
static void updateCurrentTimeString() {
  //getCurrentTimeString( text,  35);
  getDurationString(textTop, textBottom,  35, getSpiritEpoch());
  text_layer_set_text(txt_time_top, textTop);
  text_layer_set_text(txt_time_bottom, textBottom);
  text_layer_set_text(txt_title, "Spirit Duration");
}
static void handle_minute_tick_CurrentTime(struct tm *tick_time, TimeUnits units_changed) {
  updateCurrentTimeString();
}
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "up_click_handler");
  tick_timer_service_unsubscribe();
  updateCurrentTimeString();
  tick_timer_service_subscribe(SECOND_UNIT, &handle_minute_tick_CurrentTime);
  
}

// DOWN BUTTON ACTION
static void updateDown() {
  getDurationString(textTop,  textBottom, 35, getOppEpoch());
  text_layer_set_text(txt_title, "Oppy Duration");
  text_layer_set_text(txt_time_top, textTop);
  text_layer_set_text(txt_time_bottom, textBottom);
}
static void handle_minute_tick_Down(struct tm *tick_time, TimeUnits units_changed) {
  updateDown();
}
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "down_click_handler");
  tick_timer_service_unsubscribe(); 
  updateDown();
  tick_timer_service_subscribe(SECOND_UNIT, &handle_minute_tick_Down);
}


// CONFIG
void config_provider(ClickConfig **config, Window *window) {
  config[BUTTON_ID_SELECT]->click.handler = select_click_handler;
  config[BUTTON_ID_UP]->click.handler = up_click_handler;
  config[BUTTON_ID_DOWN]->click.handler = down_click_handler;
}


void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);

  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);

  txt_title = text_layer_create(GRect(/* x: */ 0, /* y: */ 0,
                                       /* width: */ 144, /* height: */ 30));
  txt_time_top = text_layer_create(GRect(/* x: */ 0, /* y: */ 31,
                                       /* width: */ 144, /* height: */ 60));
  txt_time_bottom = text_layer_create(GRect(/* x: */ 0, /* y: */ 102,
                                       /* width: */ 144, /* height: */ 40));

  text_layer_set_font(txt_title, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_font(txt_time_top, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_font(txt_time_bottom, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_title));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_top));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_bottom));
  

  text_layer_set_text(txt_title, "JPL Mars Time!");
  //text_layer_set_text(txt_time_top, "Welcome!");
  //text_layer_set_text(txt_time_bottom, "By R. Witoff");

  //DEBUG
  getDurationString(textTop, textBottom,  35, getMslEpoch());
  text_layer_set_text(txt_time_top, textTop);
  text_layer_set_text(txt_time_bottom, textBottom);
}

void handle_deinit(void) {
  text_layer_destroy(txt_title);
  text_layer_destroy(txt_time_top);
  text_layer_destroy(txt_time_bottom);
  
  window_destroy(window);
}

int main(void) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "main entrance");
  handle_init();
  app_event_loop();
  handle_deinit();
  return 0;
}
