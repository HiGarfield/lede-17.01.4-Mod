#!/bin/sh
cksumfile="out/targets.sha256sum"
echo "sha256sum:" >"$cksumfile"

for file in out/*.bin; do
	res=$(sha256sum "$file")
	finename=$(echo "$res" | cut -d'/' -f2)
	s=$(echo "$res" | cut -d' ' -f1)
	echo $finename $s  >> "$cksumfile"
done
