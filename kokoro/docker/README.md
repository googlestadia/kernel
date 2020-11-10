# Stadia kernel Docker build image

The Stadia kernel is built in a Docker image. Images are stored on the
stadia-open-source Google Cloud container registry.

Use Cloud Build to build a new image and host it on the registry:

```shell
gcloud builds submit \
  --tag=gcr.io/stadia-open-source/kernel/debian9 \
  --project=stadia-open-source \
  --machine-type=n1-highcpu-32 \
  <path to this directory>
```
