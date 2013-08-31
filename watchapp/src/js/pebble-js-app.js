var sending = false;

var allStrings = [];

function getByteAsString(bitArray, byteIndex) {
  var currentByte = 0;
  for (var i = 0; i < 8; i++) {
    currentByte = currentByte << 1;
    currentByte += bitArray[byteIndex * 8 + i];
  }
  return currentByte.toString(16);
}

function sendImage(bitArray) {
  console.log("Sending image ...");
  console.log("bits array length=" + bitArray.length + " which is " + bitArray.length / 8 + " bytes");

  if (!sending) {
    sending = true;

    var sentBytes = 0;
    var interval = setInterval(function() {
      if (sentBytes >= bitArray.length / 8) {
        console.log("Done sending.");
        clearInterval(interval);
        sending = false;
      }
      var currentLine = "=";
      var startLineIndex = sentBytes;

      for (var i = 0; i < 18; i++) {
        currentLine += getByteAsString(bitArray, sentBytes++);
        //currentLine += "0000";
      }

      allStrings.push(startLineIndex);
      allStrings.push(currentLine);

      console.log("Sending data for startIndex: " + startLineIndex + " >> " + currentLine);
      Pebble.sendAppMessage( { /*'imgIndex': '+' + startLineIndex,*/ 'imgData': currentLine });
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

