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

readonly SCRIPT_DIR=$(dirname "$(readlink -f "${0}")")
readonly SRC_DIR=${DOCKER_SRC_DIR}
readonly INITRAMFS_BIN_URL="https://storage.googleapis.com/stadia_kernel_public/initramfs/initramfs-20190312.tar.gz"
readonly INITRAMFS_BIN_SHA256="9790f1a9a859eca95c8ca036416f0ca7d76fdd0bc41c273fcad4636caf4681d3"

source "${SCRIPT_DIR}/wget.sh"

function check_env() {
  if [[ ! -d ${DOCKER_TMP_DIR} ]]; then
    echo "DOCKER_TMP_DIR is not a valid directory path."
    exit 1
  fi
  export ARTIFACT_ROOT="${DOCKER_TMP_DIR%/}/build_output"
  mkdir -p ${ARTIFACT_ROOT}

  if [[ ! -d ${DOCKER_ARTIFACTS_DIR} ]]; then
    echo "DOCKER_ARTIFACTS_DIR is not a valid directory path."
    exit 1
  fi
  export DOCKER_ARTIFACTS_DIR="${DOCKER_ARTIFACTS_DIR%/}"
}

function set_LOCALVERSION_from_buildstamp() {
  pushd "${SRC_DIR}"
  local -r buildstamp_vars_script="${ARTIFACT_ROOT}"/buildstamp.sh
  "${SCRIPT_DIR}"/get_workspace_status > "${ARTIFACT_ROOT}"/workspace-status.txt
  python3 "${SCRIPT_DIR}"/gen_buildstamp_vars.py \
    --output "${buildstamp_vars_script}" \
    --template "${SCRIPT_DIR}"/buildstamp_sh_template.txt \
    --var_format "export {}=\"{}\"" \
    "${ARTIFACT_ROOT}"/workspace-status.txt
  source "${buildstamp_vars_script}"
  if [[ -z ${STABLE_KOKORO_JOB_NAME} ]]; then
    readonly LOCALVERSION="${LOCALVERSION}-local"
  fi
  popd
}

function download_initramfs_artifacts() {
  readonly TMP_INITRAMFS_DIR="${ARTIFACT_ROOT}/initramfs"
  rm -rf "${TMP_INITRAMFS_DIR}"
  mkdir "${TMP_INITRAMFS_DIR}"

  local -r initramfs_bin_archive="$(download_wget "${INITRAMFS_BIN_URL}" "${ARTIFACT_ROOT}" "${INITRAMFS_BIN_SHA256}")"
  tar -xf "${initramfs_bin_archive}" -C "${TMP_INITRAMFS_DIR}/"
}

function install_firmware() {
  readonly FIRMWARE_INSTALL_DIR="${ARTIFACT_ROOT}/firmware-install"
  rm -rf "${FIRMWARE_INSTALL_DIR}"
  mkdir -p "${FIRMWARE_INSTALL_DIR}"

  local -r firmware_install_usr_dir="${FIRMWARE_INSTALL_DIR}/usr/lib/firmware"
  mkdir -p "${firmware_install_usr_dir}"

  local -r amdgpu_firmware_dir="${firmware_install_usr_dir}/amdgpu"
  mkdir -p "${amdgpu_firmware_dir}"
  tar -xf "${DOCKER_GFILE_DIR}/amdgpu-firmware.tar.gz" \
    -C "${amdgpu_firmware_dir}"/
}

function create_kbuild_output() {
  export KBUILD_OUTPUT="${ARTIFACT_ROOT}/obj"
  mkdir "${KBUILD_OUTPUT}" || true
}

function create_initramfs_install_dir() {
  readonly INITRAMFS_INSTALL_DIR="${KBUILD_OUTPUT}/initramfs-install"
  rm -rf "${INITRAMFS_INSTALL_DIR}"
  mkdir "${INITRAMFS_INSTALL_DIR}"
}

