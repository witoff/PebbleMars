import requests
from pprint import pprint
import os
import json
from PIL import Image 
from os import path
import time
import math
import struct
import base64
from multiprocessing import Process, JoinableQueue

IMAGE_DIR_RAW = path.join(path.dirname(__file__), 'images_raw')
IMAGE_DIR_PROCESSED = path.join(path.dirname(__file__), 'images_processed')

# From PebbleMars.h
WORD_SIZE = 4
WORD_BITS = 8 * WORD_SIZE
IMAGE_WIDTH = 144
IMAGE_HEIGHT = 168
IMAGE_COLS = float(IMAGE_WIDTH) / (8 * WORD_SIZE)
IMAGE_ROWS = IMAGE_HEIGHT
IMAGE_PADDING_BYTES = int(IMAGE_COLS % 1 * WORD_SIZE);
IMAGE_ROWS_PER_CHUNK = 4
IMAGE_CHUNKS = int(math.ceil(IMAGE_ROWS / IMAGE_ROWS_PER_CHUNK))

print IMAGE_DIR_PROCESSED

save_image_q = JoinableQueue()

def getLatestUrl():
  response = requests.get('http://mars.jpl.nasa.gov/msl-raw-images/image/image_manifest.json')
  data = response.json()

  latest_sol = data['sols'][-1]
  sol_url = latest_sol['catalog_url']
  return sol_url

  
def getLatestImages(image_count):
  
  response = requests.get(getLatestUrl())
  data = response.json()
  latest_utc = data['most_recent']
  
  images = data['images']
  # Filter only the full images.
  filtered = [i for i in images if i["sampleType"] == "full" and "CHEM" not in i["instrument"]]
  # No more than 5 images
  filtered = filtered[:5]
  
  print 'filtered length: ', len(filtered)
  if len(filtered) == 0:
    print 'warning, filtered image len == 0.  Adding all images back in'
    filtered = images 
  #  print 'new filtered len: ', len(filtered)

  metadata = []
  for i in filtered:
    metadata.append({'instrument' : i['instrument'],
      'url' : i['urlList'],
      'utc' : i['utc'],
      'id' : i['itemName'],
      'site' : i['site'],
      'sol' : i['sol']
      })
  return latest_utc, metadata

def saveRawImage(i):
  # Downloads image and puts it in the queue.
  res = None
  try:
    outFile = open(path.join(IMAGE_DIR_RAW, i['filename']), 'w')
    request = requests.get(i['url'], stream=True)
    for block in request.iter_content(1024):
      if not block:
        break
      outFile.write(block)
    outFile.close()
    print "Saved %s" % i['id']
    res = outFile.name
  except Exception as e:
    print "Error downloading image: %s, %s" % (i['url'], str(e))
  return res

def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

def saveRawImageWorker():
  for item in iter(save_image_q.get, None):
    saveRawImage(item)
    save_image_q.task_done()
  save_image_q.task_done()

def saveRawImages(images):
  # Remove old images
  print 'removing old images'
  for name in os.listdir(IMAGE_DIR_RAW):
    os.remove(path.join(IMAGE_DIR_RAW, name))

  num_procs = 10
  for group in chunks(images, num_procs):
    procs = []
    for item in group:
      p = procs.append(Process(target=saveRawImageWorker))
      procs[-1].daemon = True
      procs[-1].start()

    for i in group:
      i['filename'] = i['id'] + '.jpg'
      save_image_q.put(i)

    save_image_q.join()

    for p in procs:
      save_image_q.put(None)

    for p in procs:
      p.join()

  print "Saved %d images" % len(images)

  outManifest = open(path.join(IMAGE_DIR_RAW, 'manifest.json'), 'w')
  outManifest.write(json.dumps(images))
  outManifest.close()
  #TODO: Save JSON manifest

def getImageData(filename):
  #Load
  img = Image.open(filename) # open colour image
  
  #Scale
  dims = (IMAGE_WIDTH, IMAGE_HEIGHT)
  print 'resizing from ', img.size, ' to dims ', dims
  img = img.resize(dims, Image.ANTIALIAS)
  print 'new size: ', img.size
  
  #Black and white
  img = img.convert('1') # convert image to black and white

  #Save Temp
  img.save(path.join(IMAGE_DIR_PROCESSED, filename.split('/')[-1].split('.')[0] + ".png"))

  # Convert to bitstream
  data_bits = []
  for i in range(img.size[1]):
    for j in range(img.size[0]):
      data_bits.append(int(bool(img.getpixel((j,i)))))
    if IMAGE_PADDING_BYTES > 0:
      for k in range(IMAGE_PADDING_BYTES * 8):
        data_bits.append(0)
  return data_bits

def processImages(utc):
  image_files = os.listdir(IMAGE_DIR_RAW)
  f = open(path.join(IMAGE_DIR_RAW, 'manifest.json'), 'r')
  manifest = json.loads(f.read())
  f.close()

  response = []
  for obj in manifest:
    data_bits = getImageData(path.join(IMAGE_DIR_RAW, obj['filename']))
    data_str = [str(d) for d in data_bits]
    data_bytes = []
    pos = 0
    for chunk_id in range(IMAGE_CHUNKS):
      chunk_bytes = [struct.pack('B', chunk_id)]
      for j in range(chunk_id, chunk_id+IMAGE_ROWS_PER_CHUNK):
        for i in range(int(math.ceil(IMAGE_COLS))):
          nums = data_str[pos:pos+WORD_BITS][::-1]
          pos += WORD_BITS
          if len(nums) == 0:
            break
          chunk_bytes.append(struct.pack('I', int(''.join(nums), 2)))
      if len(chunk_bytes) <= 1:
        break
      data_bytes.append(base64.b64encode(''.join(chunk_bytes)))

    utc = utc.split(".")[0].split("Z")[0]
    secs = time.mktime(time.gmtime()) - time.mktime(time.strptime(utc, "%Y-%m-%dT%H:%M:%S"))
    hours = int(secs/3600)
    if hours == 0:
      rel_time = str(int(secs/61)) + ' mins '
    else:
      rel_time = str(hours) + ' hours '
    rel_time += 'ago'

    response.append({
      #'data' : data,
      'data_bytes' : data_bytes,
      'width' : IMAGE_WIDTH,
      'height' : IMAGE_HEIGHT,
      'rel_time' : rel_time,
      'filename' : obj['filename'].replace('jpg', 'png'),
      'instrument' : obj['instrument'],
      'utc' : obj['utc'],
      'site': obj['site'],
      'sol': obj['sol']
    })
  print 'Writing output file...'
  f = open(path.join(IMAGE_DIR_PROCESSED, 'manifest.json'), 'w')
  f.write(json.dumps(response))
  f.close()
  return response

def main(image_count):
  latest_utc, images = getLatestImages(image_count)
  pprint(images)
  saveRawImages(images)
  data = processImages(latest_utc)
  
  
if __name__ == '__main__':
  main(5)
  
  
  
  


