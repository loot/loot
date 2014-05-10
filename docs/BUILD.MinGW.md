# Build Instructions using MinGW

These instructions were used to build LOOT using mingw-w64 on Ubuntu and Debian linux, though they should also apply to other similar environments.

#### Boost

```
./bootstrap.sh
echo "using gcc : 4.6.3 : i686-w64-mingw32-g++ : <rc>i686-w64-mingw32-windres <archiver>i686-w64-mingw32-ar <ranlib>i686-w64-mingw32-ranlib ;" > tools/build/v2/user-config.jam
./b2 toolset=gcc-4.6.3 target-os=windows threadapi=win32 link=static runtime-link=static variant=release address-model=32 cxxflags=-fPIC --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system --with-iostreams --stagedir=stage-32
```

#### wxWidgets

```
./configure --host=i686-w64-mingw32 --disable-shared --enable-stl
make
```

#### yaml-cpp

```
cmake . -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../LOOT/mingw-toolchain.cmake -DBOOST_ROOT=../boost
make
```

#### Libloadorder

Follow the instructions in libloadorder's README.md to build it as a static library.

#### Libgit2

```
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DOPENSSL_ROOT_DIR=../openssl -DCMAKE_C_FLAGS=-m32 -DPROJECT_ARCH=32 -DCMAKE_TOOLCHAIN_FILE=../LOOT/mingw-toolchain.cmake
make
```

#### LOOT

```
mkdir build && cd build
cmake .. -DPROJECT_LIBS_DIR=".." -DPROJECT_ARCH=32 -DPROJECT_LINK=STATIC -DCMAKE_TOOLCHAIN_FILE="mingw-toolchain.cmake"
make
```
