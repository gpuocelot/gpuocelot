#!/bin/sh
# Build script for Docker
set -e
cd /gpuocelot_llvm/ThirdParty/llvm
rm -rf build
mkdir build
cd build
cmake ..
make -j8
