var done = false;

function sendImage() {
  console.log("Sending image ...");

  interval = setInterval(function() {
    console.log("Send loop running...");

    Pebble.sendAppMessage({ imgIndex: 42, imgData: "xxxx"});
  }, 500);
}




PebbleEventListener.addEventListener("ready",
  function(e) {
    console.log("ready! will start sending stuff...")
    sendImage();
  }
);

PebbleEventListener.addEventListener("appmessage",
  function(e) {
    // var temperatureRequest = e.payload.temperatureRequest;
    // if (temperatureRequest) {
    //   fetchWeather();
    // }

    console.log("Got message. Let's start sending image ...");
    //sendImage();
  }
);

