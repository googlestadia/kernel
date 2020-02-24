#!/bin/bash

# This script will build the Stadia instance kernel on a Linux workstation.
# Args:
#   1. Build directory. If omitted, will create and build in a temp directory.
#      Provide this arg to reuse existing artifacts for an incremental build.
#
# In order to run this successfully, you must:
# * Install and configure docker.
# * Install the Google Cloud SDK.

set -ex

readonly SCRIPT_DIR=$(dirname "$(readlink -f "${0}")")
readonly GIT_ROOT=${SCRIPT_DIR}/../..

source "${SCRIPT_DIR}/../wget.sh"

BUILD_DIR="$1"
if [[ -z "${BUILD_DIR}" ]] ; then
  BUILD_DIR=$(mktemp -d -t stadia_kernel_build_XXXXXXXX)
fi

mkdir -p "${BUILD_DIR}/tmp" "${BUILD_DIR}/artifacts" "${BUILD_DIR}/gfile"

# Detect the container engine to use.
if hash podman 2>/dev/null; then
  ENGINE_BIN="podman --runtime /usr/local/bin/crun"
elif hash docker 2>/dev/null; then
  ENGINE_BIN=docker
else
  echo "No available container engine. Install docker or podman with crun."
  exit 1
fi

# Download the amdgpu firmware. Set "AMDGPU_FIRMWARE_URL" in the environment to
# override the default package.
readonly DEFAULT_AMDGPU_FIRMWARE_URL="https://storage.googleapis.com/stadia_kernel_public/amdgpu-firmware/amdgpu-firmware-19.50.tar.gz"
readonly DEFAULT_AMDGPU_FIRMWARE_SHA256="89785ad581781bbdb98902ab82bce95bf12a861b81b84731020d7ba63a1a1533"
if [[ -z "${AMDGPU_FIRMWARE_URL}" ]]; then
  AMDGPU_FIRMWARE_URL="${DEFAULT_AMDGPU_FIRMWARE_URL}"
  AMDGPU_FIRMWARE_SHA256="${DEFAULT_AMDGPU_FIRMWARE_SHA256}"
fi
download_wget "${AMDGPU_FIRMWARE_URL}" \
  "${BUILD_DIR}/gfile/amdgpu-firmware.tar.gz" "${AMDGPU_FIRMWARE_SHA256}"

${ENGINE_BIN} run \
  --volume ${BUILD_DIR}/tmp:/workspace/tmp \
  --volume ${BUILD_DIR}/artifacts:/workspace/artifacts \
  --volume ${BUILD_DIR}/gfile:/workspace/gfile \
  --volume ${GIT_ROOT}:/workspace/src/kernel \
  --volume ${HOME}:/workspace/home \
  --volume /dev:/dev \
  --env "HOME=/workspace/home" \
  --env "USER=dockerbuilder" \
  --env "DOCKER_ARTIFACTS_DIR=/workspace/artifacts" \
  --env "DOCKER_GFILE_DIR=/workspace/gfile" \
  --env "DOCKER_TMP_DIR=/workspace/tmp" \
  --env "DOCKER_SRC_DIR=/workspace/src/kernel" \
  --net=host \
  --privileged=true \
  -t gcr.io/stadia-open-source/build/kernel@sha256:476f6e5d2c4f4ef4ec25773d1b1a9bb48cf9d65a1275adad39611f8ed0185e8f \
  /container_tools/fix_permissions.sh --user "$(id -u):$(id -g)" \
  -- \
  /workspace/src/kernel/kokoro/build.sh

echo "Build successfully created in: ${BUILD_DIR}"
