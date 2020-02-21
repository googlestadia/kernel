#!/bin/bash

# This script does the following:
# * Builds a new Docker image for the kernel build process
# * Tags the image with the correct name and pushes it to gcr.io
# * Updates repo references to previous versions of this image to the new
#   image (dirties repo)
#
# After this script is run, the changes made to this repo should be committed,
# reviewed, and merged.

set -ex

readonly SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
readonly GIT_ROOT=${SCRIPTPATH}/../..
readonly IMAGE_TAG="gcr.io/stadia-open-source/build/kernel"

pushd ${GIT_ROOT}

# Create a temporary directory for this build to serve as the build context
readonly TMP_DIR=$(mktemp -d -t build_kernel_XXXXXXXX)
readonly CURRENT_DIR=$(pwd)
trap "cd '${CURRENT_DIR}' ; rm -rf '${TMP_DIR}'" EXIT

# Push the image to our gcr.io image repository, and return the SHA of the image
docker_push() {
  local name="$1"
  echo "$(docker push ${name} | awk '/latest:/{print $3}')"
}

# Copy the Dockerfile and dependencies to the build context
cp -p ${SCRIPTPATH}/Dockerfile ${TMP_DIR}
cp -p ${SCRIPTPATH}/fix_permissions.sh ${TMP_DIR}

# The docker build must take place within the same directory as the Dockerfile
pushd ${TMP_DIR}

# Container build
docker build \
  -t "${IMAGE_TAG}" \
  .

# Login
gcloud auth print-access-token | docker login -u oauth2accesstoken --password-stdin https://gcr.io

# Container push
readonly HASH=$(docker_push "${IMAGE_TAG}")

popd # $TMP_DIR

# Update all the references to image hashes in the repository
grep -rl "${IMAGE_TAG}@sha256" | \
  xargs -r sed -i "s;${IMAGE_TAG}@sha256:[a-f0-9]\+;${IMAGE_TAG}@${HASH};g"

popd # $GIT_ROOT
