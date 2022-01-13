#!/bin/bash

tmpfs_size=12
user_password="111111"

build_dir_path="build_dir"
[ ! -d "${build_dir_path}" ] && mkdir -p "${build_dir_path}"
rm -rf ${build_dir_path}/*

freemem=$(awk '($1 == "MemTotal:") { print int($2/1048576) }' /proc/meminfo)

[ "$freemem" -gt "$tmpfs_size" ] && {
	echo "Mount build_dir as tmpfs."
	mount_record=`mount | grep "${build_dir_path}"`
	[ ! -n "${mount_record}" ] && {
		echo "$user_password" | sudo -S mount -t tmpfs -o size=${tmpfs_size}G myramdisk ${build_dir_path} 
		echo ""
	}
}

exit 0
