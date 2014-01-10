import boto
import boto.s3
from boto.s3.key import Key
import sys
import os
from os import path
import urllib
import json

FILE_ROOT = path.join(path.dirname(__file__), 'images_processed')

BUCKET_NAME = 'pebble-mars-images'

RDF_PUBLIC_URL = "https://%s.s3.amazonaws.com/update_rdf.json" % (BUCKET_NAME)
MANIFEST_PUBLIC_URL = "https://%s.s3.amazonaws.com/manifest.json" % (BUCKET_NAME)

def main():
	conn = boto.connect_s3()
	bucket = conn.lookup(BUCKET_NAME)

	manifest_path = path.join(FILE_ROOT,"manifest.json")
	if not os.path.exists(manifest_path):
		print "ERROR: Manifest file not found to process"
		return

	manifest = json.loads(open(manifest_path,'r').read())

	# Upload the manifest
	fname = manifest_path
	print 'Uploading %s to Amazon S3 bucket %s' % \
				 (fname, BUCKET_NAME)
	def percent_cb(complete, total):
			sys.stdout.write('.')
			sys.stdout.flush()

	k = Key(bucket)
	k.key = path.basename(fname)
	k.set_contents_from_filename(fname,
		cb=percent_cb, num_cb=10)

	k.make_public()
	print ""

	# Upload the RDF
	new_rdf = {
		"manifest_version": manifest[0]['sol'],
		"url": MANIFEST_PUBLIC_URL,
		"ttl": 3600
	}

	fname = path.join(FILE_ROOT,"update_rdf.json")

	f = open(fname,'w')
	f.write(json.dumps(new_rdf))
	f.close()

	print 'Uploading %s to Amazon S3 bucket %s' % \
				 (fname, BUCKET_NAME)
	def percent_cb(complete, total):
			sys.stdout.write('.')
			sys.stdout.flush()

	k = Key(bucket)
	k.key = path.basename(fname) 
	k.set_contents_from_filename(fname,
		cb=percent_cb, num_cb=10)

	k.make_public()
	print ""

if __name__ == '__main__':
	main()
