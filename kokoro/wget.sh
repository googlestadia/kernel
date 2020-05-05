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

set -xe

# Downloads a file and verifies its authenticity using a checksum.
function download_wget() {
  local -r url="$1"
  local dst_path="$2"
  local -r sha256sum="$3"

  if [[ -d "${dst_path}" ]]; then
    dst_path="${dst_path%/}/$(basename "${url}")"
  fi

  if [[ -f "${dst_path}" ]]; then
    echo "${sha256sum}  ${dst_path}" | sha256sum --quiet -c
    if [[ $? -eq 0 ]]; then
      echo "${dst_path}"
      return
    fi
  fi

  wget -q -O "${dst_path}" "${url}"
  if [[ $? -ne 0 ]]; then
    return 1
  fi
  echo "${sha256sum}  ${dst_path}" | sha256sum --quiet -c
  if [[ $? -ne 0 ]]; then
    return 1
  fi

  echo "${dst_path}"
}
