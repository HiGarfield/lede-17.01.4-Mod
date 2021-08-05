#!/bin/bash

./scripts/feeds uninstall -a
./scripts/feeds update -i
./scripts/feeds install -a
git add .

if [ -n "$(git diff --cached)" ]; then
    git commit -m "refresh feeds"
fi

exit 0