function finalize_config() {
  pushd "${SRC_DIR}"
  readonly KCONFIG_CONFIG="${KBUILD_OUTPUT}/.config"
  readonly SRC_CONFIG_FILE="${SCRIPT_DIR}/config"
  cp "${SRC_CONFIG_FILE}" "${KCONFIG_CONFIG}"
  readonly ARCH="x86_64"
  declare -rga MAKE_ARGS=(
    "ARCH=${ARCH}"
    "LOCALVERSION=${LOCALVERSION}"
  )
  if ! make syncconfig "${MAKE_ARGS[@]}"; then
    exit 1
  fi
  if ! KERNELRELEASE="$(make -s kernelrelease "${MAKE_ARGS[@]}")"; then
    exit 1
  fi
  readonly KERNELRELEASE
  if ! IMAGENAME="$(make -s image_name "${MAKE_ARGS[@]}")"; then
    exit 1
  fi
  readonly IMAGENAME
  popd
}

function build_bzimage_and_headers() {
  pushd "${SRC_DIR}"
  readonly INITRD_NAME="initrd.img-${KERNELRELEASE}"
  readonly VMLINUZ_NAME="vmlinuz-${KERNELRELEASE}"
  make -j "$(nproc)" bzImage "${MAKE_ARGS[@]}"
  make -j "$(nproc)" headers_install "${MAKE_ARGS[@]}"
  popd
}

function build_modules() {
  pushd "${SRC_DIR}"
  make -j "$(nproc)" modules "${MAKE_ARGS[@]}"
  readonly MOD_INSTALL_DIR="${KBUILD_OUTPUT}/modules-install"
  rm -rf "${MOD_INSTALL_DIR}"
  local -r mod_install_usr_dir="${MOD_INSTALL_DIR}/usr"
  mkdir -p "${mod_install_usr_dir}"
  make -j "$(nproc)" modules_install "${MAKE_ARGS[@]}" \
    INSTALL_MOD_PATH="${mod_install_usr_dir}" \
    INSTALL_MOD_STRIP=1
  rm \
    "${mod_install_usr_dir}/lib/modules/${KERNELRELEASE}/build" \
    "${mod_install_usr_dir}/lib/modules/${KERNELRELEASE}/source"
  popd
}

function build_amdgpu_external_module() {
  local -r ext_kbuild="${KBUILD_OUTPUT}"/external/amd-cloudgpu
  rm -rf "${ext_kbuild}"
  mkdir -p "${ext_kbuild}"

  # Copy DKMS sources. Based on dkms/sources and dkms/headers.
  pushd "${SRC_DIR}"/external/amd-cloudgpu
  rsync -a \
    drivers/gpu/drm/amd \
    drivers/gpu/drm/ttm \
    drivers/gpu/drm/scheduler \
    drivers/gpu/drm/amd/dkms/ \
    "${ext_kbuild}"/

  mkdir -p "${ext_kbuild}"/include/drm/
  rsync -a \
    include/drm/ttm \
    include/drm/gpu_scheduler.h \
    drivers/gpu/drm/scheduler/gpu_scheduler_trace.h \
    include/drm/amd_asic_type.h \
    include/drm/spsc_queue.h \
    include/drm/amd_rdma.h \
    "${ext_kbuild}"/include/drm/

  mkdir -p "${ext_kbuild}"/include/
  rsync -a \
    include/kcl \
    "${ext_kbuild}"/include/

  mkdir -p "${ext_kbuild}"/include/uapi/drm/
  rsync -a \
    include/uapi/drm/amdgpu_drm.h \
    "${ext_kbuild}"/include/uapi/drm/

  mkdir -p "${ext_kbuild}"/include/uapi/linux/
  rsync -a \
    include/uapi/linux/kfd_ioctl.h \
    "${ext_kbuild}"/include/uapi/linux/

  mkdir -p "${ext_kbuild}"/amd/amdkcl/
  rsync -a \
    drivers/dma-buf/dma-resv.c \
    "${ext_kbuild}"/amd/amdkcl/

  mkdir -p "${ext_kbuild}"/include/linux/
  rsync -a \
    include/linux/dma-resv.h \
    include/kcl/reservation.h \
    "${ext_kbuild}"/include/linux/
  popd

  pushd "${ext_kbuild}"
  ./pre-build.sh 5.10
  popd

  pushd "${SRC_DIR}"
  make -j "$(nproc)" \
    M="${ext_kbuild}" "${MAKE_ARGS[@]}"
  local -r mod_install_usr_dir="${MOD_INSTALL_DIR}/usr"
  make -j "$(nproc)" \
    M="${ext_kbuild}" \
    modules_install "${MAKE_ARGS[@]}" \
    INSTALL_MOD_PATH="${mod_install_usr_dir}" \
    INSTALL_MOD_STRIP=1
  popd
}

