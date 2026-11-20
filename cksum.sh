#!/bin/bash
cksumfile="out/targets.sha256sum"
mkdir -p out
echo "sha256sum:" >"$cksumfile"

for file in out/*.bin; do
	[ -f "$file" ] || continue
	res=$(sha256sum "$file")
	finename=$(echo "$res" | cut -d'/' -f2)
	s=$(echo "$res" | cut -d' ' -f1)
	echo $finename $s  >> "$cksumfile"
done
