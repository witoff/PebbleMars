PebbleEventListener.addEventListener("ready",
  function(e) {
    console.log("ready!")
  }
);

PebbleEventListener.addEventListener("appmessage",
  function(e) {
    // var temperatureRequest = e.payload.temperatureRequest;
    // if (temperatureRequest) {
    //   fetchWeather();
    // }

    console.log("Got message. Let's start sending image ...");
    sendImage();
  }
);

function sendImage() {
  console.log("Sending image ...");

  Pebble.sendAppMessage({ imgIndex: 42, imgData: "xxxx"});
}
