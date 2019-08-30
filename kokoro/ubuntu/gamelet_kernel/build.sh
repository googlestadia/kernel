#!/bin/bash
set -xe

readonly SCRIPT_DIR=$(dirname "$(readlink -f "${0}")")
readonly SRC_DIR=$(readlink -f "${SCRIPT_DIR}/../../..")
readonly GCLOUD_KEY_FILE="${KOKORO_KEYSTORE_DIR}/71274_kokoro_service_key_json"
readonly INITRAMFS_BIN_URL="gs://yeti_kernels/initramfs/initramfs-20190312.tar.gz"
readonly INITRAMFS_BIN_SHA256="9790f1a9a859eca95c8ca036416f0ca7d76fdd0bc41c273fcad4636caf4681d3"
readonly AMDGPU_FIRMWARE_URL="gs://yeti_kernels/amdgpu-firmware/amdgpu-firmware-2019.3.tar.gz"
readonly AMDGPU_FIRMWARE_SHA256="cc904ee1a9c89c2b0f80800dc43f26cf92b5fb8afc354514e85593026e12072a"

# Downloads a file from GCS and verifies its authenticity using a checksum.
function download_gcs() {
  local -r url=$1
  local dst_path=$2
  local -r sha256sum=$3

  if ! hash gsutil 2>/dev/null; then
    return 1
  fi

  if [[ -d "${dst_path}" ]]; then
    dst_path="${dst_path%/}/$(basename "${url}")"
  fi

  if [[ -f "${dst_path}" ]]; then
    local -r remote_hash="$(gsutil hash -h "${url}" \
      | grep -oP '\s+Hash \(md5\):\s+\K\w+')"
    local -r local_hash="$(md5sum "${dst_path}" | cut -d' ' -f1)"
  else
    local -r remote_hash="0"
    local -r local_hash="1"
  fi

  if [[ "${local_hash}" != "${remote_hash}" ]]; then
    gsutil -m -q cp -r "${url}" "${dst_path}"
    if [[ $? -gt 0 ]]; then
      rm -f "${dst_path}"
      return 1
    fi
  fi

  if [[ -n ${sha256sum} ]]; then
    echo "${sha256sum}  ${dst_path}" | sha256sum --quiet -c
    if [[ $? -ne 0 ]]; then
      rm -f "${dst_path}"
      if [[ ${local_hash} == "${remote_hash}" ]]; then
        download_gcs "${url}" "${dst_path}" "${sha256sum}"
      else
        return 1
      fi
    fi
  fi

  echo "${dst_path}"
}

function check_kokoro_env() {
  if [[ ! -d ${KOKORO_ROOT} ]]; then
    echo "KOKORO_ROOT is not a valid directory path."
    exit 1
  fi
  export KOKORO_ROOT="${KOKORO_ROOT%/}"
}

function set_LOCALVERSION_from_buildstamp() {
  pushd "${SRC_DIR}"
  local -r buildstamp_vars_script="${KOKORO_ROOT}"/builstamp.sh
  "${SCRIPT_DIR}"/get_workspace_status > "${KOKORO_ROOT}"/workspace-status.txt
  python3 "${SCRIPT_DIR}"/gen_buildstamp_vars.py \
    --output "${buildstamp_vars_script}" \
    --template "${SCRIPT_DIR}"/buildstamp_sh_template.txt \
    --var_format "export {}=\"{}\"" \
    "${KOKORO_ROOT}"/workspace-status.txt
  source "${buildstamp_vars_script}"
  if [[ -z ${STABLE_KOKORO_JOB_NAME} ]]; then
    readonly LOCALVERSION="${LOCALVERSION}-local"
  fi
  popd
}

function setup_gcloud() {
  if [[ -e "${GCLOUD_KEY_FILE}" ]]; then
    gcloud auth activate-service-account --key-file "${GCLOUD_KEY_FILE}"
  fi
}

