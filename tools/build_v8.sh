#!/bin/bash -e

V8_RELEASE_TAG=9.1.269.36
DEPOT_TOOLS_REPO=https://chromium.googlesource.com/chromium/tools/depot_tools.git

git -C depot_tools pull || git clone ${DEPOT_TOOLS_REPO} depot_tools

# export PATH=$PWD/depot_tools:$PATH
# gn args x64.release
# ninja -C x64.release

if [ ! -d "v8" ]; then
  fetch v8
  cd v8
else
  cd v8
  gclient sync
fi

if [ ! $(git describe --tags --abbrev=0) = "${V8_RELEASE_TAG}" ]; then
    git checkout tags/${V8_RELEASE_TAG}
    gclient sync
fi
