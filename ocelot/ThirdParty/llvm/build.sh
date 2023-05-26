#!/usr/bin/env bash
# Build script for Docker
set -e
cd $(dirname "$0")
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$1 ..
make -j8
