# Stadia kernel Docker build image

The Stadia kernel is built in a Docker image. Images are stored on the
stadia-open-source Google Cloud container registry.

Use Cloud Build to build a new image and host it on the registry:

```shell
gcloud builds submit --tag gcr.io/stadia-open-source/kernel --project stadia-open-source <path to this directory>
```

Once the build is done, update the image reference in `image.sh`.
