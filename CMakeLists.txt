# Settings passed on the command line:
#
# LIBSTR_LIBS_DIR = the directory which all external libraries may be referenced from.
# LIBSTR_ARCH = the build architecture
# LIBSTR_LINK = whether to build a static or dynamic library.

##############################
# General Settings
##############################

cmake_minimum_required (VERSION 2.8.9)
project (boss)

set (BOSS_SRC )

# Include source and library directories.
include_directories ("${BOSS_LIBS_DIR}/boost" "${BOSS_LIBS_DIR}/yaml-cpp/include" "${CMAKE_SOURCE_DIR}/src")

##############################
# Platform-Specific Settings
##############################

# Settings when compiling for Windows.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    #IF (${BOSS_LINK} MATCHES "STATIC")
    #    add_definitions (-DBOSS_STATIC)
    #ELSE ()
    #    add_definitions (-DBOSS_EXPORT)
    #ENDIF ()
ENDIF ()

# Settings when compiling on Windows.
IF (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    #set (BOSS_LIBS libboost_filesystem-vc110-mt-1_52 libboost_system-vc110-mt-1_52)
    set (CMAKE_CXX_FLAGS "/EHsc")
ENDIF ()

# Settings when compiling and cross-compiling on Linux.
IF (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set (BOSS_LIBS yaml-cpp)
    set (CMAKE_C_FLAGS  "-m${BOSS_ARCH}")
    set (CMAKE_CXX_FLAGS "-m${BOSS_ARCH}")
    set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set (CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set (CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

    link_directories ("${BOSS_LIBS_DIR}/yaml-cpp/build/")

    IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        link_directories ("${BOSS_LIBS_DIR}/yaml-cpp/build/")
    ENDIF ()
ENDIF ()

# Settings when not cross-compiling.
IF (CMAKE_SYSTEM_NAME MATCHES CMAKE_HOST_SYSTEM_NAME)
    link_directories ("${BOSS_LIBS_DIR}/yaml-cpp/build/")
ENDIF ()

##############################
# Actual Building
##############################

# Build tester.
add_executable        (parser-tester "${CMAKE_SOURCE_DIR}/src/tester.cpp")
target_link_libraries (parser-tester ${BOSS_LIBS})
