#!/bin/bash

[ "$#" -ne 1 ] && {
    echo "Usage: $0 package_path"
    exit 1
}

[ -f ".config" ] || {
    echo ".config" does not exist.
    exit 1
}

pkg_path="${1%/}"

[ -d "${pkg_path}" ] || {
    echo "${pkg_path}" does not exist.
    exit 1
}

[ -f "${pkg_path}/Makefile" ] || {
    echo "${pkg_path}/Makefile" does not exist.
    exit 1
}

make "${pkg_path}/download" V=s && {
    make "${pkg_path}/check" FIXUP=1 V=s
    [ -d "${pkg_path}/patches" ] && 
        make "${pkg_path}/refresh" V=s
} 
