PebbleEventListener.addEventListener("appmessage",
                     function(e) {
                       var temperatureRequest = e.payload.temperatureRequest;
                       if (temperatureRequest) {
                         fetchWeather();
                       }
});



function sendImage() {
  console.log("Sending image ...");

  Pebble.sendAppMessage({ imgIndex: , imgData: });
}
