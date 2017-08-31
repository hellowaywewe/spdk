# BlobKV (Blobstore Object) Design Proposal

## Use Case

Serverless computing is a FaaS ( Function as a Service ) framework.  When client sends key/value request ( GET/PUT, for example ), Serverless can receive your request and start a docker image the function to handle it. If the request involves data processing, Serverless will invoke the back-end storage service to connect with the application level k-v (such as AWS S3, Swift). For this reason, the speed of service and performance is critical, and therefore it would be great to use SPDK to accelerate the storage service at the back end.

## Proposed Architecture Overview

![SPDK BlobKV Archtecture Proposal](https://github.com/hellowaywewe/spdk/blob/spdk-objectstore/doc/blobkv.png)

- blobkv provides object storage semantics and key-value pair based API, similar to the filesystem interface provided by blobfs.

- blobkv builds upon existing infrastructure provided by blobstore and bdev layers
    
## Blobkv design proposal
![SPDK BlobKV Archtecture Proposal](https://github.com/hellowaywewe/spdk/blob/spdk-objectstore/doc/kv_api.png)

 

