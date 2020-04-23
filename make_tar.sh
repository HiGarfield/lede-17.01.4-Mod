#!/bin/bash

[ -f ".config" ] && { make dirclean; rm -f ".config"; }
[ -f ".config.old" ] && rm -f ".config.old"

./umount_build_dir.sh
rm -rf build_dir
./clean_all.sh

my_ver="$(cat version)"

folder_name="$(basename $(pwd))"
tar_file_name="$(basename $(pwd))-${my_ver}.tar.gz"
cd .. &&
tar -czvf "${tar_file_name}" "${folder_name}" &&
sha256sum "${tar_file_name}" > "${tar_file_name}.sha256sum" &&
cd "${folder_name}"
