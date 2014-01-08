// From PebbleMars.h
var WORD_SIZE = 4;
var IMAGE_WIDTH = 144;
var IMAGE_HEIGHT = 144;
var IMAGE_COLS = IMAGE_WIDTH / (8 * WORD_SIZE);
var IMAGE_ROWS = IMAGE_HEIGHT;
var IMAGE_PADDING_BYTES = IMAGE_COLS % 1 * WORD_SIZE;

var sending = false;
var sendingInterval = 0;
var currentImageIndex = 0;
var imageResendQueue = [];
var manifest = {};

var DEFAULT_UPDATE_URL = 'https://disla-pebble-mars.s3.amazonaws.com';

var rdfUrl = DEFAULT_UPDATE_URL+'/update_rdf.json';
var manifestUrl = DEFAULT_UPDATE_URL+'/manifest.json';
var configureUrl = DEFAULT_UPDATE_URL+'/configurable.html';
						
var DEFAULT_OPTIONS = {
  "marsTime": "on",
  "navcam":  "true",
  "hazcam":  "true",
  "mastcam": "true",
  "mahli":   "true"
}
						
function getSavedOptions() {
  var options = localStorage.getItem("options");
  if (options) {
	try {
      return JSON.parse(options);
	} catch(e) {
	  console.log("WARN: error parsing options from localStorage: "+e);
	  localStorage.removeItem("options");
	  return DEFAULT_OPTIONS;
	}
  } else {
	return DEFAULT_OPTIONS;
  }
}
						
Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  var options = getSavedOptions();
  console.log("Loaded options from storage: "+JSON.stringify(options));
  var url_params = "";
  for (var key in options) {
    if (url_params != "") {
        url_params += "&";
    }
    url_params += key + "=" + options[key];
  }
  Pebble.openURL(configureUrl+"?"+url_params);
});
						
Pebble.addEventListener("webviewclosed", function(e) {
	console.log("configuration closed");
	// webview closed
	try {
		var options = JSON.parse(decodeURIComponent(e.response));
		console.log("Options = " + JSON.stringify(options));
		// Save options to localStorage.
		localStorage.setItem("options",JSON.stringify(options));
		processOptions(options);
	} catch(e) {
		console.log("Error parsing options: "+e);
	}
});

function processOptions() {
	var options = getSavedOptions();
	
	// Toggle display of Mars time
	Pebble.sendAppMessage({ 'hide_mars_time': options['marsTime'] == "on" ? 0 : 1 });
}

function getRelTime(utc_str) {
    utc = utc_str.split(".")[0].split("Z")[0];
    var ms = new Date().getTime() - Date.parse(utc);
    var secs = ms / 1000;
    var hours = Math.round(secs/3600);
    if (hours == 0) {
        rel_time = Math.round(secs/61) + " mins";
    } else {
        rel_time = hours + " hours";
    }
    rel_time += " ago";
    return rel_time;   
}

function sendImage(image) {
	
  // Compute new rel_time since this is normally done on the server, but the cache may be out of date.
  image.rel_time = getRelTime(image.utc); 

  console.log("Sending image ...");
  console.log("Relative Time: " + image.rel_time);
  console.log("Instrument: " + image.instrument);
  console.log("UTC: " + image.utc);

  Pebble.sendAppMessage({ 'utc': image.utc,
                          'instrument': image.instrument,
                          'rel_time' : image.rel_time,
                          'site' : image.site,
                          'filename' : image.filename});

  sendImageChunks(image.data_bytes);
}

function sendImageChunks(chunks) {
  if (sending) {
    return;
  }
  sending = true;

  console.log("image chunks = " + chunks.length);

  var currentChunkId = 0;
  var interval = sendingInterval = setInterval(function() {
    var chunkId;
    if (imageResendQueue.length) {
      var resendChunkId = imageResendQueue.shift();
      console.log('resending chunk ' + resendChunkId);
      chunkId = resendChunkId;
    } else if (currentChunkId < chunks.length) {
      chunkId = currentChunkId++;
    } else {
      queryImageComplete();
      clearInterval(interval);
      return;
    }

    if (typeof(chunkId) !== 'undefined') {
      var chunkData = chunks[chunkId];
      //console.log(chunkId + ": " + chunkData);
      Pebble.sendAppMessage({ 'image_data':  chunkData });
    }
  }, 75);
}

function queryImageComplete() {
  Pebble.sendAppMessage({ 'image_complete': 0 });
}

function stopImageSend() {
  if (sending) {
    sending = false;
    clearInterval(sendingInterval);
  }
}

function sendCachedImage() {
	var manifest = localStorage.getItem("image_manifest");
	try {
		manifest = JSON.parse(manifest);
	} catch(e) {
		console.log("Error fetching manifest from localStorage, marking dirty: "+e);
		localStorage.removeItem("image_manifest");
		localStorage.removeItem("manifest_version");
		fetchImages(true);
		return;
	}
	var manifest_version = localStorage.getItem("manifest_version");
	
	var options = getSavedOptions();
	
	console.log("Cached manifest items: "+manifest.length);

	var filtered_images = [];
	for (var i=0; i < manifest.length; i++) {
		var image = manifest[i]; 
		var inst = image['instrument'];
		if ( (inst.match("[FR]HAZ_") && options['hazcam']) || 
			 (inst.match("NAV_") && options['navcam']) || 
			 (inst.match("MAST_") && options['mastcam']) ||
			 (inst.match("MAHLI") && options['mahli']) ) {
			
			filtered_images.push(image);
		}
	}
	console.log("Num filtered images: "+filtered_images.length);

	if (filtered_images.length > 0) {
		currentImageIndex = currentImageIndex % filtered_images.length;
    	console.log("Sending image "+currentImageIndex+"/"+filtered_images.length);
		sendImage(filtered_images[currentImageIndex]);
	}
}

