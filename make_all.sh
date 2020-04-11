#!/bin/bash

make_model(){
	echo ""
	echo "--------------------------------------------------------"
	echo "Building $1......"
	echo "--------------------------------------------------------"
	[ ! -f "conf/.config.$1" ]  && {
		echo "conf/.config.$1 does not exist!"
		return
	}
	[ ! -d "out" ] && mkdir "out"
	{
		./clean_all.sh
		cp -f "conf/.config.$1" ".config" &&
		make defconfig &&
		make dirclean  >/dev/null 2>&1 &&
		make download -j$(nproc) &&
		make -j$(nproc) &&
		cp -u -f bin/targets/*/*/lede-*-squashfs-sysupgrade.bin out/ &&
		make dirclean  >/dev/null 2>&1 &&
		rm -rf bin/* build_dir/* tmp/ staging_dir/* .config
	} || {
		make dirclean  >/dev/null 2>&1 &&
		make -j1 V=s
	} || exit
}

clear

tmpfs_size=9
user_password="111111"

build_dir_path="`pwd`/build_dir"
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

for file in conf/.config.*; do
	model_name="$(echo "$file" | sed 's/^conf\/\.config\.//g')"
	make_model "$model_name"
done

./cksum.sh

rm -rf ${build_dir_path}/*
mount_record=`mount | grep "${build_dir_path}"`
[ -n "${mount_record}" ] && {
	echo "$user_password" | sudo -S umount ${build_dir_path} 
	echo ""
}

