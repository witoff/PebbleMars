#define APP_MESSAGE_BUF_IN 100
#define APP_MESSAGE_BUF_OUT 100
#define KEY_HELLO 42

#define KEY_NAME 2
#define KEY_INSTRUMENT 3
#define KEY_UTC 4

#define SHOW_METADATA FALSE

void display_new_image(const GBitmap *image);
void set_footer_text(const char *text);
#if SHOW_METADATA
void set_metadata_text(const char* text);
#endif