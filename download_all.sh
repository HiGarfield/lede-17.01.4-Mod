#!/bin/bash

download_model_file(){
	echo ""
	echo "--------------------------------------------------------"
	echo "Downloading $1......"
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
		make -j8 download &&
		rm -f .config
	} || exit
}

clear

for file in conf/.config.*; do
	model_name="$(echo "$file" | sed 's/^conf\/\.config\.//g')"
	download_model_file "$model_name"
done
