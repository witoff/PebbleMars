#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "base64.h"

static const char alpha[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void decode_base64(uint32_t *dest, const uint8_t *src, const uint16_t src_len) {
	if(src == NULL || src_len == 0)
		return;

	uint16_t bytes_left = src_len;

	uint32_t octets; //I could use a uin24_t right about now
	uint8_t alpha_index[4];
	uint8_t copy_len;
	uint8_t *head = (uint8_t *) src;
	uint8_t *d;

	do {
		//Copy data into octets for processing 
		octets = 0x00000000;
		copy_len = 3 - (bytes_left % 3);
		memcpy(&octets, head, copy_len);

		/*
		  V-               octets      -V   Padding  
		  1111 1100  0000 0000  0000 0000  0000 0000 -\
		  0000 0011  1111 0000  0000 0000  0000 0000    -> Decoded alpha
		  0000 0000  0000 1111  1100 0000  0000 0000    -> index locations
		  0000 0000  0000 0000  0011 1111  0000 0000 -/
        */
		  						//0x00000000
		alpha_index[0] = octets & 0xFC000000;
		alpha_index[1] = octets & 0x03F00000;
		alpha_index[2] = octets & 0x000FC000;
		alpha_index[3] = octets & 0x00003F00;

		d = (uint8_t *) dest;
		for(uint8_t i = 0; i < 4; ++i, ++d) {
			*d = alpha[alpha_index[i]];
		}

		head += copy_len;
		bytes_left -= copy_len;

	} while (bytes_left != 0);


}