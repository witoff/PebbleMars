#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "PebbleMars.h"
#include "ui.h"

static Window      *window;

static Layer       *time_layer;

static BitmapLayer *mars_image_layer;
static BitmapLayer *separator;
static BitmapLayer *progress_separator;

static TextLayer   *info_layer;

static char info_text[NUM_KEYS][INFO_BUFFER_LEN];
static uint32_t swap_delay;

static uint32_t mars_image_buffer[IMAGE_ROWS][IMAGE_COLS];
static GBitmap mars_image_bitmap;

//Initialize the memory for the mars image and load a default one before telling Pebble to update the screen
static void mars_image_init() {
  mars_image_bitmap = (GBitmap) {
    .addr = mars_image_buffer,
    .row_size_bytes = 20,
    .info_flags = 0x01,
    .bounds = GRect(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT)
  };

  bitmap_layer_set_bitmap(mars_image_layer, &mars_image_bitmap);

  const ResHandle image_handle = resource_get_handle(RESOURCE_ID_IMAGE_DEFAULT);
  const size_t image_res_size = resource_size(image_handle);
  const size_t image_header_size = sizeof(GBitmap) - sizeof(void *);
  resource_load_byte_range(image_handle, image_header_size, (uint8_t *) mars_image_buffer, image_res_size - image_header_size);

  mars_image_mark_dirty();
}

static char time_text[6];
static void update_time_display_callback(Layer *layer, GContext *ctx) {
  graphics_context_set_text_color(ctx, GColorBlack);
  GRect bounds = GRect(0, 0, 144, 30);

  //Todo: Make it so this isn't called every minute we want to update the display callback
  GRect back_bounds[4];
  for(uint8_t i = 0; i < 4; i++) {
    back_bounds[i] = bounds;
  }

  back_bounds[0].origin.x++; back_bounds[0].origin.y++;
  back_bounds[1].origin.x++; back_bounds[1].origin.y--;
  back_bounds[2].origin.x--; back_bounds[2].origin.y++;
  back_bounds[3].origin.x--; back_bounds[3].origin.y--;

  //Draw the background outline text
  //Todo: Is there a better way to draw a 1px border?
  for(uint8_t i = 0; i < 4; ++i) {
    graphics_text_draw(
      ctx,
      time_text,
      fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
      back_bounds[i],
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL);
  }

  //Draw the time text on top of the outline
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_text_draw(
    ctx,
    time_text,
    fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
    bounds,
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentCenter,
    NULL);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  char *time_format;

  if(clock_is_24h_style())
    time_format = "%R";
  else
    time_format = "%I:%M";

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  //Get Pebble to call our update_time_display_callback()
  layer_mark_dirty(time_layer);
}

static void set_next_info_text(uint32_t *ms) {
  static uint8_t t = KEY_APP_TITLE;
  if(t > KEY_INSTRUMENT) t = KEY_APP_TITLE;
  text_layer_set_text(info_layer, info_text[t++]);

  app_timer_register(*ms, (AppTimerCallback) set_next_info_text, ms);
}

void slide_progress_separator(uint8_t to_row) {
  layer_set_frame(bitmap_layer_get_layer(progress_separator), GRect(0, to_row + 24, IMAGE_WIDTH, 1));
  layer_mark_dirty(bitmap_layer_get_layer(progress_separator));
}

void mars_image_set_dword(uint16_t index, uint32_t dword) {
  //translate the index into a dword address in mars_image_buffer
  uint8_t row = index / IMAGE_COLS;
  uint8_t col = index % IMAGE_COLS;

  mars_image_buffer[row][col] = dword;
}

void mars_image_mark_dirty() {
  layer_mark_dirty(bitmap_layer_get_layer(mars_image_layer));
}

void mars_image_clear() {
  for(uint16_t j = 0; j < IMAGE_ROWS; ++j) {
    for(uint16_t i = 0; i < IMAGE_COLS; ++i) {
      mars_image_buffer[j][i] = 0x00000000;
    }
  }
}

void set_info_text(uint8_t key, const char *text) {
  //Todo: Make info text storage dynamic?
  //Todo: Check this for memory safety
  snprintf(info_text[key], 100, "%s", text);

  //Todo: Split functionality of setting info text away from loading it into the text store
  text_layer_set_text(info_layer, info_text[key]);
}

void ui_init() {
  window = window_create();
  window_stack_push(window, /* Animate? */ true);
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
  set_info_text(2, "To Get the Latest");
  set_info_text(3, "Twist Your Wrist");
  //unreachable at this time
  set_info_text(4, "Feel the Greatest");
  set_info_text(5, "Burma-Shave");


  separator = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 23,
                                        /* width: */ 144, /* height: */ 1));
  bitmap_layer_set_background_color(separator, GColorWhite);


  mars_image_layer = bitmap_layer_create(GRect(/* x: */ 0, /* y: */ 24,
                                                /* width: */ 144, /* height: */ 144));
  mars_image_init();


  time_layer = layer_create(GRect(/* x: */ 0, /* y: */ 134,
                                  /* width: */ 144, /* height: */ 32));
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  layer_set_update_proc(time_layer, update_time_display_callback);

  swap_delay = 5500;
  set_next_info_text(&swap_delay);

  layer_add_child(window_layer, bitmap_layer_get_layer(mars_image_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(progress_separator));
  layer_add_child(window_layer, text_layer_get_layer(info_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(separator));
  layer_add_child(window_layer, time_layer);


  
}

void ui_deinit() {
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  layer_destroy(time_layer); 
  bitmap_layer_destroy(progress_separator);
  bitmap_layer_destroy(mars_image_layer);
  bitmap_layer_destroy(separator);
  text_layer_destroy(info_layer);

  window_destroy(window);
}