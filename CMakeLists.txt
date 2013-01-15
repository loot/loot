# Settings passed on the command line:
#
# PROJECT_LIBS_DIR = the directory which all external libraries may be referenced from.
# PROJECT_ARCH = the build architecture
# PROJECT_LINK = whether to build a static or dynamic library.

##############################
# General Settings
##############################

cmake_minimum_required (VERSION 2.8.9)
project (boss)

set (BOSS_SRC "${CMAKE_SOURCE_DIR}/src/metadata.cpp" "${CMAKE_SOURCE_DIR}/src/game.cpp" "${CMAKE_SOURCE_DIR}/src/helpers.cpp" "${CMAKE_SOURCE_DIR}/src/plugin/ModFormat.cpp" "${CMAKE_SOURCE_DIR}/src/plugin/VersionRegex.cpp")

# Include source and library directories.
include_directories ("${PROJECT_LIBS_DIR}/alphanum" "${PROJECT_LIBS_DIR}/utf8" "${PROJECT_LIBS_DIR}/boost" "${PROJECT_LIBS_DIR}/yaml-cpp/include" "${CMAKE_SOURCE_DIR}/src" "${PROJECT_LIBS_DIR}/libloadorder/src")


##############################
# Platform-Specific Settings
##############################

# Settings when compiling for Windows.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    IF (${PROJECT_LINK} MATCHES "STATIC")
        add_definitions (-DLIBLO_STATIC)
        add_definitions (-DBOSS_STATIC)
    ELSE ()
        add_definitions (-DLIBLO_STATIC)
        add_definitions (-DBOSS_EXPORT)
    ENDIF ()
ENDIF ()

# Settings when not cross-compiling.
IF (CMAKE_SYSTEM_NAME MATCHES CMAKE_HOST_SYSTEM_NAME)
    link_directories ("${PROJECT_LIBS_DIR}/boost/stage-${PROJECT_ARCH}/lib")
ENDIF ()

link_directories ("${PROJECT_LIBS_DIR}/yaml-cpp/build")
link_directories ("${PROJECT_LIBS_DIR}/libloadorder/build")

# Settings when compiling and cross-compiling on Linux.
IF (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set (CMAKE_C_FLAGS  "-m${PROJECT_ARCH}")
    set (CMAKE_CXX_FLAGS "-m${PROJECT_ARCH}")
    set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set (CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set (CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

    IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        link_directories ("${PROJECT_LIBS_DIR}/boost/stage-mingw-${PROJECT_ARCH}/lib")
        set (BOSS_LIBS version)
    ENDIF ()

    set (BOSS_LIBS ${BOSS_LIBS} loadorder yaml-cpp boost_filesystem boost_system boost_regex)
ENDIF ()

# Settings when compiling on Windows.
IF (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set (BOSS_LIBS loadorder yaml-cpp libboost_filesystem-vc110-mt-1_52 libboost_system-vc110-mt-1_52)
    set (CMAKE_CXX_FLAGS "/EHsc")
ENDIF ()

##############################
# Actual Building
##############################

# Build API.
add_library           (boss ${PROJECT_LINK} "${CMAKE_SOURCE_DIR}/src/api.cpp" ${BOSS_SRC})
target_link_libraries (boss ${BOSS_LIBS})

# Build tester.
add_executable        (tester "${CMAKE_SOURCE_DIR}/src/tester.cpp" ${BOSS_SRC})
target_link_libraries (tester ${BOSS_LIBS})
