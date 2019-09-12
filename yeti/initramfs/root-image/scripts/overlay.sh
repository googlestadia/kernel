#!/bin/sh
# Id: //yeti/scripts/overlay.sh#1

# Use overlayfs to stack a read-only root filesystem and squashfs filesystems
# (used to inject MPMs). The writable layer (aka the upperdir) is placed on the
# first writable block device with LABEL "OVERLAYRWFS", or on the /run tmpfs if
# there is no such block device.
mount_root_overlayfs() {
	# Create a container directory for the overlayfs. Under this container,
	# the boot fs is mounted as "boot", the root fs is mounted as "root",
	# each squashfs block device is mounted as "s<index>", the read-write
	# directory is mounted or created as "rw" (see above), and the workdir
	# is created as "work".
	local ofs_dir="/run/ofs"
	mkdir -p "${ofs_dir}"

	local boot_mnt="${ofs_dir}/boot"
	local root_mnt="${ofs_dir}/root"
	local krootfs_mnt="${ofs_dir}/krootfs"
	local rwfs_mnt="${ofs_dir}/rwfs"

	local squashfs_layer_index=0
	local upperdir="${ofs_dir}/rw"
	local workdir="${ofs_dir}/work"
	local lowerdir=""

	local bootfs_dev=""
	local bootfs_fstype=""
	local rootfs_dev=""
	local rootfs_devid=""
	local rwfs_dev=""
	local rwfs_fstype=""
	local ggprootfs_dev=""
	local ggprootfs_fstype=""

	# If root=UUID=<> or root=LABEL=<> was specified in cmdline, set
	# $rootfs_devid to just <> to resolve $rootfs_dev to an actual device
	# node.
	#
	# If root=auto was specified in cmdline, leave $rootfs_devid="" and
	# $rootfs_dev="". If $ggprootfs_dev is resolved, then $rootfs_dev will
	# be set to ggprootfs_dev. If $ggprootfs_dev is not resolved, there will
	# be no rootfs inserted in the overlayfs.
	#
	# Otherwise, if root=<dev>, set $rootfs_dev=<dev>.
	case "${cmdline_root}" in
		UUID=*)
			eval "${cmdline_root}"
			rootfs_devid="${UUID}"
			UUID=
			;;
		LABEL=*)
			eval "${cmdline_root}"
			rootfs_devid="${LABEL}"
			LABEL=
			;;
		auto)
			;;
		*)
			rootfs_dev="${cmdline_root}"
			;;
	esac

	# Iterate over `lsblk` lines to resolve boot, root, rw, and ggproot fs.
	# A sample line with lsblk -P:
	#    NAME="sda3" FSTYPE="ext2" LABEL="" UUID="" RO="0"
	while read -r line; do
		# Evaluate the line with the shell.
		eval "${line}"
		blk_dev="/dev/${NAME}"

		# Set $bootfs_dev if it is empty and the device LABEL is
		# "GGPBOOTFS". Also set $bootfs_fstype to FSTYTPE.
		if [ -z "${bootfs_dev}" ] && [ "GGPBOOTFS" = "${LABEL}" ]; then
			bootfs_dev="${blk_dev}"
			bootfs_fstype="${FSTYPE}"
		fi

		# Resolve $rootfs_dev if it is empty and if $UUID or $LABEL
		# match $rootfs_devid.
		if [ -z "${rootfs_dev}" ] &&  [ -n "${UUID}" ] && [ "${rootfs_devid}" = "${UUID}" ]; then
			rootfs_dev="${blk_dev}"
		fi
		if [ -z "${rootfs_dev}" ] &&  [ -n "${LABEL}" ] && [ "${rootfs_devid}" = "${LABEL}" ]; then
			rootfs_dev="${blk_dev}"
		fi

		# Set $rootfstype if it is blank and this is the root device.
		if [ "${blk_dev}" = "${rootfs_dev}" ] && [ -z "${rootfstype}" ]; then
			rootfstype="${FSTYPE}"
		fi

		# Set $ggprootfs_dev if it is empty and the device LABEL is
		# "GGPROOTFS". Also set $ggprootfs_fstype to FSTYTPE.
		if [ -z "${ggprootfs_dev}" ] && [ "GGPROOTFS" = "${LABEL}" ]; then
			ggprootfs_dev="${blk_dev}"
			ggprootfs_fstype="${FSTYPE}"
		fi

		# Set $rwfs_dev if it is empty and the device LABEL is
		# "OVERLAYRWFS" and RO == 0. Also set $rwfs_fstype to FSTYTPE.
		if [ -z "${rwfs_dev}" ] && [ "OVERLAYRWFS" = "${LABEL}" ] && [ "0" = "${RO}" ]; then
			rwfs_dev="${blk_dev}"
			rwfs_fstype="${FSTYPE}"
		fi
	done <<EOT
