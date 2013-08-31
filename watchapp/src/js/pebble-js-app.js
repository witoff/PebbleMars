var sending = false;
var done = false;

function sendImage() {
  console.log("Sending image ...");

  if (!sending) {
    sending = true;
    interval = setInterval(function() {
      console.log("Send loop running...");

      console.log("should have been sent");
      Pebble.sendAppMessage({ 'imgIndex': 42, 'imgData': "xxxx"});

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
        console.log(req.responseText);
        response = JSON.parse(req.responseText);

        var image = response[0];
        console.log("Got " + response.length + " images.");
        console.log("Sending " + image.name);

        Pebble.sendAppMessage({ 'name': image.name, 'instrument': image.instrument, 'utc': image.utc });
      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

PebbleEventListener.addEventListener("ready",
  function(e) {
    console.log("ready!")
    //    sendImage();
  }
);

PebbleEventListener.addEventListener("appmessage",
  function(e) {
    // var temperatureRequest = e.payload.temperatureRequest;
    // if (temperatureRequest) {
    //   fetchWeather();
    // }

    console.log("Got message. Let's start sending image ...");
    fetchImages();
  }
);

