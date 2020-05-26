#!/bin/bash
set -xe

readonly RELEASE_SCRIPT_DIR=$(dirname "$(readlink -f "${0}")")

function release_check_kokoro_env() {
  if [[ ! -d ${KOKORO_ARTIFACTS_DIR} ]]; then
    echo "KOKORO_ARTIFACTS_DIR is not a valid directory path."
    exit 1
  fi
  export KOKORO_ARTIFACTS_DIR="${KOKORO_ARTIFACTS_DIR%/}"
}

function stage_artifacts() {
  readonly artifacts_dir="${KOKORO_ARTIFACTS_DIR}/artifacts"
  readonly mpm_dir="${artifacts_dir}/mpm"

  rm -rf "${artifacts_dir}"
  mkdir "${artifacts_dir}"

  cp -a "${LINUX_TAR_XZ}" "${artifacts_dir}/"
  cp -a "${BOOT_DISK_GZ}" "${artifacts_dir}/"
  cp -a "${PERF_TAR_XZ}" "${artifacts_dir}/"
  cp -a "${VMLINUX}" "${artifacts_dir}/"

  readonly kernel_mpm_dir="${mpm_dir}"/chrome/cloudcast/kernel/gamelet
  mkdir -p "${kernel_mpm_dir}"
  cp -a "${TAR_INITRD}" "${kernel_mpm_dir}/initrd"
  cp -a "${TAR_VMLINUZ}" "${kernel_mpm_dir}/vmlinuz"

  # See https://g3doc.corp.google.com/cloud/network/edge/g3doc/vm_disk.md#creating-an-mpm-disk-package
  readonly kernel_disk_mpm_dir="${mpm_dir}"/chrome/cloudcast/kernel/gamelet_disk
  mkdir -p "${kernel_disk_mpm_dir}"
  cp -a "${BOOT_DISK}" "${kernel_disk_mpm_dir}/disk.raw"
}

release_check_kokoro_env
. "${RELEASE_SCRIPT_DIR}"/build.sh
stage_artifacts
