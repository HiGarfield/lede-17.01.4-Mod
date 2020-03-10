#!/bin/bash

build_dir_path="`pwd`/build_dir"
[ ! -d "${build_dir_path}" ] && mkdir -p "${build_dir_path}"
mount_record=`mount | grep "${build_dir_path}"`
[ ! -n "${mount_record}" ] && {
	echo "123456" | sudo -S mount -t tmpfs -o size=9G myramdisk ${build_dir_path} 
	echo ""
}
