#!/bin/bash
# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script will build the Stadia instance kernel on a Linux workstation.
# Args:
#   1. Build directory. If omitted, will create and build in a temp directory.
#      Provide this arg to reuse existing artifacts for an incremental build.
#
# In order to run this successfully, you must install a container engine
# (either Docker or podman).

set -ex

readonly SCRIPT_DIR=$(dirname "$(readlink -f "${0}")")
readonly GIT_ROOT=${SCRIPT_DIR}/../..

source "${SCRIPT_DIR}/../wget.sh"

BUILD_DIR="$1"
if [[ -z "${BUILD_DIR}" ]] ; then
  BUILD_DIR=$(mktemp -d -t stadia_kernel_build_XXXXXXXX)
fi

mkdir -p "${BUILD_DIR}/tmp" "${BUILD_DIR}/artifacts" "${BUILD_DIR}/gfile"

# Detect the container engine to use. Prefer podman.
if hash podman 2>/dev/null; then
  ENGINE_BIN="sudo podman"
elif hash docker 2>/dev/null; then
  ENGINE_BIN=docker
else
  echo "No available container engine. Install docker or podman."
  exit 1
fi

if [[ ! -z "${NVIDIA_DRIVER_PATH}" ]]; then
  echo "Copying NVIDIA driver package from ${NVIDIA_DRIVER_PATH}."
  cp "${NVIDIA_DRIVER_PATH}" "${BUILD_DIR}/gfile/nvidia-drivers.tar.gz"

  readonly nvidia_filename=$(basename -- ${NVIDIA_DRIVER_PATH})
  readonly NVIDIA_DRIVER_VERSION=$(echo "$nvidia_filename" | grep -Eo '[0-9]+[.]+[0-9]+')
else
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
fi

${ENGINE_BIN} pull gcr.io/stadia-open-source/kernel/debian9:latest
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
  --env "NVIDIA_DRIVER_VERSION=${NVIDIA_DRIVER_VERSION}" \
  --net=host \
  --privileged=true \
  --tty -i \
  gcr.io/stadia-open-source/kernel/debian9:latest \
  /container_tools/fix_permissions.sh --user "$(id -u):$(id -g)" \
  -- \
  /workspace/src/kernel/kokoro/build.sh

echo "Build successfully created in: ${BUILD_DIR}"
