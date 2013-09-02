#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "PebbleMars.h"

void decode_base64(uint32_t dest[IMAGE_COLS], uint8_t *src, const uint16_t src_len);