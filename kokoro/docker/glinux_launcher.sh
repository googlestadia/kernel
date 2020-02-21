#!/bin/bash

# This script will build the gamelet_kernel on a gLinux workstation.
# Args:
#   1. Build directory. If omitted, will create and build in a temp directory.
#      Provide this arg to reuse existing artifacts for an incremental build.
#
# In order to run this successfully, you must:
# * Install and configure docker according to go/docker#installation
# * Configure sudoless docker: go/docker#sudoless-docker
# * Auth to gcloud: `gcloud auth login`

set -ex

readonly SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
readonly GIT_ROOT=${SCRIPTPATH}/../..

BUILD_DIR="$1"
if [[ -z "${BUILD_DIR}" ]] ; then
  BUILD_DIR=$(mktemp -d -t kernel_build_XXXXXXXX)
fi

mkdir -p "${BUILD_DIR}/tmp"
mkdir -p "${BUILD_DIR}/artifacts"

if hash podman 2>/dev/null; then
  ENGINE_BIN="podman --runtime /usr/local/bin/crun"
elif hash docker 2>/dev/null; then
  ENGINE_BIN=docker
else
  echo "No available container engine. Install docker or podman with crun."
  exit 1
fi

${ENGINE_BIN} run \
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
  -t gcr.io/stadia-open-source/build/kernel@sha256:388ff48eea75ffcdd027bf7e7109e21aaae7f857ddd2880aa544f47f26f0c82d \
  /container_tools/fix_permissions.sh --user "$(id -u):$(id -g)" \
  -- \
  /workspace/src/kernel/kokoro/build.sh

echo "Build successfully created in: ${BUILD_DIR}"
