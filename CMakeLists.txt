cmake_minimum_required (VERSION 3.6)
cmake_policy(SET CMP0015 NEW)
project (LOOT)
include(ExternalProject)

option(MSVC_STATIC_RUNTIME "Build with static runtime libs (/MT)" OFF)

IF (${MSVC_STATIC_RUNTIME})
    set (MSVC_SHARED_RUNTIME OFF)
ELSE()
    set (MSVC_SHARED_RUNTIME ON)
ENDIF()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##############################
# Get Build Revision
##############################

find_package(Git)

IF (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE GIT_COMMIT_STRING
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
ELSE()
    SET (GIT_COMMIT_STRING "unknown")
ENDIF ()

message(STATUS "Git revision: ${GIT_COMMIT_STRING}")

# Write to file.
configure_file("${CMAKE_SOURCE_DIR}/src/gui/version.cpp.in" "${CMAKE_BINARY_DIR}/generated/version.cpp" @ONLY)
include_directories("${CMAKE_BINARY_DIR}/generated")

##############################
# External Projects
##############################

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)

IF (MSVC)
    set (Boost_USE_STATIC_RUNTIME ${MSVC_STATIC_RUNTIME})
ELSE()
    set (Boost_USE_STATIC_RUNTIME OFF)
ENDIF()

IF (NOT Boost_USE_STATIC_LIBS)
    add_definitions(-DBOOST_LOG_DYN_LINK)
ENDIF ()

find_package(Boost REQUIRED COMPONENTS locale system)

