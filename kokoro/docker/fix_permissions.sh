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

# Usage:
#   fix_permissions.sh --user 1000:1000 -- /some/script --with --args

while [ "$1" != "" ] ; do
  case $1 in
    --user)
      shift
      readonly USER_GROUP="$1"
      shift
      ;;
    --)
      shift
      break
      ;;
    *)
      shift
      ;;
  esac
done

readonly TO_RUN="$1"
shift
${TO_RUN} "$@"
readonly RETCODE=$?

chown -hR ${USER_GROUP} /workspace/artifacts

exit ${RETCODE}
