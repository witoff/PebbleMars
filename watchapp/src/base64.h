#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

void decode_base64(uint32_t *dest, const uint8_t *src, const uint16_t src_len);