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
  console.log("bits array length=" + byteArray.length);

  if (!sending) {
    sending = true;

    var sentWords = 0;
    var interval = setInterval(function() {
      if (sentWords >= byteArray.length) {
        console.log("Done sending.");
        clearInterval(interval);
        sending = false;
      }
      var currentLine = "";
      var startLineIndex = sentWords;

      for (var i = 0; i < Math.ceil(IMAGE_COLS); i++) {
        var word = byteArray[startLineIndex + i].toString(16);
        word = (WORD_ZERO_PAD + word).substr(-WORD_ZERO_PAD.length);
        currentLine += word;
        sentWords++;
      }

      console.log(startLineIndex + " >> " + currentLine);

      Pebble.sendAppMessage( { /*'imgIndex': '+' + startLineIndex,*/ 'imgData': "=" + currentLine });
    }, 500);
  }
}


function fetchImages() {
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', 'https://s3.amazonaws.com/witoff-mars-pebble/manifest.json', true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        //console.log(req.responseText);
        response = JSON.parse(req.responseText);

        var image = response[0];
        console.log("Got " + response.length + " images.");
        console.log("Title: " + image.title);
        console.log("Instrument: " + image.instrument);
        console.log("UTC: " + image.utc);

        Pebble.sendAppMessage({ 'utc': image.utc,
                                'title': image.title,
                                'instrument': image.instrument});
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

