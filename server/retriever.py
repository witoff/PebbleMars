import requests
from pprint import pprint
import os
import json
from PIL import Image 

IMAGE_DIR_RAW = './images_raw/'
IMAGE_DIR_PROCESSED = './images_processed/'
def getLatestUrl():
	response = requests.get('http://mars.jpl.nasa.gov/msl-raw-images/image/image_manifest.json')
	data = response.json()

	latest_sol = data['sols'][-1]
	sol_url = latest_sol['catalog_url']
	return sol_url
	
	
	
def getLatestImages(image_count):
	
	response = requests.get(getLatestUrl())
	data = response.json()
	
	images = data['images']
	#Filter
	filtered = []
	for i in images:
		if 'NAV_' in i['instrument'] and i["sampleType"] == "full":
			filtered.append(i)
			
	filtered = filtered[-image_count:]
	print 'filtered length: ', len(filtered)

	metadata = []
	for i in filtered:
		metadata.append({'instrument' : i['instrument'],
			'url' : i['urlList'],
			'utc' : i['utc'],
			'id' : i['itemName']
			})
	return metadata
	
def saveRawImages(images):
	# Remove old images
	print 'removing old images'
	for name in os.listdir(IMAGE_DIR_RAW):
		os.remove(IMAGE_DIR_RAW + name)
	for i in images:
		i['filename'] = i['id'] + '.jpg'
		outFile = open(IMAGE_DIR_RAW + i['filename'], 'w')
		request = requests.get(i['url'], stream=True)
		for block in request.iter_content(1024):
			if not block:
				break
		 	outFile.write(block)
		outFile.close()
		print 'saving: ', i['id'], '.png'
	outManifest = open(IMAGE_DIR_RAW + 'manifest.json', 'w')
	outManifest.write(json.dumps(images))
	outManifest.close()
	#TODO: Save JSON manifest

def getImageData(filename):
	#Load
	img = Image.open(filename) # open colour image
	
	#Scale
	dims = (144,168)
	img.thumbnail(dims, Image.ANTIALIAS)
	
	#Black and white
	img = img.convert('1') # convert image to black and white

	#Save Temp
	img.save(IMAGE_DIR_PROCESSED + filename.split('/')[-1].split('.')[0] + ".png")

	# Convert to bytestream
	bytes = []
	for i in range(img.size[0]):
		for j in range(img.size[1]):
			bytes.append(int(bool(img.getpixel((i,j)))))
	return bytes

def processImages():
	image_files = os.listdir(IMAGE_DIR_RAW)
	f = open(IMAGE_DIR_RAW + 'manifest.json', 'r')
	manifest = json.loads(f.read())
	f.close()

	response = []
	for obj in manifest:
		data = getImageData(IMAGE_DIR_RAW + obj['filename'])
		response.append({
			'data' : data,
			'name' : obj['filename'].replace('jpg', 'png'),
			'instrument' : obj['instrument'],
			'utc' : obj['utc']
		})
	print 'Writing output file...'
	f = open(IMAGE_DIR_PROCESSED + 'manifest.json', 'w')
	f.write(json.dumps(response))
	f.close()
	return response
	
if __name__ == '__main__':
	images = getLatestImages(10)
	pprint(images)
	saveRawImages(images)
	data = processImages()
	
	
	
	


