import boto
import boto.s3
from boto.s3.key import Key
import sys
import os
from os import path
import urllib
import json

FILE_ROOT = path.join(path.dirname(__file__), 'images_processed')

#BUCKET_NAME = 'pebble-mars'
BUCKET_NAME = 'disla-pebble-mars'

RDF_PUBLIC_URL = "https://%s.s3.amazonaws.com/update_rdf.json" % (BUCKET_NAME)
MANIFEST_PUBLIC_URL = "https://%s.s3.amazonaws.com/manifest.json" % (BUCKET_NAME)

def main():
	conn = boto.connect_s3()

	# Get the current RDF.
	rdf = None
	rdf_f = urllib.urlopen(RDF_PUBLIC_URL)
	if rdf_f.getcode() == 200:
		rdf = json.loads(rdf_f.read())
		print "INFO: Downloaded RDF, version is: %s" % (rdf['manifest_version'])
	else:
		print "WARN: Could not download current RDF, creating new RDF"

	bucket = conn.lookup(BUCKET_NAME)

	manifest_path = path.join(FILE_ROOT,"manifest.json")
	manifest = None

	do_manifest_upload = False
	do_rdf_update = False
	if os.path.exists(manifest_path):
		manifest = json.loads(open(manifest_path,'r').read())
		# compare RDF version with manifest version. (version is the sol number)
		if rdf:
			rdf_version = rdf['manifest_version']
			manifest_version = manifest[0]['sol']
			if rdf_version != manifest_version:
				print "INFO: Manifest version has changed from RDF (%s != %s), updating S3 RDF and manifest" % (manifest_version, rdf_version)
				do_manifest_upload = True
				do_rdf_update = True
			else:
				print "INFO: RDF and manifest versions match, skipping update"

		else:
			do_rdf_update = True

	else:
		print "ERROR: Manifest file not found to process"

	# Upload the manifest if it has changed.
	if do_manifest_upload:
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

	# Upload the RDF if it has changed.
	if do_rdf_update or True:
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
