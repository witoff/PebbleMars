import boto
import boto.s3
from boto.s3.key import Key
import sys
import os
from os import path

FILE_ROOT = path.join(path.dirname(__file__), 'images_processed')

def main():
	bucket_name = 'pebble-mars'
	conn = boto.connect_s3()

	bucket = conn.lookup(bucket_name)
	for f in os.listdir(FILE_ROOT):
		fname = path.join(FILE_ROOT, f)
		print 'Uploading %s to Amazon S3 bucket %s' % \
		       (fname, bucket_name)
		def percent_cb(complete, total):
		    sys.stdout.write('.')
		    sys.stdout.flush()

		k = Key(bucket)
		k.key = f
		k.set_contents_from_filename(fname,
			cb=percent_cb, num_cb=10)

if __name__ == '__main__':
	main()
