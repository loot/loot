#!/bin/bash
set -ev

# Move out of LOOT repo path.
cd ../..

# Install libespm.
wget https://github.com/WrinklyNinja/libespm/archive/2.5.0.tar.gz -O - | tar -xz
mv libespm-2.5.0 libespm

# Build yaml-cpp
wget https://github.com/WrinklyNinja/yaml-cpp/archive/patched-for-loot.tar.gz -O - | tar -xz
mv yaml-cpp-patched-for-loot yaml-cpp
mkdir yaml-cpp/build && cd yaml-cpp/build
cmake ..
make yaml-cpp
cd ../..

# Build libgit2
wget https://github.com/libgit2/libgit2/archive/v0.24.0.tar.gz -O - | tar -xz
mv libgit2-0.24.0 libgit2
mkdir libgit2/build && cd libgit2/build
cmake .. -DBUILD_SHARED_LIBS=OFF
make git2
cd ../..

# Build libloadorder
wget https://github.com/WrinklyNinja/libloadorder/archive/7.0.0.tar.gz -O - | tar -xz
mv libloadorder-7.0.0 libloadorder
mkdir libloadorder/build && cd libloadorder/build
cmake .. -DPROJECT_ARCH=64 -DPROJECT_STATIC_RUNTIME=OFF -DBUILD_SHARED_LIBS=OFF -DGTEST_ROOT=../gtest-1.7.0
make loadorder64
cd ../..

# Install pseudosem
wget https://github.com/WrinklyNinja/pseudosem/archive/1.0.1.tar.gz -O - | tar -xz
mv pseudosem-1.0.1 pseudosem
