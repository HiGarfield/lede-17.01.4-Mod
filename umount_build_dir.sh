#!/bin/bash

user_password="111111"

build_dir_path="build_dir"
mount_record=`mount | grep "${build_dir_path}"`
[ -n "${mount_record}" ] && {
	echo "$user_password" | sudo -S umount "${build_dir_path}"
	echo ""
}

exit 0
