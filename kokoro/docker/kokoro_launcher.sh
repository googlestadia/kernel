#!/bin/bash

# Move all non-build artifacts out of $KOKORO_ARTIFACTS_DIR

readonly KOKORO_NONBUILD_ARTIFACTS="${KOKORO_ROOT}/nonbuild"
mkdir "${KOKORO_NONBUILD_ARTIFACTS}"
mkdir -p "${KOKORO_NONBUILD_ARTIFACTS}/git"

mv "${KOKORO_ARTIFACTS_DIR}/git/gamelet_kernel" \
  "${KOKORO_NONBUILD_ARTIFACTS}/git"
mv "${KOKORO_KEYSTORE_DIR}" "${KOKORO_NONBUILD_ARTIFACTS}"
mv "${KOKORO_GFILE_DIR}" "${KOKORO_NONBUILD_ARTIFACTS}"

rm -rf "${KOKORO_ARTIFACTS_DIR}"/*

# Move the amdgpu firmware file to its canonical name for the build script.
mv "${KOKORO_NONBUILD_ARTIFACTS}/gfile/amdgpu-firmware-2019.3.tar.gz" \
  "${KOKORO_NONBUILD_ARTIFACTS}/gfile/amdgpu-firmware.tar.gz"

# Authenticate to google cloud.
gcloud auth activate-service-account --key-file \
  "${KOKORO_NONBUILD_ARTIFACTS}/keystore/71274_kokoro_service_key_json"
gcloud auth configure-docker

docker run \
  --volume ${TMPDIR}:/workspace/tmp \
  --volume ${KOKORO_ARTIFACTS_DIR}:/workspace/artifacts \
  --volume ${KOKORO_NONBUILD_ARTIFACTS}/git/gamelet_kernel:/workspace/src/kernel \
  --volume ${KOKORO_NONBUILD_ARTIFACTS}/keystore:/workspace/keystore \
  --volume ${KOKORO_NONBUILD_ARTIFACTS}/gfile:/workspace/gfile \
  --volume ${HOME}:/workspace/home \
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
  --env "RAPID_CANDIDATE_NAME=${RAPID_CANDIDATE_NAME}" \
  --env "TMP=/workspace/tmp" \
  --net=host \
  --privileged=true \
  -t gcr.io/stadia-open-source/build/kernel@sha256:476f6e5d2c4f4ef4ec25773d1b1a9bb48cf9d65a1275adad39611f8ed0185e8f \
  /container_tools/fix_permissions.sh --user "$(id -u):$(id -g)" \
  -- \
  /workspace/src/kernel/kokoro/build.sh
