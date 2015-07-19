#!/bin/bash
set -ev

# Currently inside the cloned repo path.
# Get the 3rd-party CMake modules.
wget -P build https://raw.githubusercontent.com/rpavlik/cmake-modules/master/GetGitRevisionDescription.cmake
wget -P build https://raw.githubusercontent.com/rpavlik/cmake-modules/master/GetGitRevisionDescription.cmake.in
cd ../..

# Install libespm.
wget https://github.com/WrinklyNinja/libespm/archive/master.tar.gz -O - | tar -xz
mv libespm-master libespm

# Build yaml-cpp
wget https://github.com/WrinklyNinja/yaml-cpp/archive/patched-for-loot.tar.gz -O - | tar -xz
mv yaml-cpp-patched-for-loot yaml-cpp
mkdir yaml-cpp/build && cd yaml-cpp/build
cmake ..
make yaml-cpp
cd ../..

# Build libgit2
wget https://github.com/libgit2/libgit2/archive/v0.23.0.tar.gz -O - | tar -xz
mv libgit2-0.23.0 libgit2
mkdir libgit2/build && cd libgit2/build
cmake ..
make git2
cd ../..

# Build libloadorder
wget https://github.com/WrinklyNinja/libloadorder/archive/master.tar.gz -O - | tar -xz
mv libloadorder-master libloadorder
mkdir libloadorder/build && cd libloadorder/build
cmake .. -DPROJECT_ARCH=64 -DPROJECT_STATIC_RUNTIME=OFF -DGTEST_ROOT=../gtest-1.7.0
make loadorder64
cd ../..
