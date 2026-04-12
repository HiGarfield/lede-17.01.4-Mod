#!/bin/bash

mount_tmpfs() {
	local mount_point="$1"
	local size="$2"
	local password="$3"

	if [ -z "$mount_point" ] || [ -z "$size" ] || [ -z "$password" ]; then
		echo "Usage: mount_tmpfs <mount_point> <size_in_GB> <password>"
		return 1
	fi

	if ! mount | grep -wq "$mount_point"; then
		[ -d "$mount_point" ] || sudo mkdir -p "$mount_point"
		rm -rf "${mount_point:?}"/*
		echo "$password" | sudo -S mount -t tmpfs -o size="${size}G" myramdisk "$mount_point"
		echo "Mounted tmpfs at $mount_point with size ${size}G."
	else
		echo "Mount point $mount_point is already mounted."
	fi
}

echo "Mount build_dir as tmpfs."
mount_tmpfs "build_dir" 12 "111111"
echo "Mount tmp as tmpfs."
mount_tmpfs "tmp" 4 "111111"

exit 0
