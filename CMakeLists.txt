# Settings passed on the command line:
#
# PROJECT_ARCH = the build architecture
# LIBESPM_ROOT = the path to the root of the libespm source.

##############################
# Get Build Revision
##############################

# Uses the CMake module in this repo: https://github.com/rpavlik/cmake-modules
# On Windows, this requires the msysgit install of Git, as the script won't find
# the one bundled with GitHub for Windows.

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_COMMIT_STRING --tags --long)

IF (GIT_COMMIT_STRING MATCHES "GITDIR-NOTFOUND")
    set(GIT_COMMIT_STRING "unknown")
ELSE ()
    # Get a shortened ID. Although knowing the shortest unique string requires Git,
    # 8 characters should be a good compromise between chance of ambiguity and length,
    # especially since the version numbers would narrow down the range of possibly
    # ambiguous commits.
    string(SUBSTRING ${GIT_COMMIT_STRING} 0 8 GIT_COMMIT_STRING)
ENDIF ()

message(STATUS "Git revision: ${GIT_COMMIT_STRING}")

# Write to file.
configure_file("${CMAKE_SOURCE_DIR}/src/backend/globals.cpp.in" "${CMAKE_BINARY_DIR}/generated/globals.cpp" @ONLY)

##############################
# General Settings
##############################

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0015 NEW)
project (LOOT)

option(BUILD_SHARED_LIBS "Build a shared library" ON)
option(PROJECT_STATIC_RUNTIME "Build with static runtime libs (/MT)" ON)

IF (NOT DEFINED PROJECT_ARCH)
    set (PROJECT_ARCH "32")
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

IF (NOT DEFINED CEF_ROOT)
    set (CEF_ROOT "../../cef")
ENDIF ()

set (Boost_USE_STATIC_LIBS ${PROJECT_STATIC_RUNTIME})
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME ${PROJECT_STATIC_RUNTIME})

IF (NOT Boost_USE_STATIC_LIBS)
    add_definitions(-DBOOST_LOG_DYN_LINK)
ENDIF ()

find_package(Boost REQUIRED COMPONENTS log log_setup regex locale thread date_time chrono filesystem system iostreams)
find_package(yaml-cpp)
find_package(GTest)

set (LOOT_SRC   "${CMAKE_SOURCE_DIR}/src/backend/metadata/conditional_metadata.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/file.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/formid.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/location.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/message.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/message_content.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/plugin_dirty_info.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/plugin_metadata.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/tag.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/game/game.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/game/game_settings.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/game/load_order_handler.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata_list.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/masterlist.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/plugin/plugin.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/plugin/plugin_loader.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/git_helper.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/helpers.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/language.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/version.cpp"
                "${CMAKE_SOURCE_DIR}/src/backend/plugin_sorter.cpp"
                "${CMAKE_BINARY_DIR}/generated/globals.cpp")

set (LOOT_HEADERS "${CMAKE_SOURCE_DIR}/src/backend/metadata/condition_grammar.h"
				"${CMAKE_SOURCE_DIR}/src/backend/metadata/conditional_metadata.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/file.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/formid.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/location.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/message.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/message_content.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/plugin_dirty_info.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/plugin_metadata.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata/tag.h"
                "${CMAKE_SOURCE_DIR}/src/backend/game/game.h"
                "${CMAKE_SOURCE_DIR}/src/backend/game/game_settings.h"
                "${CMAKE_SOURCE_DIR}/src/backend/game/load_order_handler.h"
                "${CMAKE_SOURCE_DIR}/src/backend/metadata_list.h"
                "${CMAKE_SOURCE_DIR}/src/backend/masterlist.h"
                "${CMAKE_SOURCE_DIR}/src/backend/plugin/plugin.h"
                "${CMAKE_SOURCE_DIR}/src/backend/plugin/plugin_loader.h"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/git_helper.h"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/helpers.h"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/language.h"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/streams.h"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/version.h"
                "${CMAKE_SOURCE_DIR}/src/backend/helpers/yaml_set_helpers.h"
                "${CMAKE_SOURCE_DIR}/src/backend/globals.h"
                "${CMAKE_SOURCE_DIR}/src/backend/plugin_sorter.h"
                "${CMAKE_SOURCE_DIR}/src/backend/error.h")

