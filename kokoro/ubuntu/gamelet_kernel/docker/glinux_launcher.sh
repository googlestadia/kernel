#!/bin/bash

# This script will build the gamelet_kernel on a gLinux workstation.
#
# In order to run this successfully, you must:
# * Install and configure docker according to go/docker#installation
# * Configure sudoless docker: go/docker#sudoless-docker
# * Auth to gcloud: `gcloud auth login`

set -ex

readonly SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
readonly GIT_ROOT=${SCRIPTPATH}/../../../..
readonly BUILD_DIR=$(mktemp -d -t kernel_build_XXXXXXXX)

mkdir "${BUILD_DIR}/tmp"
mkdir "${BUILD_DIR}/artifacts"

docker run \
  --volume ${BUILD_DIR}/tmp:/workspace/tmp \
  --volume ${BUILD_DIR}/artifacts:/workspace/artifacts \
  --volume ${GIT_ROOT}:/workspace/src/kernel \
  --volume ${HOME}:/workspace/home \
  --volume /dev:/dev \
  --env "HOME=/workspace/home" \
  --env "USER=dockerbuilder" \
  --env "DOCKER_ARTIFACTS_DIR=/workspace/artifacts" \
  --env "DOCKER_TMP_DIR=/workspace/tmp" \
  --env "DOCKER_SRC_DIR=/workspace/src/kernel" \
  --net=host \
  --privileged=true \
  -t gcr.io/google.com/stadia-engprod/build/kernel@sha256:388ff48eea75ffcdd027bf7e7109e21aaae7f857ddd2880aa544f47f26f0c82d \
  /container_tools/fix_permissions.sh --user "$(id -u):$(id -g)" \
  -- \
  /workspace/src/kernel/kokoro/ubuntu/gamelet_kernel/build.sh

echo "Build successfully created in: ${BUILD_DIR}"
