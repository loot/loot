# Build Instructions using Microsoft Visual C++

These instructions were used to build LOOT using Microsoft Visual Studio 2013 Community, though they should apply to other versions of MSVC.

#### Boost

```
bootstrap.bat
b2 toolset=msvc threadapi=win32 link=static runtime-link=static variant=release address-model=32 --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system  --with-iostreams
```

`link`, `runtime-link` and `address-model` can all be modified if shared linking or 64 bit builds are desired. LOOT uses statically-linked Boost libraries by default: to change this, edit [CMakeLists.txt](../CMakeLists.txt).

#### Chromium Embedded Framework

Most of the required binaries are pre-built, but the libcef_dll_wrapper dynamic library must be built.

1. Configure CMake and generate a build system for Visual Studio by running:

   ```
   mkdir build && cd build
   cmake.exe .. -G "Visual Studio 12"
   ```

2. Open the generated solution file, and build it with `Release` configuration.

#### Google Test

1. Configure CMake and generate a build system for Visual Studio by running:

   ```
   mkdir build && cd build
   cmake.exe .. -G "Visual Studio 12"
   ```

2. Open the generated solution file, and build it with `Release` configuration.

#### Libgit2

1. Configure CMake and generate a build system for Visual Studio by running:

   ```
   mkdir build && cd build
   cmake.exe .. -G "Visual Studio 12" -DBUILD_SHARED_LIBS=OFF
   ```

  Adapt the commands as necessary for your particular setup.
2. Open the generated solution file, and build it with `Release` configuration.

#### Libloadorder

Follow the instructions in libloadorder's README.md to build it as a static library.

Example CMake keys: `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=build -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build`

#### yaml-cpp

1. Configure CMake and generate a build system for Visual Studio by running:

   ```
   mkdir build && cd build
   cmake.exe .. -G "Visual Studio 12" -DBOOST_ROOT={BOOST_ROOT} -DMSVC_SHARED_RT=OFF
   ```

   Adapt the commands as necessary for your particular setup.
2. Open the generated solution file, and build it with `Release` configuration.

#### LOOT

LOOT uses a third-party CMake module to generate its build revision data, so first download the `GetGitRevisionDescription.cmake` and `GetGitRevisionDescription.cmake.in` files found in [this repository](https://github.com/rpavlik/cmake-modules) to the `build` subdirectory of the LOOT folder.

1. Set CMake up so that it builds the binaries in the `build` subdirectory of the LOOT folder.
2. Define any necessary parameters.
3. Configure CMake, then generate a build system for Visual Studio 12.
4. Open the generated solution file, and build it.
