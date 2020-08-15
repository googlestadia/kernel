#!/bin/sh
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

init=/lib/systemd/systemd
cmdline_root=auto
rootflags=
rootfstype=
rootro=-r
krootfsdev=/krootfs.squashfs

# Mount sysfs, procfs, devtmpfs, devpts, and tmpfs.
[ -d /sys ] || mkdir /sys
mount -t sysfs -o nodev,noexec,nosuid sysfs /sys

[ -d /proc ] || mkdir /proc
mount -t proc -o nodev,noexec,nosuid proc /proc

[ -d /dev ] || mkdir -m 0755 /dev
mount -t devtmpfs -o nosuid,mode=0755 udev /dev

mkdir /dev/pts
mount -t devpts -o noexec,nosuid,gid=5,mode=0620 devpts /dev/pts

# The primary tmpfs should be 5 GB. It is assumed that the root overlayfs will
# use this tmpfs for its writable layer (see scripts/overlay.sh).
# b/109826171
[ -d /run ] || mkdir -m 0755 /run
mount -t tmpfs -o nodev,mode=755,size=5G tmpfs /run

# Parse the command line for a few well-known options affecting how the root
# filesystem is mounted and what init to chain to.
for x in $(cat /proc/cmdline); do
	case $x in
		init=*)
			init="${x#init=}"
			;;
		root=*)
			cmdline_root="${x#root=}"
			;;
		rootflags=*)
			rootflags="${x#rootflags=}"
			;;
		rootfstype=*)
			rootfstype="${x#rootfstype=}"
			;;
		rw)
			rootro="-w"
			;;
	esac
done

rescue_shell() {
	echo "Something went wrong. Dropping to a shell."
	exec sh
}

# Mount the root overlayfs.
. /scripts/overlay.sh || rescue_shell
mount_root_overlayfs || rescue_shell

# Move filesystems to the final root fs.

[ -d /root/sys ] || mkdir /root/sys
mount -n -o move /sys /root/sys || rescue_shell
[ -d /root/proc ] || mkdir /root/proc
mount -n -o move /proc /root/proc || rescue_shell
[ -d /root/dev ] || mkdir -m 0755 /root/dev
mount -n -o move /dev /root/dev || rescue_shell
[ -d /root/run ] || mkdir -m 0755 /root/run
mount -n -o move /run /root/run || rescue_shell

# Keep /dev available (for /dev/console, below).
rm -rf /dev
ln -s /root/dev /dev || rescue_shell

# Chain to the final root fs.
exec switch_root -c /dev/console /root "${init}" || rescue_shell
