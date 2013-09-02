// From PebbleMars.h
var WORD_SIZE = 4;
var IMAGE_WIDTH = 144;
var IMAGE_HEIGHT = 144;
var IMAGE_COLS = IMAGE_WIDTH / (8 * WORD_SIZE);
var IMAGE_ROWS = IMAGE_HEIGHT;
var IMAGE_PADDING_BYTES = IMAGE_COLS % 1 * WORD_SIZE;

var sending = false;
var sendingInterval = 0;
var imageResendQueue = [];

function sendImage(chunks) {
  console.log("Sending image ...");
  console.log("image chunks = " + chunks.length);

  if (!sending) {
    sending = true;

    var currentChunkId = 0;
    var sendingInterval = setInterval(function() {
      var chunkId;
      if (imageResendQueue.length) {
        var resendChunkId = imageResendQueue.shift();
        console.log('resending chunk ' + resendChunkId);
        chunkId = resendChunkId;
      } else if (currentChunkId < chunks.length) {
        chunkId = currentChunkId++;
      }

      if (typeof(chunkId) !== 'undefined') {
        var chunkData = chunks[chunkId];
        //console.log(chunkId + ": " + chunkData);
        Pebble.sendAppMessage({ 'image_data':  chunkData });
      }
    }, 100);
  }
}

function fetchImages() {
  var response;
  var req = new XMLHttpRequest();
  var manifestUrl = 'https://pebble-mars.s3.amazonaws.com/manifest.json';
  req.open('GET', manifestUrl, true);
  console.log('Requesting ' + manifestUrl);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if (req.status == 200) {
        response = JSON.parse(req.responseText);

        var image = response[0];
        console.log("Got " + response.length + " images.");
        console.log("Relative Time: " + image.rel_time);
        console.log("Instrument: " + image.instrument);
        console.log("UTC: " + image.utc);

        Pebble.sendAppMessage({ 'utc': image.utc,
                                'instrument': image.instrument,
                                'rel_time' : image.rel_time,
                                'site' : image.site,
                                'filename' : image.filename});
        sendImage(image.data_bytes);
      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

PebbleEventListener.addEventListener("ready",
  function(e) {
    console.log("ready! Starting ...")
    fetchImages();
  }
);

PebbleEventListener.addEventListener("appmessage",
  function(e) {
    console.log('watch> ' + JSON.stringify(e.payload));
    if (e.payload.image_request_chunk) {
      imageResendQueue.push(e.payload.image_request_chunk);
    }
    if (e.payload.image_complete) {
      if (sending) {
        sending = false;
        clearInterval(sendingInterval);
      }
    }
  }
);

