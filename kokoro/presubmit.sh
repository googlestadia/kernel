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

readonly SCRIPT_DIR=$(dirname "$(readlink -f "${0}")")
readonly PRESUBMIT_ARTIFACTS="${KOKORO_ROOT}/presubmit"
mkdir ${PRESUBMIT_ARTIFACTS}

source ${SCRIPT_DIR}/check_commit_tags.sh "${KOKORO_ARTIFACTS_DIR}"
#source ${SCRIPT_DIR}/check_commit_tags.sh ".git"

# move mpm to presubmit artifacts
mv "${KOKORO_ARTIFACTS_DIR}/mpm" "${PRESUBMIT_ARTIFACTS}/"

# keep a copy of service key
cp "${KOKORO_ARTIFACTS_DIR}/keystore/71274_kokoro_service_key_json" "${PRESUBMIT_ARTIFACTS}/"

# start kernel build
${SCRIPT_DIR}/docker/kokoro_launcher.sh

export ARTIFACTS_DIR=guitar_artifacts
mkdir -p ${ARTIFACTS_DIR}
cp -av ${KOKORO_ARTIFACTS_DIR}/mpm/chrome/cloudcast/kernel/gamelet_disk/disk.raw "${ARTIFACTS_DIR}"/kernel_disk.raw

cat << EOF > /tmp/pkgdef
pkgdef mpm = {
  package_name = 'test/chrome/cloudcast/kernel/gamelet_disk'
  relative_path_anchor = 'PKGDEF_DIR'
  source = [
    {
      package_path = "disk.raw"
      source_file = "${KOKORO_ARTIFACTS_DIR}/mpm/chrome/cloudcast/kernel/gamelet_disk/disk.raw"
    },
  ]
}
EOF

label="presubmit_${KOKORO_GERRIT_REVISION}"

"${PRESUBMIT_ARTIFACTS}/mpm/tools/mpm_build" \
  --pkgdef_file=/tmp/pkgdef \
  --label="${label}" \
  --service_key="${PRESUBMIT_ARTIFACTS}/71274_kokoro_service_key_json"

RUN_FULL_FOUND=$(check_tag "Run_Full_Presubmit")

if [ -z "${RUN_FULL_FOUND}" ]; then
  echo "Running libdrm presubmit"
  ${PRESUBMIT_ARTIFACTS}/mpm/tools/guitar_presubmit \
    --service_key="${PRESUBMIT_ARTIFACTS}/71274_kokoro_service_key_json" \
    --env_params=version_kernel="${label}" \
    --test_type=LIBDRM --wait
else
  echo "Running full presubmit tests workflow"
  ${PRESUBMIT_ARTIFACTS}/mpm/tools/guitar_presubmit \
    --service_key="${PRESUBMIT_ARTIFACTS}/71274_kokoro_service_key_json" \
    --env_params=version_kernel="${label}" \
    --test_type=GUEST_KERNEL_LARGE --wait
fi

${PRESUBMIT_ARTIFACTS}/mpm/tools/guitar_presubmit \
  --service_key="${PRESUBMIT_ARTIFACTS}/71274_kokoro_service_key_json" \
  --env_params=version_kernel="${label}" \
  --test_type=GUEST_KERNEL_VIDEO --wait