function download_initramfs_artifacts() {
  readonly TMP_INITRAMFS_DIR="${KOKORO_ROOT}/initramfs"
  rm -rf "${TMP_INITRAMFS_DIR}"
  mkdir "${TMP_INITRAMFS_DIR}"

  local -r initramfs_bin_archive="$(download_gcs "${INITRAMFS_BIN_URL}" "${KOKORO_ROOT}" "${INITRAMFS_BIN_SHA256}")"
  tar -xf "${initramfs_bin_archive}" -C "${TMP_INITRAMFS_DIR}/"
}

function download_firmware() {
  readonly FIRMWARE_INSTALL_DIR="${KOKORO_ROOT}/firmware-install"
  mkdir -p "${FIRMWARE_INSTALL_DIR}"

  local -r firmware_install_usr_dir="${FIRMWARE_INSTALL_DIR}/usr/lib/firmware"
  mkdir -p "${firmware_install_usr_dir}"

  local -r amdgpu_firmware_dir="${firmware_install_usr_dir}/amdgpu"
  mkdir -p "${amdgpu_firmware_dir}"
  local -r amdgpu_firmware_archive="$(download_gcs "${AMDGPU_FIRMWARE_URL}" "${KOKORO_ROOT}" "${AMDGPU_FIRMWARE_SHA256}")"
  tar -xf "${amdgpu_firmware_archive}" -C "${amdgpu_firmware_dir}"/
}

