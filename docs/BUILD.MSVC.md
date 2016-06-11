# Build Instructions using Microsoft Visual C++

These instructions were used to build LOOT using Microsoft Visual Studio 2015 Community, though they should apply to other versions of MSVC.

#### Boost

```
bootstrap.bat
b2 toolset=msvc threadapi=win32 link=static runtime-link=static variant=release address-model=32 --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system  --with-iostreams
```

`link`, `runtime-link` and `address-model` can all be modified if shared linking or 64 bit builds are desired. LOOT uses statically-linked Boost libraries by default: to change this, edit [CMakeLists.txt](../CMakeLists.txt).

#### LOOT

1. Set CMake up so that it builds the binaries in the `build` subdirectory of the LOOT folder.
2. Define any necessary parameters.
3. Configure CMake, then generate a build system for Visual Studio.
4. Open the generated solution file, and build it.
