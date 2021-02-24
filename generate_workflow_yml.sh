#!/bin/bash

generate_device_yml() {
	local device_name="$1"
	sed "s/template_device_please_replace/$device_name/" ".github/build-lede-template.yml" >".github/workflows/build-lede-${device_name}.yml"
}

mkdir -p ".github/workflows/"
rm -rf .github/workflows/*

for file in conf/.config.*; do
	dev_name="$(basename "$file" | sed 's/\.config\.//')"
	generate_device_yml "$dev_name"
done

cp -f .github/lede-dl.yml .github/workflows/
