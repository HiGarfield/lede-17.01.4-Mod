#!/bin/sh

RAM_ROOT=/tmp/root

[ -x /usr/bin/ldd ] || ldd() { LD_TRACE_LOADED_OBJECTS=1 $*; }
libs() { ldd $* 2>/dev/null | sed -r 's/(.* => )?(.*) .*/\2/'; }

install_file() { # <file> [ <file> ... ]
	for file in "$@"; do
		dest="$RAM_ROOT/$file"
		[ -f $file -a ! -f $dest ] && {
			dir="$(dirname $dest)"
			mkdir -p "$dir"
			cp $file $dest
		}
	done
}

install_bin() { # <file> [ <symlink> ... ]
	src=$1
	files=$1
	[ -x "$src" ] && files="$src $(libs $src)"
	install_file $files
	shift
	for link in "$@"; do {
		dest="$RAM_ROOT/$link"
		dir="$(dirname $dest)"
		mkdir -p "$dir"
		[ -f "$dest" ] || ln -s $src $dest
	}; done
}

supivot() { # <new_root> <old_root>
	/bin/mount | grep "on $1 type" 2>&- 1>&- || /bin/mount -o bind $1 $1
	mkdir -p $1$2 $1/proc $1/sys $1/dev $1/tmp $1/overlay && \
	/bin/mount -o noatime,move /proc $1/proc && \
	pivot_root $1 $1$2 || {
		/bin/umount -l $1 $1
		return 1
	}

	/bin/mount -o noatime,move $2/sys /sys
	/bin/mount -o noatime,move $2/dev /dev
	/bin/mount -o noatime,move $2/tmp /tmp
	/bin/mount -o noatime,move $2/overlay /overlay 2>&-
	return 0
}

# Print all mount points at or below the given directories, deepest first.
# Paths stay in the escaped /proc/mounts form (\040 for a space, \011 for a
# tab, \012 for a newline, \134 for a backslash).  Keeping that form is what
# makes one path per line work even for a path containing a newline.  Callers
# decode it with printf '%b' before passing a path to mount or umount; the
# trailing dot they append keeps command substitution from stripping a path
# which ends in a decoded newline.
mount_points_below() { # <dir> [ <dir> ... ]
	awk -v dirs="$*" '
		BEGIN {
			n = split(dirs, dir, " ")
			count = 0
		}
		{
			for (i = 1; i <= n; i++) {
				if ($2 == dir[i] || index($2, dir[i] "/") == 1) {
					t = $2
					d = 0
					while ((p = index(t, "/")) > 0) {
						d++
						t = substr(t, p + 1)
					}
					count++
					mp[count] = $2
					depth[count] = d
					break
				}
			}
		}
		END {
			for (i = 1; i <= count; i++) {
				best = i
				for (j = i + 1; j <= count; j++)
					if (depth[j] > depth[best] ||
					    (depth[j] == depth[best] &&
					     length(mp[j]) > length(mp[best])))
						best = j
				if (best != i) {
					t = mp[i]; mp[i] = mp[best]; mp[best] = t
					t = depth[i]; depth[i] = depth[best]; depth[best] = t
				}
			}
			for (i = 1; i <= count; i++)
				print mp[i]
		}
	' /proc/mounts
}

# Unmount a set of mount trees, deepest first.  A read-only remount comes
# first: when a plain umount has to fall back to a lazy umount the filesystem
# is only detached from our namespace and stays writable, so a surviving
# writer could still touch the UBI volumes while they are being updated.
release_mounts() { # <dir> [ <dir> ... ]
	local mountpoint

	mount_points_below "$@" | while IFS= read -r mountpoint; do
		mountpoint=$(printf '%b.' "$mountpoint")
		mountpoint=${mountpoint%.}
		v "Releasing $mountpoint ..."
		mount -o remount,ro "$mountpoint" 2>/dev/null
		umount "$mountpoint" 2>/dev/null ||
			umount -l "$mountpoint" 2>/dev/null ||
			echo "Warning: could not release $mountpoint"
	done
}

