# BlobKV (Blobstore Object) Design Proposal

## Use Case

Serverless computing is a FaaS ( Function as a Service ) framework.  When client sends GET/PUT request (key/value stores), Serverless can receive your request and start a function docker image to process it. If the request involves data processing,Serverless will invoke the back-end storage service to connect with the key/value application (such as AWS S3, Swift). At this moment, we need this function to speed up the service respond time and improve performance, so we can use SPDK to provide blobstore object to accelerate it.

## Proposed Architecture Overview

![SPDK BlobKV Archtecture Proposal](https://github.com/hellowaywewe/spdk/blob/master/doc/BlobKV.png)

- BlobKV provides a set of key/value stores api for the application (such as AWS S3, Swift)

- Blobstore is a safe block allocator designed to allow asynchronous, uncached, parallel reads and writes to groups of blocks on a block device called 'blobs'. 

- Block storage in SPDK applications is provided by the SPDK bdev layer.  SPDK bdev includes:

    * a generic bdev class for implementing bdev drivers on those devices
    * bdev drivers for NVMe, Blob bdev, Linux AIO and Ceph RBD
    
Our original design was to build key/value device abstraction layer on the same floor of the block device abstraction layer，and the key/value device abstraction layer connects up to the BlobKV and down to the NVMe device. Simply, it means key/value is directly mapped to the physical block address (like key/value --> PBA).