function build_nvidia_external_module() {
  readonly NVIDIA_DRIVER_DIR="${KBUILD_OUTPUT}/nvidia-drivers"
  rm -rf "${NVIDIA_DRIVER_DIR}"
  mkdir -p "${NVIDIA_DRIVER_DIR}"

  tar -xf "${DOCKER_GFILE_DIR}/nvidia-drivers.tar.gz" \
    -C "${NVIDIA_DRIVER_DIR}"/

  local -r ext_kbuild="${KBUILD_OUTPUT}"/nvidia-kernel
  rm -rf "${ext_kbuild}"
  mkdir -p "${ext_kbuild}"

  # Copy kernel module source files.
  rsync -a ${NVIDIA_DRIVER_DIR}/kernel/ ${ext_kbuild}

  pushd "${SRC_DIR}"
  make -j "$(nproc)" \
    NV_KERNEL_SOURCES="${SRC_DIR}" \
    NV_KERNEL_OUTPUT="${KBUILD_OUTPUT}" \
    NV_KERNEL_MODULES="nvidia nvidia-uvm nvidia-modeset nvidia-drm" \
    M="${ext_kbuild}" "${MAKE_ARGS[@]}"
  local -r mod_install_usr_dir="${MOD_INSTALL_DIR}/usr"
  ls "${mod_install_usr_dir}/lib/modules/${KERNELRELEASE}"
  make -j "$(nproc)" \
    M="${ext_kbuild}" \
    modules_install "${MAKE_ARGS[@]}" \
    INSTALL_MOD_PATH="${mod_install_usr_dir}" \
    INSTALL_MOD_STRIP=1
  popd
}

function build_kernel_rootfs() {
  # LINT.IfChange
  readonly KROOTFS="${INITRAMFS_INSTALL_DIR}/krootfs.squashfs"
  # LINT.ThenChange(
  #     stadia/initramfs/root-image/init,
  #     stadia/initramfs/root-image/scripts/overlay.sh,
  # )
  readonly KROOTFS_ASSETS="${SRC_DIR}/stadia/krootfs"
  local -r krootfs_install_dir="${KBUILD_OUTPUT}/krootfs-install"
  rm -rf "${KROOTFS}" "${krootfs_install_dir}"
  mkdir "${krootfs_install_dir}"
  chmod 00755 "${krootfs_install_dir}"
  rsync -a --delete \
    "${MOD_INSTALL_DIR}"/ \
    "${FIRMWARE_INSTALL_DIR}"/ \
    "${PERF_INSTALL_DIR}"/ \
    "${KROOTFS_ASSETS}"/ \
    "${krootfs_install_dir}"/
  find "${krootfs_install_dir}" -type d -exec chmod 00755 {} \;
  find "${krootfs_install_dir}/usr/bin" -type f -exec chmod 00755 {} \;
  mksquashfs "${krootfs_install_dir}" "${KROOTFS}" \
    -comp xz -no-exports -all-root -no-progress -no-recovery -Xbcj x86
}