# Release the previous root and every mount below it.
#
# Only /mnt is walked here: supivot() moves the whole /tmp mount and
# everything below it into the new root, so a mount under /tmp/mnt
# cannot pin the old rootfs.  Leaving those mounts alone also keeps a
# firmware image stored on one of them readable while it is flashed.
#
# This must run *after* the switch to the ramfs.  The shell which drives
# run_ramfs() executes /bin/busybox (and, for a script, keeps an fd on the
# script) from the old rootfs: an exe_file or an open script fd pins that
# mount, so the overlayfs stays active, its squashfs lower layer and its
# ubifs upper layer stay active as well, and ubiblock0_0 plus ubi0_0/ubi0_1
# remain busy ("cannot remove block device, error 16").  Only once the ramfs
# shell has replaced the old process nothing references the old root
# anymore and the layers can really go away.
release_old_root() {
	# The overlayfs at the old root has to go before its upper layer.
	release_mounts /mnt
	release_mounts /overlay

	sync

	# The kernel drops the last reference asynchronously.  Waiting for
	# /dev/ubiblock* to disappear here would be wrong: that device node is
	# only removed by ubiblock -r in nand_upgrade_prepare_ubi(), whose
	# retry loop is what gives the kernel time to release the reference.
	v "Old root unmounted"
}

# Services stopped by stop_services(), restored by start_services() when the
# upgrade could not be started.
SERVICE_STOP_LIST=/tmp/sysupgrade.stopped_services

# Run the given action for every init script, except the ones which are
# needed to finish the upgrade or which would tear the system down too early.
# Remote login and web services are skipped as well: stopping them from
# inside an SSH or LuCI session can kill the shell running sysupgrade.
run_services() { # <action> <verb>
	local skip=" ubus rpcd log boot umount network dropbear telnet uhttpd "
	local svc

	for svc in /etc/init.d/*; do
		svc="${svc##*/}"
		case "$skip" in *" $svc "*) continue;; esac
		[ -x "/etc/init.d/$svc" ] || continue
		v "$2 service $svc ..."
		[ "$1" = stop ] && echo "$svc" >> "$SERVICE_STOP_LIST"
		/etc/init.d/"$svc" "$1" >/dev/null 2>&1
	done

	return 0
}

# Stop all services before the upgrade starts, while procd and ubus are still
# around.  This is what makes a failing upgrade work when the user stops the
# services by hand first: the daemons close their files on /overlay and /mnt
# while they can still exit cleanly.  A daemon which is killed in the middle
# of I/O instead ends up in uninterruptible I/O, survives SIGKILL and keeps
# the old root - and with it ubiblock0_0 - busy.
stop_services() {
	: > "$SERVICE_STOP_LIST"
	run_services stop Stopping
	sync
	sleep 1
}

# Restore only the services which stop_services() actually stopped.
start_services() {
	local svc

	[ -f "$SERVICE_STOP_LIST" ] || return 0
	while read -r svc; do
		[ -n "$svc" ] || continue
		[ -x "/etc/init.d/$svc" ] || continue
		v "Starting service $svc ..."
		/etc/init.d/"$svc" start >/dev/null 2>&1
	done < "$SERVICE_STOP_LIST"
	rm -f "$SERVICE_STOP_LIST"

	return 0
}

