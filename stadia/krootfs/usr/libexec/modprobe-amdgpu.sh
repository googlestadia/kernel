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

# Set the GTT size to 3/4 of total system memory. This is the maximum size
# recommended by AMD and more or less ensures amdgpu won't be able to consume
# all system memory.
readonly TOTAL_RAM=$(awk '$1 == "MemTotal:" { print $2; exit }' < /proc/meminfo)
readonly GTT_SIZE_MB=$((${TOTAL_RAM}*3/(4*1024)))

# --peermem_size must be large enough to fit all DGMA buffers of maximum frame
# size (each 4k HDR is ~16 MB, 8k HDR is 62 MB). A game with 3 FB will use 48MB
# but we need more to work around BO locking issue so set to 192 MB.
readonly PEERMEM_SIZE_MB=192

# modprobe amdgpu.
# - sched_jobs=1024 increases the number of inflight submissions before amdgpu
#   has to wait on a submission fence, at a minimal memory cost.
# - sched_hw_submission=8 increases the number of inflight hardware submissions
#   before amdgpu has to wait for hardware completion. The Stadia hardware and
#   firmware support at least 8, and this helps reduce submission latency.
# - no_evict allows user-mode to pin kernel buffer objects.
modprobe amdgpu sched_jobs=1024 sched_hw_submission=8 no_evict=1 \
  gttsize="${GTT_SIZE_MB}" peermem_size="${PEERMEM_SIZE_MB}"

# Allow any user to write to `power_dpm_force_performance_level` so that
# non-privileged processes (tools and amdvlk.so) can control the DPM enforcement
# mode. This is used by amdvlk when recording a Radeon Graphics Profiler trace
# in particular.
for f in /sys/class/drm/card?/device/power_dpm_force_performance_level; do
  if [ -e "$f" ]; then
    chmod a+w "$f"
  fi
done

# Route all amdgpu interrupts to vCPU0 by default. This ensures minimal
# interrupt handling latency on Stadia VMs.
readonly gpu_irq=`grep amdgpu /proc/interrupts | awk -F: '{print $1}' | tr -d '[:space:]'`
readonly irq_path="/proc/irq/${gpu_irq}/smp_affinity"
if [ -f "${irq_path}" ]; then
  echo 1 > "${irq_path}"
fi
