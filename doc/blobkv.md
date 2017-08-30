# BlobKV (Blobstore Object) Design Proposal

## Use Case

Serverless computing is a FaaS ( Function as a Service ) framework.  When client sends key/value request ( GET/PUT, for example ), Serverless can receive your request and start a docker image the function to handle it. If the request involves data processing, Serverless will invoke the back-end storage service to connect with the key/value application (such as AWS S3, Swift). At this moment, we hope to speed up the service and improve performance, so we can use SPDK to accelerate the storage service of objects at the back end.

## Proposed Architecture Overview

![SPDK BlobKV Archtecture Proposal](https://github.com/hellowaywewe/spdk/blob/master/doc/BlobKV.png)

- BlobKV provides a set of APIs for the application (such as AWS S3, Swift)

- Blobstore is a safe block allocator designed to allow asynchronous, uncached, parallel reads and writes to groups of blocks on a block device called 'blobs'. 

- Block storage in SPDK applications is provided by the SPDK bdev layer.  SPDK bdev includes:

    * a generic bdev class for implementing bdev drivers on those devices
    * bdev drivers for NVMe, Blob bdev, Linux AIO and Ceph RBD
    
Our original idea was to build the key/value device abstraction layer on the same floor of the block device abstraction layer，and the key/value device abstraction layer connects up to the BlobKV and down to the NVMe device， which simply means mapping key/value directly to the physical block address (from key/value to PBA). Unfortunately,  neither the BlobKV nor key/value device abstraction layer are provided in the SPDK, we are going to implement them in the following two phases.

### phases one
We're going to build BlobKV on the block device abstraction layer by providing a set of key/value APIs for object store. We can use the block device abstraction layer to firstly convert an object store into a block store, and then from a block store into a blob one, which simply means mapping key/value to the logical block address, and then to physical block address (from key/value to LBA, then from LBA to PBA).

### phases two
After the phases one is completed, we're going to design the key/value device abstraction layer on the same floor of the block device abstraction layer which can provide:

* a driver module API for implementing key/value object drivers for Blob object, NVMe, etc.
* an application API for enumerating and claiming SPDK key/value object devices and performance operations (read, write, unmap, etc.) on those devices.
    
By completely integrating the above two phases, it goes in line with our original proposal. 

