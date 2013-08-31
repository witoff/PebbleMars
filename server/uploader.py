import boto
import boto.s3
from boto.s3.key import Key
import sys
import os
from os import path
	
# AWS ACCESS DETAILS
AWS_ACCESS_KEY_ID = 'AKIAJBAG2LJ2R6P7SX5Q'
AWS_SECRET_ACCESS_KEY = '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'

FILE_ROOT = path.join(path.dirname(__file__), 'images_processed')

def main():
	bucket_name = 'witoff-mars-pebble'
	conn = boto.connect_s3(AWS_ACCESS_KEY_ID,
		    AWS_SECRET_ACCESS_KEY)

	bucket = conn.create_bucket(bucket_name,
		location=boto.s3.connection.Location.DEFAULT)
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
