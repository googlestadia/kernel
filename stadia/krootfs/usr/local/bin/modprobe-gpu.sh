#!/bin/bash
TOTAL_RAM=$(awk '$1 == "MemTotal:" { print $2; exit }' < /proc/meminfo)
GTT_RAM=$((${TOTAL_RAM}*3/(4*1024)))
# --peermem_size must be large enough to fit all DGMA buffers of maximum frame
# size (each 4k HDR is ~16 MB, 8k HDR is 62 MB). Sizes larger than 64 MB are
# subdivided but we need more to work around BO locking issue so set to 128 MB.
PEER_SIZE=128

modprobe amdgpu gttsize="${GTT_RAM}" sched_jobs=256 sched_hw_submission=8 \
  peermem_size="${PEER_SIZE}" no_evict=1

# Allow RGP to set the DPM level to profile_standard when host is in SIMPERF
for f in /sys/class/drm/card?/device/power_dpm_force_performance_level; do
  if [ -e "$f" ]; then
    chmod a+w "$f"
  fi
done
