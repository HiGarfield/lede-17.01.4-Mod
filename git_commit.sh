#!/bin/bash

{
	echo "Input commit message:" &&
	read commit_msg &&
	./umount_build_dir.sh &&
	rm -rf build_dir &&
	./clean_all.sh &&
	git add . &&
	git commit -m "$commit_msg"
} || exit 1
