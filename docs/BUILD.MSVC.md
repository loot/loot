# Build Instructions using Microsoft Visual C++

These instructions were used to build LOOT using Microsoft Visual Studio 2012 and Microsoft Visual Studio Express 2013 for Desktop, though they should also apply to other versions of MSVC.

LOOT's CMake configuration builds an executable that can be run on Windows XP, but this support has only been implemented for MSVC 2013 - other versions may require editing of LOOT's `CMakeLists.txt` file.

#### Boost

```
bootstrap.bat
b2 toolset=msvc threadapi=win32 link=static variant=release address-model=32 --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system  --with-iostreams --stagedir=stage-32
```

#### wxWidgets

Just build the solution provided by wxWidgets.

#### zlib

1. Add `/safeseh` to both lines in `contrib\masmx86\bld_ml32.bat`.
2. Configure CMake and generate a build system for Visual Studio by running CMake with keys `-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build`.
3. Open the generated solution file, and build target `zlibstatic` with `Release` configuration.

#### yaml-cpp

1. Configure CMake and generate a build system for Visual Studio by running CMake with keys `-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=build -DBOOST_ROOT={BOOST_ROOT}`.
2. Open the generated solution file, and build it with `Release` configuration.

#### Libloadorder

Follow the instructions in libloadorder's README.md to build it as a static library.

CMake keys: `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=build -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build -DPROJECT_ARCH=32 -DPROJECT_LINK=STATIC -DPROJECT_LIBS_DIR=..`

#### Libgit2

1. Configure CMake and generate a build system for Visual Studio by running CMake with keys `-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=build -DBUILD_SHARED_LIBS=OFF -DSTATIC_CRT=OFF`.
2. Open the generated solution file, and build it with `Release` configuration.

#### LOOT

1. Set CMake up so that it builds the binaries in the `build` subdirectory of the LOOT folder.
2. Define `PROJECT_ARCH=32` or `PROJECT_ARCH=64` to build 32 or 64 bit executables respectively.
3. Define `PROJECT_LINK=STATIC` to build a static API, or `PROJECT_LINK=SHARED` to build a DLL API.
4. Define `PROEJCT_LIBS_DIR` to point to the folder holding all the required libraries' folders.
5. Configure CMake, then generate a build system for Visual Studio 12.
6. Open the generated solution file, and build it.