function build_initramfs() {
  pushd "${KBUILD_OUTPUT}"
  readonly INITRAMFS_DATA_CPIO="${KBUILD_OUTPUT}/initramfs_data.cpio"
  readonly INITRAMFS_DATA_CPIO_XZ="${INITRAMFS_DATA_CPIO}.xz"
  "${SRC_DIR}"/usr/gen_initramfs.sh \
    -o "${INITRAMFS_DATA_CPIO}" \
    -u -1 -g -1 \
    "${SRC_DIR}"/stadia/initramfs/root-files \
    "${SRC_DIR}"/stadia/initramfs/root-image \
    "${INITRAMFS_INSTALL_DIR}" \
    "${TMP_INITRAMFS_DIR}"
  xz --threads=0 -f --check=crc32 --lzma2=dict=1MiB "${INITRAMFS_DATA_CPIO}"
  popd
}

function build_linux_tar_xz() {
  pushd "${SRC_DIR}"
  readonly TAR_INSTALL_DIR="${KBUILD_OUTPUT}/tar-install"
  readonly LINUX_TAR_XZ="${KBUILD_OUTPUT}/linux-${KERNELRELEASE}-${ARCH}.tar.xz"
  readonly VMLINUX="${KBUILD_OUTPUT}/vmlinux"
  local -r tar_boot_dir="${TAR_INSTALL_DIR}/boot"
  readonly TAR_INITRD="${tar_boot_dir}/${INITRD_NAME}"
  readonly TAR_VMLINUZ="${tar_boot_dir}/${VMLINUZ_NAME}"
  rm -rf "${TAR_INSTALL_DIR}"
  mkdir -p "${tar_boot_dir}"
  cp -a "${KBUILD_OUTPUT}/${IMAGENAME}" "${TAR_VMLINUZ}"
  cp -a "${KBUILD_OUTPUT}/System.map" \
    "${tar_boot_dir}/System.map-${KERNELRELEASE}"
  cp -a "${KCONFIG_CONFIG}" "${tar_boot_dir}/config-${KERNELRELEASE}"
  cp -a "${INITRAMFS_DATA_CPIO_XZ}" "${TAR_INITRD}"
  # Getting the permissions wrong on /, /usr, or /boot is disastrous.
  # Kokoro sets the sgid bit on its root build folder, which must not make it
  # into the tar archive. Use GNU tar's --mode flag to forcefully remove them.
  # https://cs.corp.google.com/piper///depot/google3/devtools/kokoro/vanadium/linux_scripts/usr/local/bin/format_tmpfs.sh
  chmod 00755 "${TAR_INSTALL_DIR}" "${TAR_INSTALL_DIR}"/*
  tar -cf - --hard-dereference --owner=root --group=root --mode='u-s,g-s' \
    -C "${TAR_INSTALL_DIR}" . | xz --threads=0 > "${LINUX_TAR_XZ}"
  # Print the content of the archive for the build log.
  tar -tvf "${LINUX_TAR_XZ}"
  ln -sf "$(basename "${LINUX_TAR_XZ}")" \
    "$(dirname "${LINUX_TAR_XZ}")/linux-latest.tar.xz"
  popd
}

function build_boot_disk() {
  readonly BOOT_DISK_NAME="disk-${KERNELRELEASE}-${ARCH}.raw"
  readonly BOOT_DISK="${KBUILD_OUTPUT}/${BOOT_DISK_NAME}"
  readonly BOOT_DISK_GZ="${BOOT_DISK}.gz"
  local -r mounts_dir="${KBUILD_OUTPUT}/mnt"
  "${SCRIPT_DIR}"/build_boot_disk_helper.sh "${BOOT_DISK}" "${mounts_dir}" \
    "${TAR_INSTALL_DIR}" "${VMLINUZ_NAME}" "${INITRD_NAME}"
  pigz --keep --force "${BOOT_DISK}"
  ln -sf "$(basename "${BOOT_DISK}")" \
    "$(dirname "${BOOT_DISK}")/disk-latest.raw"
  ln -sf "$(basename "${BOOT_DISK_GZ}")" \
    "$(dirname "${BOOT_DISK_GZ}")/disk-latest.raw.gz"
  cat >"${KBUILD_OUTPUT}/disk-latest-pkgdef" <<EOL
pkgdef mpm = {
  package_name = 'test/chrome/cloudcast/kernel/gamelet_disk_test_${USER}'
  source = [
    {
      source_file = '${BOOT_DISK_NAME}'
      package_path = 'disk.raw'
    },
  ]
  relative_path_anchor = 'PKGDEF_DIR'
}
EOL
}

function build_nvidia_modules_disk() {
  readonly NVIDIA_DISK_NAME="nvidia-disk-${NVIDIA_DRIVER_VERSION}-${KERNELRELEASE}-${ARCH}.squashfs"
  readonly NVIDIA_DISK="${KBUILD_OUTPUT}/${NVIDIA_DISK_NAME}"

  local -r nvidiafs_assets="${SRC_DIR}/stadia/nvrootfs"
  local -r nvidiafs_install_dir="${KBUILD_OUTPUT}/nvidiafs-install"
  local -r nvidiafs_lib_dir="${nvidiafs_install_dir}/usr/lib/x86_64-linux-gnu"
  rm -rf "${NVIDIA_DISK}" "${nvidiafs_install_dir}"
  mkdir "${nvidiafs_install_dir}"
  mkdir "${nvidiafs_install_dir}/etc"
  mkdir -p "${nvidiafs_install_dir}/usr/lib/modules/${KERNELRELEASE}/extra/nvidia"
  mkdir -p "${nvidiafs_lib_dir}"

  # Copy manifest to /etc
  install -D -m "u=rw,go=r,a-s" "${NVIDIA_DRIVER_DIR}/nvidia_icd.json" \
  "${nvidiafs_install_dir}/etc/vulkan/icd.d/nvidia_icd.json"

  # Copy libraries to /usr/lib/x86_64-linux-gnu
  install -D "${NVIDIA_DRIVER_DIR}/lib"* "${nvidiafs_lib_dir}"

  # Create symlinks for all the libraries
  # - Always create a <name>.so link for each library
  # - For libraries that contain the NVIDIA_DRIVER_VERSION number
  # create a symlink that ends in <name>.so.1
  # - For the rest of the libraries with the pattern .so.<major>.<minor>.<patch>
  # create a symlink <name>.so.<major> if the `major` > 0.
  pushd ${nvidiafs_lib_dir}
    for lib in $(ls lib*); do
      local name=$(basename -- ${lib})
      local basename=${name%%.*}

      ln -s ${lib} ${basename}.so

      if [[ "$name" =~ .*${NVIDIA_DRIVER_VERSION}.* ]]; then
        ln -s ${lib} ${basename}.so.1
      else
        local version=$(echo ${name} | cut -d "." -f 3)
        if [[ version -gt 0 ]]; then
          ln -s ${lib} ${basename}.so.${version}
        fi
      fi
    done
  popd

  rsync -a \
    "${nvidiafs_assets}"/ \
    "${nvidiafs_install_dir}"/

  rsync -a \
    "${MOD_INSTALL_DIR}/usr/lib/modules/${KERNELRELEASE}/extra"/ \
    "${nvidiafs_install_dir}/usr/lib/modules/${KERNELRELEASE}/extra/nvidia"/

  # chown everything under the install dir to root:root, then refine as needed.
  # Do not follow symlinks (-h).
  chown -hR 0:0 "${nvidiafs_install_dir}"

  chmod 00755 "${nvidiafs_install_dir}"

  find "${nvidiafs_install_dir}" -type d -exec chmod 00755 {} \;
  mksquashfs "${nvidiafs_install_dir}" "${NVIDIA_DISK}" \
    -comp xz -no-exports -no-progress -no-recovery -Xbcj x86
}

function build_perf() {
  pushd "${SRC_DIR}"/tools/perf
  local -r perf_objs="${KBUILD_OUTPUT}"/tools/perf
  mkdir -p "${perf_objs}"
  readonly PERF_INSTALL_DIR="${KBUILD_OUTPUT}"/perf-install
  rm -rf "${PERF_INSTALL_DIR}"
  mkdir -p "${PERF_INSTALL_DIR}"
  make -f Makefile.perf -j "$(nproc)" install-bin \
    O="${perf_objs}" \
    DESTDIR="${PERF_INSTALL_DIR}" \
    prefix=/usr \
    lib=lib \
    VF=1 \
    NO_GTK2=1 \
    NO_LIBPERL=1 \
    NO_LIBZSTD=1 \
    NO_JVMTI=1 \
    WERROR=0 \
    "${MAKE_ARGS[@]}"
  popd
}

function build_perf_tar_xz() {
  readonly PERF_TAR_XZ="${KBUILD_OUTPUT}/perf-${KERNELRELEASE}-${ARCH}.tar.xz"
  # Getting the permissions wrong on /, /usr, or /boot is disastrous.
  # Kokoro sets the sgid bit on its root build folder, which must not make it
  # into the tar archive. Use GNU tar's --mode flag to forcefully remove them.
  # https://cs.corp.google.com/piper///depot/google3/devtools/kokoro/vanadium/linux_scripts/usr/local/bin/format_tmpfs.sh
  chmod 00755 "${PERF_INSTALL_DIR}" "${PERF_INSTALL_DIR}"/*
  tar -cf - --hard-dereference --owner=root --group=root --mode='u-s,g-s' \
    -C "${PERF_INSTALL_DIR}" . | xz --threads=0 > "${PERF_TAR_XZ}"
  # Print the content of the archive for the build log.
  tar -tvf "${PERF_TAR_XZ}"
  ln -sf "$(basename "${PERF_TAR_XZ}")" \
    "$(dirname "${PERF_TAR_XZ}")/perf-latest.tar.xz"
}

function stage_kernel_artifacts() {
  readonly artifacts_dir="${DOCKER_ARTIFACTS_DIR}"
  readonly mpm_dir="${artifacts_dir}/mpm"

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

function stage_nvidia_artifacts() {
    # See https://g3doc.corp.google.com/cloud/network/edge/g3doc/vm_disk.md#creating-an-mpm-disk-package
  readonly artifacts_dir="${DOCKER_ARTIFACTS_DIR}"
  readonly nvidia_disk_dir="${artifacts_dir}/disk"
  mkdir -p "${nvidia_disk_dir}"
  cp -a "${NVIDIA_DISK}" "${artifacts_dir}/"
  cp -a "${NVIDIA_DISK}" "${nvidia_disk_dir}/disk.raw"
}

function build_kernel_disk() {
  check_env
  set_LOCALVERSION_from_buildstamp
  download_initramfs_artifacts
  install_firmware
  create_kbuild_output
  create_initramfs_install_dir
  finalize_config
  build_bzimage_and_headers
  build_modules
  build_amdgpu_external_module
  build_perf
  build_kernel_rootfs
  build_initramfs
  build_linux_tar_xz
  build_perf_tar_xz
  build_boot_disk
  stage_kernel_artifacts
}

function build_nvidia_disk() {
  check_env
  set_LOCALVERSION_from_buildstamp
  create_kbuild_output
  finalize_config
  build_bzimage_and_headers
  build_modules
  build_nvidia_external_module
  build_nvidia_modules_disk
  stage_nvidia_artifacts
}

function build() {
  if [[ ! -z ${NVIDIA_DRIVER_VERSION} ]]; then
    echo "Building NVIDIA userspace modules and kernel modules disk"
    build_nvidia_disk
  else
    echo "Building kernel disk with amdgpu modules"
    build_kernel_disk
  fi
}

build
