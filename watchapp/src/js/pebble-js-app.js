// From PebbleMars.h
var WORD_SIZE = 4;
var IMAGE_WIDTH = 144;
var IMAGE_HEIGHT = 144;
var IMAGE_COLS = IMAGE_WIDTH / (8 * WORD_SIZE);
var IMAGE_ROWS = IMAGE_HEIGHT;
var IMAGE_PADDING_BYTES = IMAGE_COLS % 1 * WORD_SIZE;
var WORD_ZERO_PAD = new Array(WORD_SIZE + 1).join("00");

var sending = false;

function sendImage(byteArray) {
  console.log("Sending image ...");
  console.log("data length=" + byteArray.length);

  if (!sending) {
    sending = true;

    var line = 0;
    var interval = setInterval(function() {
      if (line >= byteArray.length) {
        console.log("Done sending.");
        clearInterval(interval);
        sending = false;
      }
      var currentLine = byteArray[line++];
      console.log(line + ": " + currentLine);
      Pebble.sendAppMessage({ 'imgData': currentLine });
    }, 500);
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
    // var temperatureRequest = e.payload.temperatureRequest;
    // if (temperatureRequest) {
    //   fetchWeather();
    // }

    //console.log("Got message. Let's start sending image ...");
    //fetchImages();
  }
);