ExternalProject_Add(GTest
                    PREFIX "external"
                    URL "https://github.com/google/googletest/archive/release-1.8.1.tar.gz"
                    CMAKE_ARGS -Dgtest_force_shared_crt=${MSVC_SHARED_RUNTIME}
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(GTest SOURCE_DIR BINARY_DIR)
set (GTEST_INCLUDE_DIRS "${SOURCE_DIR}/googletest/include")
set (GTEST_LIBRARIES "${BINARY_DIR}/googlemock/gtest/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.14.4/libloot-0.14.4-0-gda80a78_dev-win32.7z")
else()
    set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.14.4/libloot.tar.xz")
endif()

ExternalProject_Add(json
                    PREFIX "external"
                    URL "https://github.com/nlohmann/json/releases/download/v3.5.0/json.hpp"
                    DOWNLOAD_NO_EXTRACT 1
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
set(JSON_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/external/src")

ExternalProject_Add(libloot
                    PREFIX "external"
                    URL ${LIBLOOT_URL}
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(libloot SOURCE_DIR)
set(LIBLOOT_EXTRACTED_PATH ${SOURCE_DIR})

include_directories("${CMAKE_SOURCE_DIR}/src"
                    "${LIBLOOT_EXTRACTED_PATH}/include")
link_directories(${LIBLOOT_EXTRACTED_PATH})
set(LIBLOOT_STATIC_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}loot${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(LIBLOOT_SHARED_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}loot${CMAKE_SHARED_LIBRARY_SUFFIX}")

ExternalProject_Add(testing-plugins
                    PREFIX "external"
                    URL "https://github.com/Ortham/testing-plugins/archive/1.0.0.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CEF_URL "http://opensource.spotify.com/cefbuilds/cef_binary_3.3440.1806.g65046b7_windows32_minimal.tar.bz2")
    set(CEF_URL_HASH "cffc227946871d6592f8458daa971899559c133e")
    set(CEF_PRECOMPILED_BINARIES "libcef.dll" "natives_blob.bin" "snapshot_blob.bin" "v8_context_snapshot.bin" "d3dcompiler_47.dll" "libEGL.dll" "libGLESv2.dll" "chrome_elf.dll")
    set(CEF_LIB libcef${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSE ()
    set(CEF_URL "http://opensource.spotify.com/cefbuilds/cef_binary_3.3440.1806.g65046b7_linux64_minimal.tar.bz2")
    set(CEF_URL_HASH "defe5a9dde8c5616a193e40d1bc0281b2e7e7529")
    set(CEF_PRECOMPILED_BINARIES "libcef.so" "natives_blob.bin" "snapshot_blob.bin" "v8_context_snapshot.bin" "chrome-sandbox")
    set(CEF_LIB libcef${CMAKE_SHARED_LIBRARY_SUFFIX})
ENDIF()

ExternalProject_Add(cef
                    PREFIX "external"
                    URL ${CEF_URL}
                    URL_HASH SHA1=${CEF_URL_HASH}
                    CMAKE_ARGS -DUSE_SANDBOX=OFF
                    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target libcef_dll_wrapper --config $(CONFIGURATION)
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(cef SOURCE_DIR BINARY_DIR)
set(CEF_INCLUDE_DIRS "${SOURCE_DIR}")
set(CEF_LIBRARIES "${SOURCE_DIR}/Release/${CEF_LIB}"
                  "${BINARY_DIR}/libcef_dll_wrapper/${CMAKE_CFG_INTDIR}/libcef_dll_wrapper${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(CEF_RESOURCES_DIR "${SOURCE_DIR}/Resources")
set(CEF_RESOURCES "cef.pak"
                  "cef_100_percent.pak"
                  "cef_200_percent.pak"
                  "devtools_resources.pak"
                  "icudtl.dat")
set(CEF_LOCALES_DIR "${CEF_RESOURCES_DIR}/locales")
set(CEF_LOCALE_FILE "en-US.pak")
set(CEF_PRECOMPILED_BINARIES_DIR "${SOURCE_DIR}/Release")

ExternalProject_Add(cpptoml
                    PREFIX "external"
                    URL "https://github.com/skystrife/cpptoml/archive/v0.1.1.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(cpptoml SOURCE_DIR)
set(CPPTOML_INCLUDE_DIRS "${SOURCE_DIR}/include")

ExternalProject_Add(spdlog
                    PREFIX "external"
                    URL "https://github.com/gabime/spdlog/archive/v1.3.0.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(spdlog SOURCE_DIR)
set(SPDLOG_INCLUDE_DIRS "${SOURCE_DIR}/include")

##############################
# General Settings
##############################

set (LOOT_GUI_SRC "${CMAKE_BINARY_DIR}/generated/version.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/main.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/helpers.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_handler.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_app.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_scheme_handler_factory.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/window_delegate.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/query_handler.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/logging.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/resource.rc")

set (LOOT_GUI_HEADERS "${CMAKE_SOURCE_DIR}/src/gui/helpers.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_handler.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_app.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_scheme_handler_factory.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/window_delegate.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/derived_plugin_metadata.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/json.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/apply_sort_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/cancel_find_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/cancel_sort_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/change_game_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/clear_all_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/clear_plugin_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/clipboard_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/close_settings_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/copy_content_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/copy_load_order_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/copy_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/discard_unapplied_changes_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/editor_opened_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/editor_closed_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_auto_sort_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_conflicting_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_game_data_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_game_types_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_init_errors_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_installed_games_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_languages_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_settings_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/get_version_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/open_log_location_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/open_readme_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/redate_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/save_filter_state_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/save_user_groups_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/sort_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/types/update_masterlist_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/query_handler.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_detection_error.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/logging.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/unapplied_change_counter.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/resource.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/version.h")

set(LOOT_GUI_TESTS_SRC "${CMAKE_BINARY_DIR}/generated/version.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/helpers.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/logging.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.cpp"
                       "${CMAKE_SOURCE_DIR}/src/tests/gui/main.cpp")

set (LOOT_GUI_TESTS_HEADERS "${CMAKE_SOURCE_DIR}/src/gui/helpers.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/cef/query/json_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/game_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/game_settings_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/helpers_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_paths_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_settings_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/unapplied_change_counter_test.h")

source_group("Header Files\\gui" FILES ${LOOT_GUI_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_TESTS_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_GUI_TESTS_HEADERS})

source_group("Source Files\\gui" FILES ${LOOT_GUI_SRC})
source_group("Source Files\\tests" FILES ${LOOT_TESTS_SRC})
source_group("Header Files\\tests" FILES ${LOOT_GUI_TESTS_SRC})

# Include source and library directories.
include_directories ("${CMAKE_SOURCE_DIR}/src"
                     ${CPPTOML_INCLUDE_DIRS}
                     ${CEF_INCLUDE_DIRS}
                     ${JSON_INCLUDE_DIRS}
                     ${Boost_INCLUDE_DIRS}
                     ${GTEST_INCLUDE_DIRS}
                     ${SPDLOG_INCLUDE_DIRS})

##############################
# System-Specific Settings
##############################

# Settings when compiling for Windows.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions (-DUNICODE -D_UNICODE)
    set(LIBLOOT_LINK_LIBRARY ${LIBLOOT_STATIC_LIBRARY})
else()
    set(LIBLOOT_LINK_LIBRARY ${LIBLOOT_SHARED_LIBRARY})
endif()

IF (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};.")
    set (CMAKE_BUILD_WITH_INSTALL_RPATH ON)

    set (LOOT_LIBS pthread http_parser ssh2 stdc++fs)
    set (LOOT_GUI_LIBS X11 ${LOOT_LIBS})
    set (LOOT_TEST_LIBS ${LOOT_LIBS})
ENDIF ()

IF (MSVC)
    # Turn off permissive mode to be more standards-compliant and avoid compiler errors.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")

    # Set /bigobj to allow building Debug tests
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")

    # Update compiler flags.
    IF (MSVC_STATIC_RUNTIME)
        FOREACH(flag
            CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_INIT
            CMAKE_CXX_FLAGS_RELEASE  CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_DEBUG  CMAKE_CXX_FLAGS_DEBUG_INIT)
            STRING(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
            SET("${flag}" "${${flag}} /EHsc")
        ENDFOREACH()
    ENDIF ()

    set (LOOT_GUI_LIBS comctl32
                       Psapi)
ENDIF ()

##############################
# Define Targets
##############################

# Build application.
add_executable       (LOOT ${LOOT_GUI_SRC} ${LOOT_GUI_HEADERS})
add_dependencies     (LOOT cef cpptoml json libloot spdlog)
target_link_libraries(LOOT ${Boost_LIBRARIES} ${CEF_LIBRARIES} ${LIBLOOT_LINK_LIBRARY} ${LOOT_GUI_LIBS})

# Build application tests.
add_executable       (loot_gui_tests ${LOOT_GUI_TESTS_SRC} ${LOOT_GUI_TESTS_HEADERS})
add_dependencies     (loot_gui_tests cpptoml libloot spdlog GTest testing-plugins)
target_link_libraries(loot_gui_tests ${Boost_LIBRARIES} ${LIBLOOT_LINK_LIBRARY} ${GTEST_LIBRARIES} ${LOOT_TEST_LIBS})

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

##############################
# Post-Build Steps
##############################

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_custom_command(
        TARGET LOOT
        POST_BUILD
        COMMAND "mt.exe" -manifest \"${CMAKE_SOURCE_DIR}/src/gui/LOOT.exe.manifest\" -inputresource:"$(TargetDir)$(TargetFileName)"\;\#1 -outputresource:"$(TargetDir)$(TargetFileName)"\;\#1
        COMMENT "Adding manifest..."
    )
ENDIF ()

# Copy CEF binaries.
FOREACH(cef_dll ${CEF_PRECOMPILED_BINARIES})
    add_custom_command(TARGET LOOT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CEF_PRECOMPILED_BINARIES_DIR}/${cef_dll}"
            $<TARGET_FILE_DIR:LOOT>/${cef_dll})
ENDFOREACH()

# Copy CEF resources.
FOREACH(cef_resource ${CEF_RESOURCES})
    add_custom_command(TARGET LOOT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CEF_RESOURCES_DIR}/${cef_resource}"
            $<TARGET_FILE_DIR:LOOT>/${cef_resource})
ENDFOREACH()

# Copy CEF en-US locale resource.
add_custom_command(TARGET LOOT POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CEF_LOCALES_DIR}/${CEF_LOCALE_FILE}"
        $<TARGET_FILE_DIR:LOOT>/resources/l10n/${CEF_LOCALE_FILE})

# Copy the API binary to the build directory.
add_custom_command(TARGET LOOT POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBLOOT_EXTRACTED_PATH}/${LIBLOOT_SHARED_LIBRARY}"
        "$<TARGET_FILE_DIR:LOOT>/${LIBLOOT_SHARED_LIBRARY}")
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBLOOT_EXTRACTED_PATH}/${LIBLOOT_SHARED_LIBRARY}"
        "$<TARGET_FILE_DIR:loot_gui_tests>/${LIBLOOT_SHARED_LIBRARY}")

# Build the UI HTML.
add_custom_command(TARGET LOOT POST_BUILD
    COMMAND yarn build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Copy testing plugins
ExternalProject_Get_Property(testing-plugins SOURCE_DIR)
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        $<TARGET_FILE_DIR:loot_gui_tests>)
