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

# https://docs.bazel.build/versions/master/user-manual.html#flag--workspace_status_command

# STABLE

git_rev=$(git rev-parse --short HEAD)
if [[ $? != 0 ]]; then
  git_rev=""
fi
echo "STABLE_BUILD_SCM_REVISION ${git_rev:-unknown}"

git_branch=$(git rev-parse --abbrev-ref HEAD)
if [[ $? != 0 ]]; then
  git_branch=""
fi
echo "STABLE_BUILD_SCM_BRANCH ${git_branch:-unknown}"

# Include kokoro information if available.
if [[ -n "${KOKORO_BUILD_ID}" ]]; then
  echo "STABLE_KOKORO_BUILD_ID ${KOKORO_BUILD_ID}"
fi
if [[ -n "${KOKORO_BUILD_NUMBER}" ]]; then
  echo "STABLE_KOKORO_BUILD_NUMBER ${KOKORO_BUILD_NUMBER}"
fi
if [[ -n "${KOKORO_JOB_NAME}" ]]; then
  echo "STABLE_KOKORO_JOB_NAME ${KOKORO_JOB_NAME}"
fi
if [[ -n "${KOKORO_JOB_TYPE}" ]]; then
  echo "STABLE_KOKORO_JOB_TYPE ${KOKORO_JOB_TYPE}"
fi

# Include rapid information if available.
if [[ -n "${RAPID_CANDIDATE_NAME}" ]]; then
  echo "STABLE_RAPID_CANDIDATE_NAME ${RAPID_CANDIDATE_NAME}"
fi

# VOLATILE
