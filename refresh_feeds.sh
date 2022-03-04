#!/bin/bash

rm -rf feeds/*.index feeds/*.targetindex feeds/*.tmp packages/feeds
./scripts/feeds update -i
./scripts/feeds install -a
git add feeds package/feeds

if [ -n "$(git diff --cached)" ]; then
    git commit -m "refresh feeds"
fi

exit 0
