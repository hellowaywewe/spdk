# BlobKV (Blobstore Object) Design Proposal

## Use Case

Serverless computing is a FaaS ( Function as a Service ) framework.  When client sends GET/PUT request (kv),
Serverless can receive your request and start a function docker image to process it. If the request involves data 
processing,Serverless will invoke the back-end storage service to connect with the kv application (such as AWS S3, Swift). 
At this moment, we need this function to speed up the service respond time and improve performance, so we can use 
SPDK to provide blobstore object to accelerate it.

## Proposed Architecture Overview

![SPDK BlobKV Archtecture Proposal](https://github.com/hellowaywewe/spdk/blob/master/doc/BlobKV.png)