set (LOOT_GUI_SRC ${LOOT_SRC}
                  "${CMAKE_SOURCE_DIR}/src/gui/main_win.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/handler.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_handler.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_app.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_state.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/scheme.cpp"
                  "${CMAKE_SOURCE_DIR}/src/resource.rc")

set (LOOT_GUI_HEADERS ${LOOT_HEADERS}
                  # Code the API doesn't need.
                  "${CMAKE_SOURCE_DIR}/src/backend/helpers/json.h"
                  # Actual GUI code.
                  "${CMAKE_SOURCE_DIR}/src/gui/handler.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_handler.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_app.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_state.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/scheme.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/resource.h")

set (LOOT_API_SRC ${LOOT_SRC}
                  "${CMAKE_SOURCE_DIR}/src/api/api.cpp")

set (LOOT_API_HEADERS ${LOOT_HEADERS}
                  "${CMAKE_SOURCE_DIR}/src/api/api.h")

set (LOOT_VALIDATOR_SRC ${LOOT_SRC}
                        "${CMAKE_SOURCE_DIR}/src/validator/main.cpp")

set (LOOT_VALIDATOR_HEADERS ${LOOT_HEADERS})

set (LOOT_TESTS_SRC "${CMAKE_SOURCE_DIR}/src/tests/main.cpp")

set (LOOT_TESTS_HEADERS "${CMAKE_SOURCE_DIR}/src/tests/fixtures.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/printers.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/test_api.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/game/test_game.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/game/test_game_settings.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/game/test_load_order_handler.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/helpers/test_git_helper.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/helpers/test_helpers.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/helpers/test_language.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/helpers/test_version.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/helpers/test_yaml_set_helpers.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_condition_grammar.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_conditional_metadata.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_file.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_formid.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_location.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_message.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_message_content.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_plugin_dirty_info.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_plugin_metadata.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/metadata/test_tag.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/plugin/test_plugin.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/plugin/test_plugin_loader.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/test_metadata_list.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/test_masterlist.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/backend/test_plugin_sorter.h")

source_group("Header Files" FILES ${LOOT_HEADERS} ${LOOT_GUI_HEADERS} ${LOOT_API_HEADERS} ${LOOT_TESTS_HEADERS})

# Include source and library directories.
include_directories ("${CMAKE_SOURCE_DIR}/src"
                     "${LIBLOADORDER_ROOT}/src"
                     "${LIBGIT2_ROOT}/include"
                     ${CEF_ROOT}
                     ${LIBESPM_ROOT}
                     ${Boost_INCLUDE_DIRS}
					 ${YAML_CPP_INCLUDE_DIR}
                     ${GTEST_INCLUDE_DIRS})

link_directories ("${LIBLOADORDER_ROOT}/build"
                  "${LIBGIT2_ROOT}/build"
                  "${CEF_ROOT}/Release"
                  "${CEF_ROOT}/build/libcef_dll/Release")


##############################
# System-Specific Settings
##############################

# If building on Travis, set a preprocessor definition.
IF ($ENV{TRAVIS})
	add_definitions (-DTRAVIS)
ENDIF ()

# Settings when compiling for Windows. Since it's a Windows-only app this is always true, but useful to check for copy/paste into other projects.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions (-DUNICODE -D_UNICODE -DLIBLO_STATIC)
	set (LOOT_API_SRC ${LOOT_API_SRC} "${CMAKE_SOURCE_DIR}/src/resource.rc")
ENDIF ()

