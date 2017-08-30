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
    
Our original design was to build the key/value device abstraction layer on the same floor of the block device abstraction layer，and the key/value device abstraction layer connects up to the BlobKV and down to the NVMe device. Simply, it means key/value is directly mapped to the physical block address (like key/value --> PBA). However,  neither the BlobKV nor key/value device abstraction layer are in the SPDK architecture, we are going to implement the design in two phases.

### phases one
We're going to build BlobKV on the block device abstraction layer and design for it to a set of key/value api for object store. We can use the block device abstraction layer to convert object store into block store, and then to convert block store into blob store. Simply, it means key/value is  mapped to the logical block address and then to physical block address (like key/value --> LBA --> PBA).

### phases two
when the phases one is completed, we will succeed in realizing the BlobKV. And then, We're going to design the key/value device abstraction layer on the same floor of the block device abstraction layer. the key/value device abstraction layer can provide:
    * a driver module API for implementing key/value object drivers for Blob object, NVMe, etc.
    * an application API for enumerating and claiming SPDK key/value object devices and performance operations (read, write, unmap, etc.) on those devices.
Complete integration of the two phases，which is in line with our original goal. 

