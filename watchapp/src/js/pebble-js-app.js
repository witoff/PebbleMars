var sending = false;

var allStrings = [];

function sendImage(byteArray) {
  console.log("Sending image ...");
  console.log("bits array length=" + byteArray.length);

  if (!sending) {
    sending = true;

    var sentBytes = 0;
    var interval = setInterval(function() {
      if (sentBytes >= byteArray.length) {
        console.log("Done sending.");
        clearInterval(interval);
        sending = false;
      }
      var currentLine = "";
      var startLineIndex = sentBytes;

      for (var i = 0; i < 18; i++) {
        var word = byteArray[startLineIndex + i].toString(16);
        if (word.length == 1) { word = '0' + word; }
        currentLine += word[1] + word[0];
        sentBytes++;
      }

      allStrings.push(startLineIndex);
      allStrings.push(currentLine);

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