run_ramfs() { # [ --release-old-root ] <command> [...]
	# Only the NAND hand-off needs the old root released from inside the
	# ramfs shell.  Every other upgrade keeps the legacy sequence below.
	if [ "$1" = "--release-old-root" ]; then
		do_release=1
		shift
	else
		do_release=0
	fi

	install_bin /bin/busybox /bin/ash /bin/sh /bin/mount /bin/umount	\
		/sbin/pivot_root /sbin/reboot /bin/sync /bin/dd	/bin/grep       \
		/bin/cp /bin/mv /bin/tar /usr/bin/md5sum "/usr/bin/[" /bin/dd	\
		/bin/vi /bin/ls /bin/cat /usr/bin/awk /usr/bin/hexdump		\
		/bin/sleep /bin/zcat /usr/bin/bzcat /usr/bin/printf /usr/bin/wc \
		/bin/cut /usr/bin/printf /bin/sync /bin/mkdir /bin/rmdir	\
		/bin/rm /usr/bin/basename /bin/kill /bin/chmod /usr/bin/find \
		/bin/mknod

	install_bin /bin/uclient-fetch /bin/wget
	install_bin /sbin/mtd
	install_bin /sbin/mount_root
	install_bin /sbin/snapshot
	install_bin /sbin/snapshot_tool
	install_bin /usr/sbin/ubiupdatevol
	install_bin /usr/sbin/ubiattach
	install_bin /usr/sbin/ubiblock
	install_bin /usr/sbin/ubiformat
	install_bin /usr/sbin/ubidetach
	install_bin /usr/sbin/ubirsvol
	install_bin /usr/sbin/ubirmvol
	install_bin /usr/sbin/ubimkvol
	install_bin /usr/sbin/partx
	install_bin /usr/sbin/losetup
	install_bin /usr/sbin/mkfs.ext4
	for file in $RAMFS_COPY_BIN; do
		install_bin ${file//:/ }
	done
	install_file /etc/resolv.conf /lib/*.sh /lib/functions/*.sh /lib/upgrade/*.sh $RAMFS_COPY_DATA

	[ -L "/lib64" ] && ln -s /lib $RAM_ROOT/lib64

	# NAND hand-off only: release the disks mounted below /mnt while the
	# old root is still in place.  Only plain umount is used, so nothing
	# kills the shell which is running the upgrade.  stop_services() is
	# not called on every platform, so a failure has to be reported
	# instead of being ignored: release_old_root() gets another chance
	# after the switch.  Mounts below /tmp/mnt are left alone, see
	# release_old_root().
	if [ "$do_release" -eq 1 ]; then
		mount_points_below /mnt | while IFS= read -r mountpoint; do
			mountpoint=$(printf '%b.' "$mountpoint")
			mountpoint=${mountpoint%.}
			v "Unmounting $mountpoint ..."
			umount "$mountpoint" 2>/dev/null ||
				echo "Warning: could not unmount $mountpoint"
		done
	fi

	supivot $RAM_ROOT /mnt || {
		echo "Failed to switch over to ramfs. Please reboot."
		exit 1
	}

	if [ "$do_release" -eq 1 ]; then
		# Spawn a new shell from the ramdisk to reduce the probability of
		# cache issues.  release_old_root() has to run inside that shell:
		# the current one still executes /bin/busybox and this script from
		# the old rootfs, which pins the old root and its overlay layers.
		exec /bin/busybox ash -c ". /lib/functions.sh; include /lib/upgrade; release_old_root; $*"
	fi

	# Legacy path for every non-NAND upgrade: keep the pre-exec release
	# sequence and the command line exactly as before, so NOR devices
	# see no change compared with the previous behaviour.
	/bin/mount -o remount,ro /mnt
	/bin/umount -l /mnt

	grep /overlay /proc/mounts > /dev/null && {
		/bin/mount -o noatime,remount,ro /overlay
		/bin/umount -l /overlay
	}

	# spawn a new shell from ramdisk to reduce the probability of cache issues
	exec /bin/busybox ash -c "$*"
}

kill_remaining() { # [ <signal> ]
	local sig="${1:-TERM}"
	echo -n "Sending $sig to remaining processes ... "

	local my_pid=$$
	local my_ppid=$(cut -d' ' -f4  /proc/$my_pid/stat)
	local my_ppisupgraded=
	grep -q upgraded /proc/$my_ppid/cmdline >/dev/null && {
		local my_ppisupgraded=1
	}
	
	local stat
	for stat in /proc/[0-9]*/stat; do
		[ -f "$stat" ] || continue

		local pid name state ppid rest
		read pid name state ppid rest < $stat
		name="${name#(}"; name="${name%)}"

		local cmdline
		read cmdline < /proc/$pid/cmdline

		# Skip kernel threads
		[ -n "$cmdline" ] || continue

		if [ $$ -eq 1 ] || [ $my_ppid -eq 1 ] && [ -n "$my_ppisupgraded" ]; then
			# Running as init process, kill everything except me
			if [ $pid -ne $$ ] && [ $pid -ne $my_ppid ]; then
				echo -n "$name "
				kill -$sig $pid 2>/dev/null
			fi
		else 
			case "$name" in
				# Skip essential services
				*procd*|*ash*|*init*|*watchdog*|*ssh*|*dropbear*|*telnet*|*login*|*hostapd*|*wpa_supplicant*|*nas*|*relayd*) : ;;

				# Killable process
				*)
					if [ $pid -ne $$ ] && [ $ppid -ne $$ ]; then
						echo -n "$name "
						kill -$sig $pid 2>/dev/null
					fi
				;;
			esac
		fi
	done
	echo ""
}

