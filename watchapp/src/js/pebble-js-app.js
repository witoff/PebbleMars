// From PebbleMars.h
var sizeof_gbword_t = 4;
var IMAGE_WIDTH = 144;
var IMAGE_HEIGHT = 144;
var IMAGE_COLS = IMAGE_WIDTH / (8 * sizeof_gbword_t);
var IMAGE_ROWS = IMAGE_HEIGHT;
var IMAGE_PADDING_BYTES = IMAGE_COLS % 1 * sizeof_gbword_t;
var WORD_ZERO_PAD = new Array(sizeof_gbword_t + 1).join("00");

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
    }, 1000);
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
        console.log("Name: " + image.name);
        console.log("Instrument: " + image.instrument);
        console.log("UTC: " + image.utc);

        Pebble.sendAppMessage({ 'utc': image.utc });
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