function create_kbuild_output() {
  export KBUILD_OUTPUT="${KOKORO_ROOT}/obj"
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
  readonly SRC_CONFIG_FILE=${SCRIPT_DIR}/kernel_configs/${CONFIG_NAME:-release}
  if [[ ! -e "${SRC_CONFIG_FILE}" ]]; then
    echo "There is no config file named ${CONFIG_NAME}."
    exit 1
  fi
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

function build_dkms_modules() {
  pushd "${SRC_DIR}"
  local -r dkms_kbuild="${KBUILD_OUTPUT}"/amdgpu-dkms
  rm -rf "${dkms_kbuild}"
  mkdir "${dkms_kbuild}" "${dkms_kbuild}"/include
  rsync -a \
    dkms/drivers/gpu/drm/amd \
    dkms/drivers/gpu/drm/radeon \
    dkms/drivers/gpu/drm/scheduler \
    dkms/drivers/gpu/drm/ttm \
    dkms/drivers/gpu/drm/amd/dkms/ \
    "${dkms_kbuild}"/
  rsync -a \
    dkms/include/kcl \
    "${dkms_kbuild}"/include/
  mkdir -p "${dkms_kbuild}"/include/drm/
  rsync -a \
    dkms/include/drm/amd_rdma.h \
    dkms/include/drm/gpu_scheduler.h \
    dkms/include/drm/spsc_queue.h \
    dkms/include/drm/ttm \
    "${dkms_kbuild}"/include/drm/
  mkdir -p "${dkms_kbuild}"/include/uapi/drm/
  rsync -a \
    dkms/include/uapi/drm/amdgpu_drm.h \
    "${dkms_kbuild}"/include/uapi/drm/
  mkdir -p "${dkms_kbuild}"/include/uapi/linux
  rsync -a \
    dkms/include/uapi/linux/kfd_ioctl.h \
    "${dkms_kbuild}"/include/uapi/linux
  popd
  pushd "${dkms_kbuild}"
  patch -p1 < "${SCRIPT_DIR}"/dkms.patch
  ./pre-build.sh 4.19
  make -j "$(nproc)" -C "${KBUILD_OUTPUT}" \
    M="${dkms_kbuild}" "${MAKE_ARGS[@]}"
  local -r mod_install_usr_dir="${MOD_INSTALL_DIR}/usr"
  make -j "$(nproc)" -C "${KBUILD_OUTPUT}" \
    M="${dkms_kbuild}" \
    modules_install "${MAKE_ARGS[@]}" \
    INSTALL_MOD_PATH="${mod_install_usr_dir}" \
    INSTALL_MOD_STRIP=1
  popd
}

function build_kernel_rootfs() {
  # LINT.IfChange
  readonly KROOTFS="${INITRAMFS_INSTALL_DIR}/krootfs.squashfs"
  # LINT.ThenChange(
  #     yeti/initramfs/root-image/init,
  #     yeti/initramfs/root-image/scripts/overlay.sh,
  # )
  readonly KROOTFS_ASSETS="${SRC_DIR}/yeti/krootfs"
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
  readonly INITRAMFS_DATA_CPIO_XZ="${KBUILD_OUTPUT}/initramfs_data.cpio.xz"
  "${SRC_DIR}"/usr/gen_initramfs_list.sh \
    -o "${INITRAMFS_DATA_CPIO_XZ}" \
    -u -1 -g -1 \
    "${SRC_DIR}"/yeti/initramfs/root-files \
    "${SRC_DIR}"/yeti/initramfs/root-image \
    "${INITRAMFS_INSTALL_DIR}" \
    "${TMP_INITRAMFS_DIR}"
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
  tar -cf - --owner=root --group=root --mode='u-s,g-s' \
    -C "${TAR_INSTALL_DIR}" . | xz --threads=0 > "${LINUX_TAR_XZ}"
  # Print the content of the archive for the build log.
  tar -tvf "${LINUX_TAR_XZ}"
  ln -sf "$(basename "${LINUX_TAR_XZ}")" \
    "$(dirname "${LINUX_TAR_XZ}")/linux-latest.tar.xz"
  popd
}

function build_boot_disk() {
  readonly BOOT_DISK="${KBUILD_OUTPUT}/disk-${KERNELRELEASE}-${ARCH}.raw"
  readonly BOOT_DISK_GZ="${BOOT_DISK}.gz"
  local -r boot_disk_mount_dir="${KBUILD_OUTPUT}/boot-mount"
  rm -f "${BOOT_DISK}"
  touch "${BOOT_DISK}"
  fallocate --zero-range --length 35M "${BOOT_DISK}"
  /sbin/parted --script "${BOOT_DISK}" -- mklabel msdos mkpart primary ext2 \
    2048s -1s set 1 boot on
  /sbin/mkfs.ext2 -F -L GGPBOOTFS -Eoffset=1048576 "${BOOT_DISK}"
  sudo "${SCRIPT_DIR}"/build_boot_disk_helper.sh "${BOOT_DISK}" \
    "${boot_disk_mount_dir}" "${TAR_INSTALL_DIR}" "${VMLINUZ_NAME}" \
    "${INITRD_NAME}"
  dd if=/usr/lib/syslinux/mbr/mbr.bin of="${BOOT_DISK}" conv=notrunc
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
      source_file = '${BOOT_DISK}'
      package_path = 'disk.raw'
    },
  ]
}
EOL
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
    NO_JVMTI=1 \
    LIBCLANGLLVM=1 \
    LLVM_CONFIG=/usr/bin/llvm-config-3.9 \
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
  tar -cf - --owner=root --group=root --mode='u-s,g-s' \
    -C "${PERF_INSTALL_DIR}" . | xz --threads=0 > "${PERF_TAR_XZ}"
  # Print the content of the archive for the build log.
  tar -tvf "${PERF_TAR_XZ}"
  ln -sf "$(basename "${PERF_TAR_XZ}")" \
    "$(dirname "${PERF_TAR_XZ}")/perf-latest.tar.xz"
}

function build() {
  check_kokoro_env
  set_LOCALVERSION_from_buildstamp
  setup_gcloud
  download_initramfs_artifacts
  download_firmware
  create_kbuild_output
  create_initramfs_install_dir
  finalize_config
  build_bzimage_and_headers
  build_modules
  build_dkms_modules
  build_perf
  build_kernel_rootfs
  build_initramfs
  build_linux_tar_xz
  build_perf_tar_xz
  build_boot_disk
}

build