run_hooks() {
	local arg="$1"; shift
	for func in "$@"; do
		eval "$func $arg"
	done
}

ask_bool() {
	local default="$1"; shift;
	local answer="$default"

	[ "$INTERACTIVE" -eq 1 ] && {
		case "$default" in
			0) echo -n "$* (y/N): ";;
			*) echo -n "$* (Y/n): ";;
		esac
		read answer
		case "$answer" in
			y*) answer=1;;
			n*) answer=0;;
			*) answer="$default";;
		esac
	}
	[ "$answer" -gt 0 ]
}

v() {
	[ "$VERBOSE" -ge 1 ] && echo "$@"
}

rootfs_type() {
	/bin/mount | awk '($3 ~ /^\/$/) && ($5 !~ /rootfs/) { print $5 }'
}

get_image() { # <source> [ <command> ]
	local from="$1"
	local conc="$2"
	local cmd

	case "$from" in
		http://*|ftp://*) cmd="wget -O- -q";;
		*) cmd="cat";;
	esac
	if [ -z "$conc" ]; then
		local magic="$(eval $cmd \"$from\" 2>/dev/null | dd bs=2 count=1 2>/dev/null | hexdump -n 2 -e '1/1 "%02x"')"
		case "$magic" in
			1f8b) conc="zcat";;
			425a) conc="bzcat";;
		esac
	fi

	eval "$cmd \"$from\" 2>/dev/null ${conc:+| $conc}"
}

get_magic_word() {
	(get_image "$@" | dd bs=2 count=1 | hexdump -v -n 2 -e '1/1 "%02x"') 2>/dev/null
}

get_magic_long() {
	(get_image "$@" | dd bs=4 count=1 | hexdump -v -n 4 -e '1/1 "%02x"') 2>/dev/null
}

export_bootdevice() {
	local cmdline uuid disk uevent line
	local MAJOR MINOR DEVNAME DEVTYPE

	if read cmdline < /proc/cmdline; then
		case "$cmdline" in
			*block2mtd=*)
				disk="${cmdline##*block2mtd=}"
				disk="${disk%%,*}"
			;;
			*root=*)
				disk="${cmdline##*root=}"
				disk="${disk%% *}"
			;;
		esac

		case "$disk" in
			PARTUUID=[a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9][a-f0-9]-02)
				uuid="${disk#PARTUUID=}"
				uuid="${uuid%-02}"
				for disk in $(find /dev -type b); do
					set -- $(dd if=$disk bs=1 skip=440 count=4 2>/dev/null | hexdump -v -e '4/1 "%02x "')
					if [ "$4$3$2$1" = "$uuid" ]; then
						uevent="/sys/class/block/${disk##*/}/uevent"
						break
					fi
				done
			;;
			/dev/*)
				uevent="/sys/class/block/${disk##*/}/uevent"
			;;
		esac

		if [ -e "$uevent" ]; then
			while read line; do
				export -n "$line"
			done < "$uevent"
			export BOOTDEV_MAJOR=$MAJOR
			export BOOTDEV_MINOR=$MINOR
			return 0
		fi
	fi

	return 1
}

