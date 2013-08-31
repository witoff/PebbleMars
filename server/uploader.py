import sys
import boto
import boto.s3

# AWS ACCESS DETAILS
AWS_ACCESS_KEY_ID = 'AKIAJBAG2LJ2R6P7SX5Q'
AWS_SECRET_ACCESS_KEY = '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'


import boto

bucket_name = 'witoff-mars-pebble'
conn = boto.connect_s3(AWS_ACCESS_KEY_ID,
            AWS_SECRET_ACCESS_KEY)



import boto.s3
bucket = conn.create_bucket(bucket_name,
        location=boto.s3.connection.Location.DEFAULT)

testfile = "images_processed/manifest.json"
print 'Uploading %s to Amazon S3 bucket %s' % \
       (testfile, bucket_name)

import sys
def percent_cb(complete, total):
    sys.stdout.write('.')
    sys.stdout.flush()

from boto.s3.key import Key
k = Key(bucket)
k.key = 'manifest.json'
k.set_contents_from_filename(testfile,
        cb=percent_cb, num_cb=10)