#!/bin/bash

# Move all non-build artifacts out of $KOKORO_ARTIFACTS_DIR

readonly KOKORO_NONBUILD_ARTIFACTS=${KOKORO_ROOT}/nonbuild
mkdir ${KOKORO_NONBUILD_ARTIFACTS}
mkdir -p ${KOKORO_NONBUILD_ARTIFACTS}/git

mv ${KOKORO_ARTIFACTS_DIR}/git/gamelet_kernel ${KOKORO_NONBUILD_ARTIFACTS}/git
mv ${KOKORO_ARTIFACTS_DIR}/keystore ${KOKORO_NONBUILD_ARTIFACTS}

rm -rf ${KOKORO_ARTIFACTS_DIR}/*

gcloud auth activate-service-account \
  --key-file ${KOKORO_NONBUILD_ARTIFACTS}/keystore/71274_kokoro_service_key_json

gcloud auth configure-docker

docker run \
  --volume ${TMPDIR}:/workspace/tmp \
  --volume ${KOKORO_ARTIFACTS_DIR}:/workspace/artifacts \
  --volume ${KOKORO_NONBUILD_ARTIFACTS}/git/gamelet_kernel:/workspace/src/kernel \
  --volume ${KOKORO_NONBUILD_ARTIFACTS}/keystore:/workspace/keystore \
  --volume ${HOME}:/workspace/home \
  --volume /dev:/dev \
  --env "HOME=/workspace/home" \
  --env "USER=dockerbuilder" \
  --env "DOCKER_ARTIFACTS_DIR=/workspace/artifacts" \
  --env "DOCKER_TMP_DIR=/workspace/tmp" \
  --env "DOCKER_SRC_DIR=/workspace/src/kernel" \
  --env "DOCKER_GCLOUD_KEY_FILE=/workspace/keystore/71274_kokoro_service_key_json" \
  --env "KOKORO_BUILD_ID=${KOKORO_BUILD_ID}" \
  --env "KOKORO_BUILD_NUMBER=${KOKORO_BUILD_NUMBER}" \
  --env "KOKORO_JOB_NAME=${KOKORO_JOB_NAME}" \
  --env "KOKORO_JOB_TYPE=${KOKORO_JOB_TYPE}" \
  --env "RAPID_CANDIDATE_NAME=${RAPID_CANDIDATE_NAME}" \
  --env "TMP=/workspace/tmp" \
  --net=host \
  --privileged=true \
  -t gcr.io/stadia-open-source/build/kernel@sha256:388ff48eea75ffcdd027bf7e7109e21aaae7f857ddd2880aa544f47f26f0c82d \
  /container_tools/fix_permissions.sh --user "$(id -u):$(id -g)" \
  -- \
  /workspace/src/kernel/kokoro/ubuntu/gamelet_kernel/build.sh
