#!/bin/sh
# Copyright (C) 2014 OpenWrt.org
#

. /lib/functions.sh

# 'kernel' partition on NAND contains the kernel
CI_KERNPART="kernel"

# 'ubi' partition on NAND contains UBI
CI_UBIPART="ubi"

ubi_mknod() {
	local dir="$1"
	local dev="/dev/$(basename $dir)"

	[ -e "$dev" ] && return 0

	local devid="$(cat $dir/dev)"
	local major="${devid%%:*}"
	local minor="${devid##*:}"
	mknod "$dev" c $major $minor
}

nand_find_volume() {
	local ubidevdir ubivoldir
	ubidevdir="/sys/devices/virtual/ubi/$1"
	[ ! -d "$ubidevdir" ] && return 1
	for ubivoldir in $ubidevdir/${1}_*; do
		[ ! -d "$ubivoldir" ] && continue
		if [ "$( cat $ubivoldir/name )" = "$2" ]; then
			basename $ubivoldir
			ubi_mknod "$ubivoldir"
			return 0
		fi
	done
}

nand_find_ubi() {
	local ubidevdir ubidev mtdnum
	mtdnum="$( find_mtd_index $1 )"
	[ ! "$mtdnum" ] && return 1
	for ubidevdir in /sys/devices/virtual/ubi/ubi*; do
		[ ! -d "$ubidevdir" ] && continue
		cmtdnum="$( cat $ubidevdir/mtd_num )"
		[ ! "$mtdnum" ] && continue
		if [ "$mtdnum" = "$cmtdnum" ]; then
			ubidev=$( basename $ubidevdir )
			ubi_mknod "$ubidevdir"
			echo $ubidev
			return 0
		fi
	done
}

nand_get_magic_long() {
	dd if="$1" skip=$2 bs=4 count=1 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

get_magic_long_tar() {
	( tar xf $1 $2 -O | dd bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2> /dev/null
}

identify_magic() {
	local magic=$1
	case "$magic" in
		"55424923")
			echo "ubi"
			;;
		"31181006")
			echo "ubifs"
			;;
		"68737173")
			echo "squashfs"
			;;
		"d00dfeed")
			echo "fit"
			;;
		"4349"*)
			echo "combined"
			;;
		*)
			echo "unknown $magic"
			;;
	esac
}


identify() {
	identify_magic $(nand_get_magic_long "$1" "${2:-0}")
}

identify_tar() {
	identify_magic $(get_magic_long_tar "$1" "$2")
}

nand_restore_config() {
	sync
	local ubidev=$( nand_find_ubi $CI_UBIPART )
	local ubivol="$( nand_find_volume $ubidev rootfs_data )"
	[ ! "$ubivol" ] &&
		ubivol="$( nand_find_volume $ubidev rootfs )"
	mkdir /tmp/new_root
	if ! mount -t ubifs /dev/$ubivol /tmp/new_root; then
		echo "mounting ubifs $ubivol failed"
		rmdir /tmp/new_root
		return 1
	fi
	mv "$1" "/tmp/new_root/sysupgrade.tgz"
	umount /tmp/new_root
	sync
	rmdir /tmp/new_root
}

nand_upgrade_prepare_ubi() {
	local rootfs_length="$1"
	local rootfs_type="$2"
	local has_kernel="${3:-0}"
	local has_env="${4:-0}"

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition $CI_UBIPART"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi

	if [ ! "$ubidev" ]; then
		ubiformat /dev/mtd$mtdnum -y
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
		[ "$has_env" -gt 0 ] && {
			ubimkvol /dev/$ubidev -n 0 -N ubootenv -s 1MiB
			ubimkvol /dev/$ubidev -n 1 -N ubootenv2 -s 1MiB
		}
	fi

	local kern_ubivol="$( nand_find_volume $ubidev kernel )"
	local root_ubivol="$( nand_find_volume $ubidev rootfs )"
	local data_ubivol="$( nand_find_volume $ubidev rootfs_data )"

	# remove ubiblock device of rootfs
	local root_ubiblk="ubiblock${root_ubivol:3}"
	local try removed=0 name vol
	if [ "$root_ubivol" -a -e "/dev/$root_ubiblk" ]; then
		echo "removing $root_ubiblk"
		# The last reference to the old root is dropped asynchronously,
		# so keep trying while the kernel releases it.  Do not hide the
		# error from ubiblock: a busy device is the only clue that a
		# process is still holding the old root.
		for try in 1 2 3 4 5 6 7 8 9 10; do
			umount "/dev/$root_ubiblk" 2>/dev/null
			if ubiblock -r "/dev/$root_ubivol"; then
				removed=1
				break
			fi
			[ "$try" -lt 10 ] && sleep 1
		done
		[ "$removed" = 1 ] || {
			echo "cannot remove $root_ubiblk"
			return 1
		}
	fi

	# kill volumes: a busy volume must abort the upgrade here, before any
	# image data is written, instead of being silently ignored.
	for name in kernel rootfs rootfs_data; do
		case "$name" in
			kernel) vol="$kern_ubivol" ;;
			rootfs) vol="$root_ubivol" ;;
			rootfs_data) vol="$data_ubivol" ;;
		esac
		[ -n "$vol" ] || continue
		ubirmvol /dev/$ubidev -N "$name" || {
			echo "cannot remove $name volume"
			return 1
		}
	done

	# update kernel
	if [ "$has_kernel" = "1" ]; then
		if ! ubimkvol /dev/$ubidev -N kernel -s $kernel_length; then
			echo "cannot create kernel volume"
			return 1;
		fi
	fi

	# update rootfs
	local root_size_param
	if [ "$rootfs_type" = "ubifs" ]; then
		root_size_param="-m"
	else
		root_size_param="-s $rootfs_length"
	fi
	if ! ubimkvol /dev/$ubidev -N rootfs $root_size_param; then
		echo "cannot create rootfs volume"
		return 1;
	fi

	# create rootfs_data for non-ubifs rootfs
	if [ "$rootfs_type" != "ubifs" ]; then
		if ! ubimkvol /dev/$ubidev -N rootfs_data -m; then
			echo "cannot initialize rootfs_data volume"
			return 1
		fi
	fi
	sync
	return 0
}

