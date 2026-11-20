#!/bin/bash

# Some devices produce only a sysupgrade image, so an unmatched image
# glob must not survive as a literal file name in the cp command.
shopt -s nullglob

make_model() {
	echo ""
	echo "--------------------------------------------------------"
	echo "Building $1......"
	echo "--------------------------------------------------------"
	[ ! -f "conf/.config.$1" ] && {
		echo "conf/.config.$1 does not exist!"
		return
	}
	{
		cp -f "conf/.config.$1" ".config" &&
			make defconfig &&
			make dirclean >/dev/null 2>&1 &&
			make download -j$(($(nproc) + 1)) &&
			make -j$(($(nproc) + 1)) &&
			cp -u -f bin/targets/*/*/lede-*-sysupgrade.bin \
				bin/targets/*/*/lede-*-factory.bin \
				out/ &&
			make dirclean >/dev/null 2>&1 &&
			rm -rf bin/* build_dir/* tmp/ staging_dir/* .config
	} || {
		make dirclean >/dev/null 2>&1 &&
			make -j1 V=s
	} || exit
}

clear
./clean_all.sh
./mount_tmpfs.sh
[ ! -d "out" ] && mkdir "out"

for file in conf/.config.*; do
	model_name="${file##conf\/\.config\.}"
	make_model "$model_name"
done

./cksum.sh
./umount_tmpfs.sh
