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
static TextLayer *txt_time_years;
static TextLayer *txt_time_years_lbl;
static TextLayer *txt_time_days;
static TextLayer *txt_time_days_lbl;
static TextLayer *txt_time_hours;
static TextLayer *txt_time_hours_lbl;
static TextLayer *txt_time_mins;
static TextLayer *txt_time_mins_lbl;
static TextLayer *txt_time_secs;
static TextLayer *txt_time_secs_lbl;



static char text[35];
static char txt_years[3];
static char txt_days[4];
static char txt_hours[3];
static char txt_mins[3];
static char txt_secs[3];

static void updateTime(delta *d) {
  snprintf(txt_years, 3, "%i", d->years);
  text_layer_set_text(txt_time_years, txt_years);
  snprintf(txt_days, 4, "%i", d->days);
  text_layer_set_text(txt_time_days, txt_days);
  snprintf(txt_hours, 3, "%i", d->hours);
  text_layer_set_text(txt_time_hours, txt_hours);
  snprintf(txt_mins, 3, "%i", d->mins);
  text_layer_set_text(txt_time_mins, txt_mins);
  snprintf(txt_secs, 3, "%i", d->secs);
  text_layer_set_text(txt_time_secs, txt_secs);
}

static delta d;
// SELECT BUTTON ACTION
static void updateSelect() {
  //getDurationString(textTop,  textBottom, 35, getMslEpoch());
  //delta d;
  getDelta(getMslEpoch(), &d);
  updateTime(&d);
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
  //delta d;
  getDelta(getSpiritEpoch(), &d);
  updateTime(&d);
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
  //delta d;
  getDelta(getOppEpoch(), &d);
  updateTime(&d);
  text_layer_set_text(txt_title, "Oppy Duration");
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
                                       /* width: */ 144, /* height: */ 14));
  txt_time_years = text_layer_create(GRect(0,14,144,42));
  txt_time_years_lbl = text_layer_create(GRect(0,56,144,16));
  txt_time_days = text_layer_create(GRect(0,72,144,30));
  txt_time_days_lbl = text_layer_create(GRect(0,102,144,16));
  txt_time_hours = text_layer_create(GRect(0,120,48,14));
  txt_time_hours_lbl = text_layer_create(GRect(0,134,48,14));
  txt_time_mins = text_layer_create(GRect(48,120,48,30));
  txt_time_mins_lbl = text_layer_create(GRect(48,134,48,14));
  txt_time_secs = text_layer_create(GRect(96,120,48,30));
  txt_time_secs_lbl = text_layer_create(GRect(96,134,48,14));

  text_layer_set_text_alignment(txt_title,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_years,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_years_lbl,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_days,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_days_lbl,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_hours,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_hours_lbl,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_mins,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_mins_lbl,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_secs,GTextAlignmentCenter);
  text_layer_set_text_alignment(txt_time_secs_lbl,GTextAlignmentCenter);


  text_layer_set_font(txt_title, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_font(txt_time_years, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_font(txt_time_years_lbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_font(txt_time_days, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(txt_time_days_lbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_font(txt_time_hours, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_font(txt_time_hours_lbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_font(txt_time_mins, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_font(txt_time_mins_lbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_font(txt_time_secs, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_font(txt_time_secs_lbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_title));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_years));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_years_lbl));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_days));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_days_lbl));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_hours));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_hours_lbl));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_mins));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_mins_lbl));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_secs));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(txt_time_secs_lbl));

  // Init static layers
  text_layer_set_text(txt_time_years_lbl, "years");
  text_layer_set_text(txt_time_days_lbl, "days");
  text_layer_set_text(txt_time_hours_lbl, "hours");
  text_layer_set_text(txt_time_mins_lbl, "mins");
  text_layer_set_text(txt_time_secs_lbl, "secs");

  //Kickoff Displays
  select_click_handler(NULL, NULL);
  //updateSelect();

  /*
  text_layer_set_text(txt_title, "JPL Mars Time!");
  text_layer_set_text(txt_time_years, "6");
  text_layer_set_text(txt_time_days, "365");
  text_layer_set_text(txt_time_hours, "11");
  text_layer_set_text(txt_time_mins, "22");
  text_layer_set_text(txt_time_secs, "18");
  */
}

void handle_deinit(void) {
  text_layer_destroy(txt_title);
  text_layer_destroy(txt_time_years);
  text_layer_destroy(txt_time_years_lbl);
  
  window_destroy(window);
}

int main(void) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "main entrance");
  handle_init();
  app_event_loop();
  handle_deinit();
  return 0;
}