nand_do_upgrade_success() {
	local conf_tar="/tmp/sysupgrade.tgz"
	
	sync
	[ -f "$conf_tar" ] && nand_restore_config "$conf_tar"
	echo "sysupgrade successful"
	umount -a
	reboot -f
}

# Flash the UBI image to MTD partition
nand_upgrade_ubinized() {
	local ubi_file="$1"
	local mtdnum="$(find_mtd_index "$CI_UBIPART")"

	[ ! "$mtdnum" ] && {
		CI_UBIPART="rootfs"
		mtdnum="$(find_mtd_index "$CI_UBIPART")"
	}

	if [ ! "$mtdnum" ]; then
		echo "cannot find mtd device $CI_UBIPART"
		umount -a
		reboot -f
	fi

	local mtddev="/dev/mtd${mtdnum}"
	ubidetach -p "${mtddev}" || true
	sync
	ubiformat "${mtddev}" -y -f "${ubi_file}" || {
		echo "cannot write ubi image, upgrade aborted"
		return 1
	}
	ubiattach -p "${mtddev}" || {
		echo "cannot reattach ubi device"
		return 1
	}
	nand_do_upgrade_success
}

# Write the UBIFS image to UBI volume
nand_upgrade_ubifs() {
	local rootfs_length=`(cat $1 | wc -c) 2> /dev/null`

	if ! nand_upgrade_prepare_ubi "$rootfs_length" "ubifs" "0" "0"; then
		echo "cannot prepare ubi volumes, aborting upgrade"
		return 1
	fi

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	local root_ubivol="$(nand_find_volume $ubidev rootfs)"
	ubiupdatevol /dev/$root_ubivol -s $rootfs_length $1 || {
		echo "cannot write rootfs volume, upgrade aborted"
		return 1
	}

	nand_do_upgrade_success
}

nand_board_name() {
	if type 'platform_nand_board_name' >/dev/null 2>/dev/null; then
		platform_nand_board_name
		return
	fi

	cat /tmp/sysinfo/board_name
}

nand_upgrade_tar() {
	local tar_file="$1"
	local board_name="$(nand_board_name)"
	local kernel_mtd="$(find_mtd_index $CI_KERNPART)"

	local kernel_length=`(tar xf $tar_file sysupgrade-$board_name/kernel -O | wc -c) 2> /dev/null`
	local rootfs_length=`(tar xf $tar_file sysupgrade-$board_name/root -O | wc -c) 2> /dev/null`

	local rootfs_type="$(identify_tar "$tar_file" sysupgrade-$board_name/root)"

	local has_kernel=1
	local has_env=0

	[ "$kernel_length" = 0 -o ! -z "$kernel_mtd" ] && has_kernel=0

	# Release the ubi volumes and create the new ones before anything is
	# written: when the volumes are still busy this aborts here, instead
	# of leaving the box with a new kernel and the old rootfs.
	if ! nand_upgrade_prepare_ubi "$rootfs_length" "$rootfs_type" "$has_kernel" "$has_env"; then
		echo "cannot prepare ubi volumes, aborting upgrade"
		return 1
	fi

	[ "$kernel_length" != 0 -a -n "$kernel_mtd" ] && {
		tar xf $tar_file sysupgrade-$board_name/kernel -O | mtd write - $CI_KERNPART || {
			echo "cannot write kernel partition"
			return 1
		}
	}

	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	[ "$has_kernel" = "1" ] && {
		local kern_ubivol="$(nand_find_volume $ubidev kernel)"
	 	tar xf $tar_file sysupgrade-$board_name/kernel -O | \
			ubiupdatevol /dev/$kern_ubivol -s $kernel_length - || {
			echo "cannot write kernel volume"
			return 1
		}
	}

	local root_ubivol="$(nand_find_volume $ubidev rootfs)"
	tar xf $tar_file sysupgrade-$board_name/root -O | \
		ubiupdatevol /dev/$root_ubivol -s $rootfs_length - || {
		echo "cannot write rootfs volume, upgrade aborted"
		return 1
	}

	nand_do_upgrade_success
}

