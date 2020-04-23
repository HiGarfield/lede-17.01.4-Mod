#!/bin/bash
rm -rf bin build_dir/* tmp staging_dir out .config .config.old
[ -L dl ] && rm -f dl
make -C scripts/config clean >/dev/null 2>&1
make -C feeds/luci/modules/luci-base/src clean >/dev/null 2>&1