# GCC and MinGW settings.
IF (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_C_FLAGS "-m${PROJECT_ARCH} -O3 -std=c++11")
    set (CMAKE_CXX_FLAGS "-m${PROJECT_ARCH} -O3 -std=c++11")
    IF (PROJECT_STATIC_RUNTIME)
        set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
        set (CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
        set (CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    ENDIF ()

    set (LOOT_LIBS git2
                   ssl
                   curl
                   loadorder${PROJECT_ARCH})
ENDIF ()

# MinGW settings.
IF (MINGW)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,windows")
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--subsystem,windows")
    set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--subsystem,windows")

    set (LOOT_LIBS ${LOOT_LIBS}
                   version
                   ws2_32
                   shlwapi)
    set (LOOT_GUI_LIBS ${LOOT_LIBS}
                       cef_sandbox
                       libcef
                       libcef_dll_wrapper
                       rpcrt4
                       comctl32)
ELSEIF (MSVC)
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

    set (LOOT_LIBS  git2
                    version
                    loadorder${PROJECT_ARCH}
                    ws2_32
                    shlwapi
                    winhttp
                    crypt32
                    Rpcrt4)
    set (LOOT_GUI_LIBS ${LOOT_LIBS}
                       cef_sandbox
                       libcef
                       libcef_dll_wrapper
                       rpcrt4
                       comctl32)
ENDIF ()


##############################
# Define Targets
##############################

# Build API.
add_library           (loot${PROJECT_ARCH} ${LOOT_API_SRC} ${LOOT_API_HEADERS})
target_link_libraries (loot${PROJECT_ARCH} ${Boost_LIBRARIES} ${YAML_CPP_LIBRARIES} ${LOOT_LIBS})

IF (${GTEST_FOUND})
    # Build tests.
    add_executable(tests ${LOOT_TESTS_SRC} ${LOOT_SRC} ${LOOT_TESTS_HEADERS})
    target_link_libraries(tests loot${PROJECT_ARCH} ${Boost_LIBRARIES} ${YAML_CPP_LIBRARIES} ${LOOT_LIBS} ${GTEST_BOTH_LIBRARIES})
ENDIF ()

# Build application.
add_executable        (LOOT ${LOOT_GUI_SRC} ${LOOT_GUI_HEADERS})
target_link_libraries (LOOT ${Boost_LIBRARIES} ${YAML_CPP_LIBRARIES} ${LOOT_GUI_LIBS})

# Build validator.
add_executable        (metadata-validator ${LOOT_VALIDATOR_SRC} ${LOOT_VALIDATOR_HEADERS})
target_link_libraries (metadata-validator ${Boost_LIBRARIES} ${YAML_CPP_LIBRARIES} ${LOOT_LIBS})



##############################
# Set Target-Specific Flags
##############################

IF (MSVC)
    set (LOOT_LINK_FLAGS "/SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE")
    get_target_property (EXISTING_LINK_FLAGS LOOT LINK_FLAGS)
    IF (EXISTING_LINK_FLAGS)
        set (LOOT_LINK_FLAGS "${EXISTING_LINK_FLAGS} ${LOOT_LINK_FLAGS}")
    ENDIF ()
    set_target_properties (LOOT PROPERTIES LINK_FLAGS ${LOOT_LINK_FLAGS})
ENDIF ()


IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    IF (BUILD_SHARED_LIBS)
        set_target_properties (loot${PROJECT_ARCH} PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} LOOT_EXPORT")
    ELSE ()
        set_target_properties (loot${PROJECT_ARCH} PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} LOOT_STATIC")
    ENDIF ()
ENDIF ()


##############################
# Post-Build Steps
##############################

add_custom_command(
    TARGET LOOT
    POST_BUILD
    COMMAND "mt.exe" -manifest \"${CMAKE_SOURCE_DIR}/src/gui/LOOT.exe.manifest\" -inputresource:"$(TargetDir)$(TargetFileName)"\;\#1 -outputresource:"$(TargetDir)$(TargetFileName)"\;\#1
    COMMENT "Adding manifest..."
)

# Copy CEF DLLs.
FOREACH(cef_dll d3dcompiler_47.dll libEGL.dll libGLESv2.dll libcef.dll wow_helper.exe)
    add_custom_command(TARGET LOOT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/${CEF_ROOT}/$<CONFIGURATION>/${cef_dll}"
            $<TARGET_FILE_DIR:LOOT>)
ENDFOREACH()

# Copy CEF resources.
FOREACH(cef_resource cef.pak cef_100_percent.pak cef_200_percent.pak devtools_resources.pak icudtl.dat)
    add_custom_command(TARGET LOOT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/${CEF_ROOT}/Resources/${cef_resource}"
            $<TARGET_FILE_DIR:LOOT>)
ENDFOREACH()

# Copy CEF locale resources. This is just to reduce error messages while debugging.
FOREACH(cef_resource en-US.pak)
    add_custom_command(TARGET LOOT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/${CEF_ROOT}/Resources/locales/${cef_resource}"
            $<TARGET_FILE_DIR:LOOT>/resources/l10n)
ENDFOREACH()
