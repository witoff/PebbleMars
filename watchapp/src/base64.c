#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "PebbleMars.h"
#include "base64.h"

static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

static void decode_block(uint8_t *in, uint8_t *out) {
	out[0] = in[0] << 2 | in[1] >> 4;
	out[1] = in[1] << 4 | in[2] >> 2;
	out[2] = (((in[2] << 6) & 0xc0) | in[3]); 
}

void decode_base64(uint8_t *dest, uint8_t *src, const uint16_t src_len) {
	uint8_t in[4] = {0, 0, 0, 0};
	uint8_t out[3]= {0, 0, 0};

	uint16_t src_length = src_len;

	uint8_t *src_p = src;
	uint8_t *dest_p = dest;

	uint8_t v;
	uint16_t i, len;

	while(src_length > 0) {
		len = 0;
		for(i = 0; i < 4 && src_length > 0; i++) {
			v = 0;
			while(src_length > 0 && v == 0) {
				v = *src_p++;
				src_length--;
				v = ((v < 43 || v > 122) ? 0 : (uint8_t) cd64[ v - 43 ]);
				if(v != 0) {
					v = ((v == (uint8_t)'$') ? 0 : v - 61);
				}
			}
			if(src_length > 0) {
				len++;
				if(v != 0) {
					in[i] = v - 1;
				}
			} else {
				in[i] = 0;
			}
		}
		if(len > 0) {
			decode_block(in, out);
			for( i = 0; i < len - 1; i++) {
				*dest_p++ = out[i];
			}
		}
	}

}

