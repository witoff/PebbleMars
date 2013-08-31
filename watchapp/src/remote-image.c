#include <pebble_os.h>
#include "remote-image.h"

void remote_image_data(DictionaryIterator *received) {
  Tuple *imgIndexTuple, *imgDataTuple;

  imgIndexTuple = dict_find(received, KEY_IMG_INDEX);
  imgDataTuple = dict_find(received, KEY_IMG_DATA);

  if (imgIndexTuple && imgDataTuple) {
    int32_t imgIndex = imgIndexTuple->value->int32;
    uint8_t *data = imgDataTuple->value->data;
    uint16_t length = imgDataTuple->length;

    APP_LOG(APP_LOG_LEVEL_INFO, "Received image[%li] - %d bytes", imgIndex, length);
  }
  else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Not a remote-image message");
  }
}
