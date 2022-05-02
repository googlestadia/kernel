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

set -ex

# Move all non-build artifacts out of $KOKORO_ARTIFACTS_DIR
readonly KOKORO_NONBUILD_ARTIFACTS="${KOKORO_ROOT}/nonbuild"
mkdir "${KOKORO_NONBUILD_ARTIFACTS}"
mv "${KOKORO_ARTIFACTS_DIR}/git" "${KOKORO_NONBUILD_ARTIFACTS}/"
mv "${KOKORO_KEYSTORE_DIR}" "${KOKORO_NONBUILD_ARTIFACTS}/"
mv "${KOKORO_GFILE_DIR}" "${KOKORO_NONBUILD_ARTIFACTS}/"

rm -rf "${KOKORO_ARTIFACTS_DIR}"/*

# Authenticate to google cloud.
gcloud auth activate-service-account --key-file \
  "${KOKORO_NONBUILD_ARTIFACTS}/keystore/71274_kokoro_service_key_json"
gcloud auth configure-docker

# If the NVIDIA driver version is defined download the required files
# to build the NVIDIA driver disk.
# If not defined download the required files to build the kernel disk
# with the amdgpu modules
if [[ ! -z ${NVIDIA_DRIVER_VERSION} ]]; then
  readonly nvidia_driver_url="gs://yeti_graphics_drivers/nvidia/nvidia-${NVIDIA_DRIVER_VERSION}.tar.gz"
  gsutil cp "${nvidia_driver_url}" "${KOKORO_NONBUILD_ARTIFACTS}/gfile/nvidia-drivers.tar.gz"
else
  # Move the amdgpu firmware file to its canonical name for the build script.
  mv "${KOKORO_NONBUILD_ARTIFACTS}/gfile/amdgpu-firmware-"*.tar.gz \
    "${KOKORO_NONBUILD_ARTIFACTS}/gfile/amdgpu-firmware.tar.gz"
fi

docker pull gcr.io/stadia-open-source/kernel/debian9:latest
docker run \
  --volume "${TMPDIR}":/workspace/tmp \
  --volume "${KOKORO_ARTIFACTS_DIR}":/workspace/artifacts \
  --volume "${KOKORO_NONBUILD_ARTIFACTS}"/git/gamelet_kernel:/workspace/src/kernel \
  --volume "${KOKORO_NONBUILD_ARTIFACTS}"/keystore:/workspace/keystore \
  --volume "${KOKORO_NONBUILD_ARTIFACTS}"/gfile:/workspace/gfile \
  --volume "${HOME}":/workspace/home \
  --volume /dev:/dev \
  --env "HOME=/workspace/home" \
  --env "USER=dockerbuilder" \
  --env "DOCKER_ARTIFACTS_DIR=/workspace/artifacts" \
  --env "DOCKER_TMP_DIR=/workspace/tmp" \
  --env "DOCKER_SRC_DIR=/workspace/src/kernel" \
  --env "DOCKER_GFILE_DIR=/workspace/gfile" \
  --env "DOCKER_KEYSTORE_DIR=/workspace/keystore" \
  --env "KOKORO_BUILD_ID=${KOKORO_BUILD_ID}" \
  --env "KOKORO_BUILD_NUMBER=${KOKORO_BUILD_NUMBER}" \
  --env "KOKORO_JOB_NAME=${KOKORO_JOB_NAME}" \
  --env "KOKORO_JOB_TYPE=${KOKORO_JOB_TYPE}" \
  --env "NVIDIA_DRIVER_VERSION=${NVIDIA_DRIVER_VERSION}" \
  --env "RAPID_CANDIDATE_NAME=${RAPID_CANDIDATE_NAME}" \
  --env "TMP=/workspace/tmp" \
  --net=host \
  --privileged=true \
  --tty \
  gcr.io/stadia-open-source/kernel/debian9:latest \
  /container_tools/fix_permissions.sh --user "$(id -u):$(id -g)" \
  -- \
  /workspace/src/kernel/kokoro/build.sh
