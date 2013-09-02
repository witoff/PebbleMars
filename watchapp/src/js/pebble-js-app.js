// From PebbleMars.h
var WORD_SIZE = 4;
var IMAGE_WIDTH = 144;
var IMAGE_HEIGHT = 144;
var IMAGE_COLS = IMAGE_WIDTH / (8 * WORD_SIZE);
var IMAGE_ROWS = IMAGE_HEIGHT;
var IMAGE_PADDING_BYTES = IMAGE_COLS % 1 * WORD_SIZE;

var sending = false;
var sendingInterval = 0;
var currentImageIndex = 0;
var imageResendQueue = [];
var manifest = {};

function sendImage(image) {
  console.log("Sending image ...");
  console.log("Relative Time: " + image.rel_time);
  console.log("Instrument: " + image.instrument);
  console.log("UTC: " + image.utc);

  Pebble.sendAppMessage({ 'utc': image.utc,
                          'instrument': image.instrument,
                          'rel_time' : image.rel_time,
                          'site' : image.site,
                          'filename' : image.filename});

  sendImageChunks(image.data_bytes);
}

function sendImageChunks(chunks) {
  if (sending) {
    return;
  }
  sending = true;

  console.log("image chunks = " + chunks.length);

  var currentChunkId = 0;
  var interval = sendingInterval = setInterval(function() {
    var chunkId;
    if (imageResendQueue.length) {
      var resendChunkId = imageResendQueue.shift();
      console.log('resending chunk ' + resendChunkId);
      chunkId = resendChunkId;
    } else if (currentChunkId < chunks.length) {
      chunkId = currentChunkId++;
    } else {
      queryImageComplete();
      clearInterval(interval);
      return;
    }

    if (typeof(chunkId) !== 'undefined') {
      var chunkData = chunks[chunkId];
      //console.log(chunkId + ": " + chunkData);
      Pebble.sendAppMessage({ 'image_data':  chunkData });
    }
  }, 100);
}

function queryImageComplete() {
  Pebble.sendAppMessage({ 'image_complete': 0 });
}

function stopImageSend() {
  if (sending) {
    sending = false;
    clearInterval(sendingInterval);
  }
}

function fetchImages(isUpdate) {
  var req = new XMLHttpRequest();
  var manifestUrl = 'https://pebble-mars.s3.amazonaws.com/manifest.json';
  req.open('GET', manifestUrl, true);
  console.log('Requesting ' + manifestUrl);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if (req.status == 200) {
        manifest = JSON.parse(req.responseText);

        console.log("Got " + manifest.length + " images.");
        if (isUpdate) {
          sendImage(manifest[currentImageIndex]);
        }
      } else {
        console.log("Error getting manifest");
      }
    }
  }
  req.send(null);
}

PebbleEventListener.addEventListener("ready",
  function(e) {
    fetchImages(true);
  }
);

PebbleEventListener.addEventListener("appmessage",
  function(e) {
    console.log('watch> ' + JSON.stringify(e.payload));
    if (e.payload.hasOwnProperty('image_request_chunk')) {
      imageResendQueue.push(e.payload.image_request_chunk);
    }
    if (e.payload.hasOwnProperty('image_complete')) {
      stopImageSend();
    }
    if (e.payload.hasOwnProperty('image_request_next')) {
      stopImageSend();
      currentImageIndex++;
      if (currentImageIndex >= manifest.length) {
        fetchImages(false);
        currentImageIndex = 0;
      }
      sendImage(manifest[currentImageIndex]);
    }
  }
);