$(lsblk -P -o NAME,FSTYPE,LABEL,UUID,RO)
EOT

	# If $rootfs_dev is empty and $ggprootfs_dev is not, set
	# $rootfs_dev=$ggprootfs_dev. Do the same for rootfstype.
	if [ -z "${rootfs_dev}" ] && [ -n "${ggprootfs_dev}" ]; then
		rootfs_dev="${ggprootfs_dev}"
		rootfstype="${ggprootfs_fstype}"
	fi

	# Iterate over `lsblk` lines to build the overlay's squashfs lowerdir.
	# A sample line with lsblk -P:
	#    NAME="sda3" FSTYPE="ext2" LABEL="" UUID="" RO="0"
	while read -r line; do
		# Evaluate the line with the shell.
		eval "${line}"
		blk_dev="/dev/${NAME}"

		# If the block device is a squashfs, and it is not the root
		# device, mount it read-only and prepend its mount path to
		# `lowerdir`. overlayfs stacks layers from right to left
		# (leftmost is top).
		if [ "${FSTYPE}" = "squashfs" ] && [ "${blk_dev}" != "${rootfs_dev}" ]; then
			local squash_mnt="${ofs_dir}/s${squashfs_layer_index}"
			mkdir "${squash_mnt}"
			mount -t squashfs -r "${blk_dev}" "${squash_mnt}" \
				|| return 1
			squashfs_layer_index=$((${squashfs_layer_index} + 1))
			lowerdir="${squash_mnt}:${lowerdir}"
		fi
	done <<EOT
$(lsblk -P -o NAME,FSTYPE,LABEL,UUID,RO)
EOT

	# If a boot fs was found, mount it read-only and prepend its mount path
	# to `lowerdir`. overlayfs stacks layers from right to left (leftmost is
	# top).
	# NOTE: By design, stacked squashfs filesystems and the root fs (if any)
	# do not override or mask the contents of the boot fs.
	if [ -n "${bootfs_dev}" ]; then
		mkdir "${boot_mnt}"
		mount -t "${bootfs_fstype}" -r "${bootfs_dev}" "${boot_mnt}" \
			|| return 1
		lowerdir="${boot_mnt}:${lowerdir}"
	fi

	# If a kernel rootfs was bundled with the initramfs, mount it and
	# prepend its mount path to `lowerdir`. overlayfs stacks layers from
	# right to left (leftmost is top).
	# NOTE: By design, stacked squashfs filesystems and the root fs (if any)
	# do not override or mask the contents of the kernel rootfs.
	# NOTE: By design, the kernel rootfs is stacked "above" the boot fs.
	if [ -f "${krootfsdev}" ]; then
		mkdir "${krootfs_mnt}"
		mount -t squashfs -r "${krootfsdev}" "${krootfs_mnt}" \
			|| return 1
		lowerdir="${krootfs_mnt}:${lowerdir}"
	fi

	# If a root fs was specified or found, mount it read-only and append its
	# mount path to `lowerdir`. overlayfs stacks layers from right to left
	# (leftmost is top). If `lowerdir` is not empty, it will contain a
	# suffix ':'.
	if [ -n "${rootfs_dev}" ]; then
		mkdir "${root_mnt}"
		mount -t "${rootfstype}" "${rootro}" -o "${rootflags}" \
			"${rootfs_dev}" "${root_mnt}" || return 1
		lowerdir="${lowerdir}${root_mnt}:"
	fi

	# Finalize `lowerdir` by stripping any trailing ':'.
	lowerdir=${lowerdir%:}

	# If a overlayfs read-write filesystem was found, mount it and place the
	# overlayfs upperdir and workdir on it.
	if [ -n "${rwfs_dev}" ]; then
		mkdir "${rwfs_mnt}"
		mount -t "${rwfs_fstype}" "${rwfs_dev}" "${rwfs_mnt}" \
			|| return 1
		upperdir="${rwfs_mnt}/rw"
		workdir="${rwfs_mnt}/work"
	fi

	# Mount the overlayfs.
	mkdir "${upperdir}" "${workdir}"
	mount -t overlay overlay -o \
		"lowerdir=${lowerdir},upperdir=${upperdir},workdir=${workdir}" \
		/root || return 1


	# WORKAROUND: Create /mnt/package in the root overlayfs.
	# TODO(b/139151730): ESC asset mounting broken
	mkdir -p /root/mnt/package
}
