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

TOTAL_RAM=$(awk '$1 == "MemTotal:" { print $2; exit }' < /proc/meminfo)
GTT_RAM=$((${TOTAL_RAM}*3/(4*1024)))
# --peermem_size must be large enough to fit all DGMA buffers of maximum frame
# size (each 4k HDR is ~16 MB, 8k HDR is 62 MB). A game with 3 FB will use 48MB
# but we need more to work around BO locking issue so set to 192 MB.
PEER_SIZE=192

modprobe amdgpu gttsize="${GTT_RAM}" sched_jobs=1024 sched_hw_submission=8 \
  peermem_size="${PEER_SIZE}" no_evict=1

# Allow RGP to set the DPM level to profile_standard when host is in SIMPERF
for f in /sys/class/drm/card?/device/power_dpm_force_performance_level; do
  if [ -e "$f" ]; then
    chmod a+w "$f"
  fi
done
