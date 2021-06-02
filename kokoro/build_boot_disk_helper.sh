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

readonly BOOT_DISK="$1"
readonly MOUNTS_DIR="$2"
readonly TAR_INSTALL_DIR="$3"
readonly VMLINUZ_NAME="$4"
readonly INITRD_NAME="$5"

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

rm -f "${BOOT_DISK}"
touch "${BOOT_DISK}"
fallocate --zero-range --length 36M "${BOOT_DISK}"

sgdisk \
  --clear \
  --new=1:: --typecode=1:EF00 --change-name=1:GGPBOOTFS \
  --attributes=1:set:2 \
  "${BOOT_DISK}"

kpartx -va "${BOOT_DISK}"
dmsetup --noudevsync mknodes
readonly LO_NODE="$(losetup --associated "${BOOT_DISK}" | cut -d: -f1)"
readonly LO_DEV="$(basename "${LO_NODE}p1")"
readonly DM_NODE="/dev/mapper/${LO_DEV}"

cleanup() {
  umount "${DM_NODE}" || true
  kpartx -vd "${BOOT_DISK}" || true
  dmsetup remove "${LO_DEV}" || true
  losetup -d "${LO_NODE}" || true
}
trap 'cleanup' ERR

/sbin/mkfs.vfat -s 1 -F 32 -n GGPBOOTFS "${DM_NODE}"
readonly EFI_MOUNT_DIR="${MOUNTS_DIR}"/efi
mkdir -p "${EFI_MOUNT_DIR}"
mount "${DM_NODE}" "${EFI_MOUNT_DIR}"

rsync -a "${TAR_INSTALL_DIR}"/boot/ "${EFI_MOUNT_DIR}"/

mkdir -p "${EFI_MOUNT_DIR}"/syslinux
echo "SERIAL 0
CONSOLE 0
DEFAULT stadia
TIMEOUT 0
LABEL stadia
  KERNEL /${VMLINUZ_NAME}
  APPEND initrd=/${INITRD_NAME} root=auto ro console=ttyS0
" > "${EFI_MOUNT_DIR}"/syslinux/syslinux.cfg
extlinux --install "${EFI_MOUNT_DIR}"/syslinux

mkdir -p "${EFI_MOUNT_DIR}"/EFI/BOOT
cp /usr/lib/SYSLINUX.EFI/efi64/syslinux.efi \
  "${EFI_MOUNT_DIR}"/EFI/BOOT/BOOTX64.EFI
cp /usr/lib/syslinux/modules/efi64/ldlinux.e64 \
  "${EFI_MOUNT_DIR}"/syslinux/syslinux.cfg \
  "${EFI_MOUNT_DIR}"/EFI/BOOT/

# Copy the "boot sector" to its backup for fsck.fat, after unmounting.
umount "${DM_NODE}"
dd if="${DM_NODE}" of="${DM_NODE}" bs=512 count=1 seek=6

# Install syslinux's GPT bootloader code in the image's protective MBR.
dd if=/usr/lib/syslinux/mbr/gptmbr.bin of="${LO_NODE}"

cleanup
