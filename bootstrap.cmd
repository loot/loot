@cd "libs\boost"
@call bootstrap.bat
@b2 toolset=msvc threadapi=win32 link=static variant=release address-model=32 --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system  --with-iostreams --stagedir=stage-32
@cd "..\.."

@cd "libs\wxWidgets"
@devenv build\msw\wx_vc10.sln /Upgrade
@devenv build\msw\wx_vc10.sln /Build Release
cd "..\.."

@cd "libs\zlib"
@copy "..\..\contrib\zlib\bld_ml32.bat" "contrib\masmx86\bld_ml32.bat"
@cmake . -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build
@devenv zlib.sln /build Release /project zlibstatic.vcxproj
@cd "..\.."

@cd "libs\yaml-cpp"
@cmake . -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=build -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build -DBOOST_ROOT=..\boost
@devenv YAML_CPP.sln /build Release
@cd "..\.."

@cd "libs\libloadorder"
@cmake . -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=build -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build -DPROJECT_ARCH=32 -DPROJECT_LINK=STATIC -DPROJECT_LIBS_DIR=..
@devenv libloadorder.sln /build Release
@cd "..\.."

@cd "libs\libgit2"
@cmake . -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=build -DBUILD_SHARED_LIBS=OFF -DSTATIC_CRT=OFF
@devenv libgit2.sln /build Release
@cd "..\.."

@cmake .  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=build -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=build -DPROJECT_ARCH=32 -DPROJECT_LINK=STATIC -DPROJECT_LIBS_DIR=libs