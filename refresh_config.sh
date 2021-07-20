#!/bin/bash

./generate_conf_file.sh &&
git add conf

if [ -n "$(git diff --cached)" ]; then
    git commit -m "refresh .config files"
fi

exit 0
