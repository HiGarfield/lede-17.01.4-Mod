#!/bin/bash
./generate_conf_file.sh &&
git add . &&
[ -n "$(git diff --cached)" ] &&
git commit -m "refresh .config files"
