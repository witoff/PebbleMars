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

var rdfUrl = 'https://disla-pebble-mars.s3.amazonaws.com/update_rdf.json';
var manifestUrl = 'https://disla-pebble-mars.s3.amazonaws.com/manifest.json';

function sendImage(image) {
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
		fetchImages();
		return;
	}
	var manifest_version = localStorage.getItem("manifest_version");
	
	console.log("Cached manifest items: "+manifest.length);
 
	if (manifest && manifest.length >= currentImageIndex) {
		console.log("Sending image from manifest version "+manifest_version+", index: "+currentImageIndex);
		sendImage(manifest[currentImageIndex]);
	} else {
		console.log("Error sending image from manifest because manifest was not invalid.")
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
    console.log("Error fetching manifest, skiping image send.");
	sendCachedImage();
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
	sendCachedImage();
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
      console.log("Sending image "+currentImageIndex+"/"+manifest.length);
      //sendImage(manifest[currentImageIndex]);
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