# Recognize type of passed file and start the upgrade process
nand_do_upgrade_stage2() {
	local file_type=$(identify $1)

	if type 'platform_nand_pre_upgrade' >/dev/null 2>/dev/null; then
		platform_nand_pre_upgrade "$1"
	fi

	[ ! "$(find_mtd_index "$CI_UBIPART")" ] && CI_UBIPART="rootfs"

	case "$file_type" in
		"ubi")		nand_upgrade_ubinized "$1" || return 1;;
		"ubifs")	nand_upgrade_ubifs "$1" || return 1;;
		*)		nand_upgrade_tar "$1" || return 1;;
	esac
}

# Number of user space processes which are still alive, excluding this shell
# and PID 1 (the upgraded binary).
nand_processes_left() {
	local pid n=0

	for pid in /proc/[0-9]*; do
		pid="${pid#/proc/}"
		[ "$pid" = "$$" ] && continue
		[ "$pid" = "1" ] && continue
		[ -n "$(readlink /proc/$pid/exe 2>/dev/null)" ] || continue
		n=$((n + 1))
	done

	echo $n
}

# Ask the remaining processes to exit and give them time to finish their I/O
# before escalating to SIGKILL.  A daemon which is writing to the USB disks or
# to the overlay and is killed mid-flight can end up in uninterruptible I/O:
# it survives SIGKILL, keeps its files open and therefore keeps the old root -
# and with it ubiblock0_0 and the ubi volumes - busy.  That is exactly the
# "cannot remove block device (Resource busy)" failure.
nand_stop_processes() {
	local i

	kill_remaining TERM

	i=0
	while [ "$i" -lt 15 ]; do
		[ "$(nand_processes_left)" -eq 0 ] && break
		sleep 1
		i=$((i + 1))
	done

	[ "$(nand_processes_left)" -gt 0 ] && {
		echo "processes still running, sending SIGKILL"
		kill_remaining KILL

		i=0
		while [ "$i" -lt 5 ]; do
			[ "$(nand_processes_left)" -eq 0 ] && break
			sleep 1
			i=$((i + 1))
		done
	}

	[ "$(nand_processes_left)" -gt 0 ] &&
		echo "warning: $(nand_processes_left) processes did not exit"

	sync
}

nand_upgrade_stage2() {
	[ $1 = "nand" ] && {
		[ -f "$2" ] && {
			touch /tmp/sysupgrade

			nand_stop_processes

			if [ -n "$(rootfs_type)" ]; then
				v "Switching to ramdisk..."
				# run_ramfs() ends in exec and never returns.  Failures of
				# nand_do_upgrade_stage2() are reported by the ramfs shell;
				# its exit status makes upgraded reboot the box.
				run_ramfs --release-old-root ". /lib/functions.sh; include /lib/upgrade; nand_do_upgrade_stage2 $2"
			else
				nand_do_upgrade_stage2 "$2" || {
					echo "Nand upgrade failed"
					exit 1
				}
			fi
			return 0
		}
		echo "Nand upgrade failed"
		exit 1
	}
}

nand_upgrade_stage1() {
	[ -f /tmp/sysupgrade-nand-path ] && {
		path="$(cat /tmp/sysupgrade-nand-path)"
		[ "$SAVE_CONFIG" != 1 -a -f "$CONF_TAR" ] &&
			rm $CONF_TAR

		ubus call system nandupgrade "{\"path\": \"$path\" }"
		exit 0
	}
}

# Check if passed file is a valid one for NAND sysupgrade. Currently it accepts
# 3 types of files:
# 1) UBI - should contain an ubinized image, header is checked for the proper
#    MAGIC
# 2) UBIFS - should contain UBIFS partition that will replace "rootfs" volume,
#    header is checked for the proper MAGIC
# 3) TAR - archive has to include "sysupgrade-BOARD" directory with a non-empty
#    "CONTROL" file (at this point its content isn't verified)
#
# You usually want to call this function in platform_check_image.
#
# $(1): board name, used in case of passing TAR file
# $(2): file to be checked
nand_do_platform_check() {
	local board_name="$1"
	local tar_file="$2"
	local control_length=`(tar xf $tar_file sysupgrade-$board_name/CONTROL -O | wc -c) 2> /dev/null`
	local file_type="$(identify $2)"

	[ "$control_length" = 0 -a "$file_type" != "ubi" -a "$file_type" != "ubifs" ] && {
		echo "Invalid sysupgrade file."
		return 1
	}

	return 0
}

# Start NAND upgrade process
#
# $(1): file to be used for upgrade
nand_do_upgrade() {
	echo -n $1 > /tmp/sysupgrade-nand-path
	cp /sbin/upgraded /tmp/
	nand_upgrade_stage1
}
