#!/bin/bash

generate_device_yml() {
	local device_name="$1"
	local total_keep_num="$2"
	sed -e "s/template_device_please_replace/$device_name/g" \
		-e "s/template_total_keep_number_please_replace/$total_keep_num/g" \
		".github/build-lede-template.yml" \
		>".github/workflows/build-lede-${device_name}.yml"
}

process_devices() {
	local total_dev=${#devices[@]}
	echo "$total_dev devices in total."
	local total_keep_num=$((total_dev + 1))
	for i in $(seq 0 $((total_dev - 1))); do
		echo "processing ${devices[i]}"
		generate_device_yml "${devices[i]}" "$total_keep_num"
	done
	sed "s/template_total_keep_number_please_replace/$total_keep_num/g" \
		".github/lede-dl.yml" \
		>".github/workflows/lede-dl.yml"
}

rm -rf ".github/workflows/"
mkdir -p ".github/workflows/"

auto_find_device=0

if [ "$auto_find_device" -ne "1" ]; then
	devices=(
		# "tl-wr841-v8"
		"tl-wr841-v8-cn"
		"tl-wr941nd-v6"
		"tl-wr941nd-v6-cn"
		"tl-wr941n-v7"
		"tl-wr841-v9"
		"csac"
		"domywifi-dw33d"
		"hq55"
		"tl-wdr7500-v3"
		"tl-wdr4310-v1"
		"tl-wr1041n-v2"
		"tl-wr842n-v2"
		# "tl-wr842n-v2-cn"
		# "tl-wr941nd-v2"
		"wr1200js"
		"newifi-d2"
		"a3004ns"
		"ghl-r-001-e"
		"ghl-r-001-f"
		"y1s"
	)
else
	idx=0
	for file in conf/.config.*; do
		dev_name="$(basename "$file" | sed 's/\.config\.//')"
		devices[$idx]=$dev_name
		idx=$((idx + 1))
	done
fi

process_devices