export_partdevice() {
	local var="$1" offset="$2"
	local uevent line MAJOR MINOR DEVNAME DEVTYPE

	for uevent in /sys/class/block/*/uevent; do
		while read line; do
			export -n "$line"
		done < "$uevent"
		if [ $BOOTDEV_MAJOR = $MAJOR -a $(($BOOTDEV_MINOR + $offset)) = $MINOR -a -b "/dev/$DEVNAME" ]; then
			export "$var=$DEVNAME"
			return 0
		fi
	done

	return 1
}

hex_le32_to_cpu() {
	[ "$(echo 01 | hexdump -v -n 2 -e '/2 "%x"')" == "3031" ] && {
		echo "${1:0:2}${1:8:2}${1:6:2}${1:4:2}${1:2:2}"
		return
	}
	echo "$@"
}

get_partitions() { # <device> <filename>
	local disk="$1"
	local filename="$2"

	if [ -b "$disk" -o -f "$disk" ]; then
		v "Reading partition table from $filename..."

		local magic=$(dd if="$disk" bs=2 count=1 skip=255 2>/dev/null)
		if [ "$magic" != $'\x55\xAA' ]; then
			v "Invalid partition table on $disk"
			exit
		fi

		rm -f "/tmp/partmap.$filename"

		local part
		for part in 1 2 3 4; do
			set -- $(hexdump -v -n 12 -s "$((0x1B2 + $part * 16))" -e '3/4 "0x%08X "' "$disk")

			local type="$(( $(hex_le32_to_cpu $1) % 256))"
			local lba="$(( $(hex_le32_to_cpu $2) ))"
			local num="$(( $(hex_le32_to_cpu $3) ))"

			[ $type -gt 0 ] || continue

			printf "%2d %5d %7d\n" $part $lba $num >> "/tmp/partmap.$filename"
		done
	fi
}

jffs2_copy_config() {
	if grep rootfs_data /proc/mtd >/dev/null; then
		# squashfs+jffs2
		mtd -e rootfs_data jffs2write "$CONF_TAR" rootfs_data
	else
		# jffs2
		mtd jffs2write "$CONF_TAR" rootfs
	fi
}

# Flash firmware to MTD partition
#
# $(1): path to image
# $(2): (optional) pipe command to extract firmware, e.g. dd bs=n skip=m
default_do_upgrade() {
	sync
	if [ "$SAVE_CONFIG" -eq 1 ]; then
		get_image "$1" "$2" | mtd $MTD_CONFIG_ARGS -j "$CONF_TAR" write - "${PART_NAME:-image}"
	else
		get_image "$1" "$2" | mtd write - "${PART_NAME:-image}"
	fi
}

do_upgrade() {
	v "Performing system upgrade..."
	if type 'platform_do_upgrade' >/dev/null 2>/dev/null; then
		platform_do_upgrade "$ARGV"
	else
		default_do_upgrade "$ARGV"
	fi

	if [ "$SAVE_CONFIG" -eq 1 ] && type 'platform_copy_config' >/dev/null 2>/dev/null; then
		platform_copy_config
	fi

	v "Upgrade completed"
	[ -n "$DELAY" ] && sleep "$DELAY"
	ask_bool 1 "Reboot" && {
		v "Rebooting system..."
		umount -a
		reboot -f
		sleep 5
		echo b 2>/dev/null >/proc/sysrq-trigger
	}
}
