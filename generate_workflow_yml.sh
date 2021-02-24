#!/bin/bash

generate_device_yml() {
	local device_name="$1"
	local total_keep_num="$2"
	sed -e "s/template_device_please_replace/$device_name/" \
		-e "s/template_total_keep_number_please_replace/$total_keep_num/" \
		".github/build-lede-template.yml" \
		>".github/workflows/build-lede-${device_name}.yml"
}

mkdir -p ".github/workflows/"
rm -rf ".github/workflows/*"

total_keep_num=$(($(find conf/.config.* -type f | wc -l) + 1))

for file in conf/.config.*; do
	dev_name="$(basename "$file" | sed 's/\.config\.//')"
	generate_device_yml "$dev_name" "$total_keep_num"
done

sed "s/template_total_keep_number_please_replace/$total_keep_num/" \
	".github/lede-dl.yml" \
	>".github/workflows/lede-dl.yml"
