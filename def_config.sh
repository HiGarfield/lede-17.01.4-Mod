#!/bin/bash

for f in conf/.config.*; do
	echo "Processing $f"
	rm -rf  tmp/ .config .config.old
	cp "$f" ".config"
	make defconfig
	cp ".config" "$f"
done

rm -f .config .config.old

echo "Make defconfig done."