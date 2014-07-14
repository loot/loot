# Settings passed on the command line:
#
# PROJECT_ARCH = the build architecture
# LIBESPM_ROOT = the path to the root of the libespm source.

##############################
# General Settings
##############################

cmake_minimum_required (VERSION 2.8.9)
project (LOOT)

option(BUILD_SHARED_LIBS "Build a shared library" OFF)
option(PROJECT_STATIC_RUNTIME "Build with static runtime libs (/MT)" ON)

IF (NOT DEFINED PROJECT_ARCH)
    set (PROJECT_ARCH "32")
ENDIF ()

IF (NOT DEFINED ALPHANUM_ROOT)
    set (ALPHANUM_ROOT "../../alphanum")
ENDIF ()

IF (NOT DEFINED LIBESPM_ROOT)
    set (LIBESPM_ROOT "../../libespm")
ENDIF ()

IF (NOT DEFINED LIBGIT2_ROOT)
    set (LIBGIT2_ROOT "../../libgit2")
ENDIF ()

IF (NOT DEFINED LIBLOADORDER_ROOT)
    set (LIBLOADORDER_ROOT "../../libloadorder")
ENDIF ()

IF (NOT DEFINED YAMLCPP_ROOT)
    set (YAMLCPP_ROOT "../../yaml-cpp")
ENDIF ()

IF (NOT DEFINED WXWIDGETS_ROOT)
    set (WXWIDGETS_ROOT "../../wxWidgets")
ENDIF ()

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME PROJECT_STATIC_RUNTIME)

set (LOOT_SRC   "${CMAKE_SOURCE_DIR}/src/backend/metadata.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/game.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/globals.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/generators.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/sort.cpp")

set (LOOT_GUI_SRC ${LOOT_SRC}
                  # Code the API doesn't need.
                  "${CMAKE_SOURCE_DIR}/src/backend/graph.cpp"
                  "${CMAKE_SOURCE_DIR}/src/backend/git.cpp"
                  # Actual GUI code.
                  "${CMAKE_SOURCE_DIR}/src/gui/ids.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/main.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/settings.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/editor.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/viewer.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/misc.cpp"
                  "${CMAKE_SOURCE_DIR}/src/resource.rc")

set (LOOT_API_SRC ${LOOT_SRC}
                  "${CMAKE_SOURCE_DIR}/src/api/api.cpp")

find_package(Boost REQUIRED COMPONENTS log log_setup locale thread chrono date_time filesystem system regex iostreams)

# Include source and library directories.
include_directories ("${CMAKE_SOURCE_DIR}/src"
                     ${ALPHANUM_ROOT}
                     "${YAMLCPP_ROOT}/include"
                     "${LIBLOADORDER_ROOT}/src"
                     "${LIBGIT2_ROOT}/include"
                     "${WXWIDGETS_ROOT}/include"
                     ${LIBESPM_ROOT}
                     ${Boost_INCLUDE_DIRS})

link_directories ("${LIBLOADORDER_ROOT}/build"
                  "${LIBGIT2_ROOT}/build"
                  "${YAMLCPP_ROOT}/build")


##############################
# System-Specific Settings
##############################

# Settings when compiling for Windows. Since it's a Windows-only app this is always true, but useful to check for copy/paste into other projects.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions (-DUNICODE -D_UNICODE -D__WXMSW__ -DNDEBUG -DLIBLO_STATIC -DWIN32 -D_WINDOWS)
    IF (BUILD_SHARED_LIBS)
        add_definitions (-DLOOT_EXPORT)
    ELSE ()
        add_definitions (-DLOOT_STATIC)
    ENDIF ()
ENDIF ()

# MinGW settings.
IF (MINGW)
    include_directories("${WXWIDGETS_ROOT}/lib/wx/include/${COMPILER_PREFIX}-msw-unicode-static-3.0")
    link_directories("${WXWIDGETS_ROOT}/lib")
    set (CMAKE_C_FLAGS  "-m${PROJECT_ARCH} -O3")
    set (CMAKE_CXX_FLAGS "-m${PROJECT_ARCH} -O3")
    set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -Wl,--subsystem,windows")
    set (CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc -Wl,--subsystem,windows")
    set (CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -Wl,--subsystem,windows")
    set (LOOT_LIBS ${LOOT_LIBS}
                   yaml-cpp
                   boost_log_setup
                   boost_log
                   boost_locale
                   boost_thread_win32
                   boost_chrono
                   boost_date_time
                   boost_filesystem
                   boost_system
                   boost_regex
                   version
                   loadorder${PROJECT_ARCH}
                   ssl
                   crypto
                   ws2_32)
    set (LOOT_GUI_LIBS ${LOOT_LIBS}
                       git2
                       wx_mswu_webview-3.0-${COMPILER_PREFIX}
                       wx_mswu_core-3.0-${COMPILER_PREFIX}
                       wx_baseu-3.0-${COMPILER_PREFIX}
                       wx_mswu_adv-3.0-${COMPILER_PREFIX}
                       wxpng-3.0-${COMPILER_PREFIX}
                       wxzlib-3.0-${COMPILER_PREFIX}
                       comctl32)
ELSEIF (MSVC)
    include_directories("${WXWIDGETS_ROOT}/lib/vc_lib/mswu")
    link_directories("${WXWIDGETS_ROOT}/lib/vc_lib")

    # Force static C++ runtime linkage.
    IF (PROJECT_STATIC_RUNTIME)
        FOREACH(flag
            CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_INIT
            CMAKE_CXX_FLAGS_RELEASE  CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_DEBUG  CMAKE_CXX_FLAGS_DEBUG_INIT)
            STRING(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
            SET("${flag}" "${${flag}} /EHsc")
        ENDFOREACH()
    ENDIF ()

    set (CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:WINDOWS")
    set (LOOT_LIBS libyaml-cppmt
                   version
                   loadorder${PROJECT_ARCH}
                   ws2_32)
    set (LOOT_GUI_LIBS ${LOOT_LIBS}
                       git2
                       wxmsw30u_webview
                       wxmsw30u_adv
                       wxmsw30u_core
                       wxbase30u
                       wxpng
                       wxzlib
                       comctl32)
ENDIF ()


##############################
# Actual Building
##############################

# Build API.
add_library           (loot${PROJECT_ARCH} ${LOOT_API_SRC})
target_link_libraries (loot${PROJECT_ARCH} ${Boost_LIBRARIES} ${LOOT_LIBS})

# Build application.
add_executable        (LOOT ${LOOT_GUI_SRC})
target_link_libraries (LOOT ${Boost_LIBRARIES} ${LOOT_GUI_LIBS})