function fetchRdf(url, done_cb, error_cb) {
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  console.log('Requesting ' + url);
  req.onload = function(e) {
	if (req.readyState == 4) {
      if(req.status == 200) {
        try {
          rdf = JSON.parse(req.responseText);
          done_cb.apply(null,[rdf]);
		} catch(e) {
          error_cb.apply(null,[e]);
		}
	  }
	} else {
	  error_cb.apply(null,["XHR completed with status: "+req.status]);
	}
  }
  req.onerror = function(e) {
    error_cb.apply(null,[e]);
  }
  req.send(null);
}

function fetchManifest(url, done_cb, error_cb) {
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  console.log('Requesting ' + url);
  req.onload = function(e) {
	if (req.readyState == 4) {
      if(req.status == 200) {
		try {
          manifest = JSON.parse(req.responseText);
          done_cb.apply(null,[manifest]);
		} catch(e) {
          error_cb.apply(null,[e]);
		}
	  }
	} else {
	  error_cb.apply(null,["XHR completed with status: "+req.status]);
	}
  }
  req.onerror = function(e) {
    error_cb.apply(null,[e]);
  }
  req.send(null);
}

function fetchImages(isUpdate) {
  var fetch_rdf = false;
  var last_rdf_update = localStorage.getItem('last_rdf_update');
  var update_ttl = localStorage.getItem('update_ttl');
  var time_until_check = 0;
	
  if (last_rdf_update && update_ttl) {
    time_until_check = parseInt(update_ttl - ((new Date().getTime()/1000) - last_rdf_update),10);
	console.log("Seconds until next RDF check: "+time_until_check);
	if (time_until_check <= 0) {
      console.log("RDF TTL has expired, fetching new RDF");
	  fetch_rdf = true;
	} else {
	  console.log("RDF TTL has not expired, skipping RDF fetch.");
	}
  } else {
	console.log("RDF localStorage keys have not been set, fetching RDF");
    fetch_rdf = true;
  }

  // Manifest fetch complete callback.
  var manifest_fetch_done_cb = function(manifest) {
	// Use sol of first image as manifest version.
	var manifest_version = manifest[0]['sol'];
	  
    localStorage.setItem("image_manifest",JSON.stringify(manifest));
	localStorage.setItem('manifest_version',manifest_version);
	  
	console.log("Wrote manifest version "+manifest_version+", with "+manifest.length+" items to localStorage");
	  
    if (isUpdate) {
      sendCachedImage();
    }
  }
  
  // Manifest fetch error callback.
  var manifest_fetch_error_cb = function(e) {
    console.log("Error fetching manifest.");
    if (isUpdate) {
	  sendCachedImage();
	}
  }
	
  // RDF Fetch complete callback.
  var rdf_fetch_done_cb = function(rdf) {
    var rdf_ttl = rdf['ttl'];
	localStorage.setItem('update_ttl', rdf_ttl);
	
	var curr_manifest_version = localStorage.getItem('manifest_version');
	var new_manifest_version = rdf['manifest_version'];
	console.log("Got RDF manifest_version: "+new_manifest_version+", current version: "+curr_manifest_version);
	 
	if (curr_manifest_version != new_manifest_version) {
      console.log("RDF manifest versions have changed: "+curr_manifest_version+" != "+new_manifest_version+", fetching new manifest.");
      // Manifest versions have changed, get the new manifest.
      fetchManifest(manifestUrl, manifest_fetch_done_cb, manifest_fetch_error_cb);
    } else {
      console.log("Latest RDF manifest version is same as localStorage version, skipping manifest update.");
      if (isUpdate) {
        sendCachedImage();
      }
    }
	localStorage.setItem('last_rdf_update',new Date().getTime()/1000);
  }
  
  // RDF Fetch error callback
  var rdf_fetch_error_cb = function(e) {
	console.log("Error fetching RDF, downloading manifest anyway");
	fetchManifest(manifestUrl, manifest_fetch_done_cb, manifest_fetch_error_cb);
  }
  
  if (fetch_rdf) {
    fetchRdf(rdfUrl, rdf_fetch_done_cb, rdf_fetch_error_cb);
  } else {
    if (isUpdate) {
	  sendCachedImage();
	}
  }
}

Pebble.addEventListener("ready",
  function(e) {
	fetchImages(true);
  }
);

Pebble.addEventListener("appmessage",
  function(e) {
    console.log('watch> ' + JSON.stringify(e.payload));
    if (e.payload.hasOwnProperty('image_request_chunk')) {
      imageResendQueue.push(e.payload.image_request_chunk);
    }
    if (e.payload.hasOwnProperty('image_complete')) {
      stopImageSend();
    }
    if (e.payload.hasOwnProperty('image_request_next')) {
      stopImageSend();
      currentImageIndex++;
      if (currentImageIndex >= manifest.length) {
        fetchImages(false);
        currentImageIndex = 0;
      }
	  sendCachedImage();
    }
	if (e.payload.hasOwnProperty('tz_offset')) {
	  // Get the number of seconds to add to convert localtime to utc
      var offsetSeconds = new Date().getTimezoneOffset() * 60;
      // Send it to the watch
	  Pebble.sendAppMessage({ 'tz_offset': offsetSeconds });
	}
  }
);
