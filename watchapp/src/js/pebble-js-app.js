var sending = false;
var done = false;
var imageData;
var sentBytes;
var interval;

function getByteAsString(bitArray, byteIndex) {
  var currentByte = 0;
  for (var i = 0; i < 8; i++) {
    currentByte = currentByte << 1;
    currentByte += bitArray[byteIndex * 8 + i];
  }
  console.log("Byte " + byteIndex + " is ", currentByte);
  if (currentByte == 0) {
    // dirty hack to avoid sending \0s ...
    currentByte = 1;
  }
  return String.fromCharCode(currentByte) + " ";
}

function sendImage(bitArray) {
  console.log("Sending image ...");

  if (!sending) {
    imageData = bitArray;
    sentBytes = 0;
    sending = true;
    interval = setInterval(function() {
      console.log("Send loop...");


      Pebble.sendAppMessage( { 'imgIndex': sentBytes, 'imgData': getByteAsString(bitArray, sentBytes) });
      sentBytes++;
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

        Pebble.sendAppMessage({ 'instrument': image.instrument, 'utc': image.utc });
        sendImage(image.data);
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

