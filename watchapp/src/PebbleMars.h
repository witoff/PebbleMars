#define APP_MESSAGE_BUF_IN 100
#define APP_MESSAGE_BUF_OUT 100
#define KEY_HELLO 42

#define KEY_NAME 2
#define KEY_INSTRUMENT 3
#define KEY_UTC 4

#define SHOW_METADATA FALSE

void display_new_image();
void set_footer_text(const char *text);

/* Constants chosen for image size of 144px by 144px
    Pebble requires num_bytes % 4 to equal zero
  */
uint8_t byte_buffer[144][20];

void set_bitmap_byte(uint16_t index, uint8_t byte);
void clear_bitmap();

#define SHOW_METADATA FALSE
#if SHOW_METADATA
void set_metadata_text(const char* text);
#endif