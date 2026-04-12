#!/bin/bash

umount_tmpfs() {
	local mount_point="$1"
	local user_password="$2"

	if [ -z "$mount_point" ]; then
		echo "Usage: umount_tmpfs <mount_point>"
		return 1
	fi

	if mount | grep -wq "$mount_point"; then
		echo "$user_password" | sudo -S umount "$mount_point"
		echo "Unmounted tmpfs at $mount_point."
	else
		echo "Mount point $mount_point is not mounted."
	fi
}

echo "Unmount build_dir tmpfs."
umount_tmpfs "build_dir" "111111"
echo "Unmount tmp tmpfs."
umount_tmpfs "tmp" "111111"

exit 0
