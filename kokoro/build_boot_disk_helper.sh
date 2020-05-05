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
readonly BOOT_DISK_MOUNT_DIR="$2"
readonly TAR_INSTALL_DIR="$3"
readonly VMLINUZ_NAME="$4"
readonly INITRD_NAME="$5"

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

umount "${BOOT_DISK_MOUNT_DIR}" || true
mkdir -p "${BOOT_DISK_MOUNT_DIR}"
mount -o loop,offset=1048576 "${BOOT_DISK}" "${BOOT_DISK_MOUNT_DIR}"
rm -rf "${BOOT_DISK_MOUNT_DIR}"/lost+found/
rsync -a "${TAR_INSTALL_DIR}"/ "${BOOT_DISK_MOUNT_DIR}"/
mkdir -p "${BOOT_DISK_MOUNT_DIR}"/boot/syslinux
extlinux --install "${BOOT_DISK_MOUNT_DIR}"/boot/syslinux
echo "default stadia
timeout 0
label stadia
kernel /boot/${VMLINUZ_NAME}
append initrd=/boot/${INITRD_NAME} root=auto ro console=ttyS0
" > "${BOOT_DISK_MOUNT_DIR}"/boot/syslinux/syslinux.cfg
chown -R root:root "${BOOT_DISK_MOUNT_DIR}" || true
umount "${BOOT_DISK_MOUNT_DIR}"
