cmake_minimum_required (VERSION 2.8.12.1)
cmake_policy(SET CMP0015 NEW)
project (LOOT)
include(ExternalProject)

option(MSVC_STATIC_RUNTIME "Build with static runtime libs (/MT)" OFF)

IF (${MSVC_STATIC_RUNTIME})
    set (MSVC_SHARED_RUNTIME OFF)
ELSE()
    set (MSVC_SHARED_RUNTIME ON)
ENDIF()

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

find_package(Boost REQUIRED COMPONENTS atomic log log_setup regex locale thread date_time chrono filesystem system iostreams)

ExternalProject_Add(GTest
                    PREFIX "external"
                    URL "https://github.com/google/googletest/archive/release-1.8.0.tar.gz"
                    CMAKE_ARGS -Dgtest_force_shared_crt=${MSVC_SHARED_RUNTIME}
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(GTest SOURCE_DIR BINARY_DIR)
set (GTEST_INCLUDE_DIRS "${SOURCE_DIR}/googletest/include")
set (GTEST_LIBRARIES "${BINARY_DIR}/googlemock/gtest/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(LOOT_API_URL "https://github.com/loot/loot-api/releases/download/0.11.0/loot_api-0.11.0-0-g8864f78_dev-win32.7z")
else()
    set(LOOT_API_URL "https://github.com/loot/loot-api/releases/download/0.11.0/loot-api.tar.xz")
endif()

ExternalProject_Add(loot_api
                    PREFIX "external"
                    URL ${LOOT_API_URL}
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(loot_api SOURCE_DIR)
set(LOOT_API_EXTRACTED_PATH ${SOURCE_DIR})

include_directories("${CMAKE_SOURCE_DIR}/src"
                    "${LOOT_API_EXTRACTED_PATH}/include")
link_directories(${LOOT_API_EXTRACTED_PATH})
set(LOOT_API_STATIC_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}loot_api${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(LOOT_API_SHARED_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}loot_api${CMAKE_SHARED_LIBRARY_SUFFIX}")

ExternalProject_Add(testing-metadata
                    PREFIX "external"
                    URL "https://github.com/loot/testing-metadata/archive/1.2.0.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")

ExternalProject_Add(testing-plugins
                    PREFIX "external"
                    URL "https://github.com/WrinklyNinja/testing-plugins/archive/1.0.0.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")

ExternalProject_Add(yaml-cpp
                    PREFIX "external"
                    URL "https://github.com/WrinklyNinja/yaml-cpp/archive/patched-for-loot.tar.gz"
                    CMAKE_ARGS -DMSVC_SHARED_RT=${MSVC_SHARED_RUNTIME}
                    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target yaml-cpp --config $(CONFIGURATION)
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(yaml-cpp SOURCE_DIR BINARY_DIR)
set(YAML_CPP_INCLUDE_DIRS "${SOURCE_DIR}/include")
set(YAML_CPP_LIBRARIES "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/libyaml-cpp${CMAKE_STATIC_LIBRARY_SUFFIX}")

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CEF_URL "http://opensource.spotify.com/cefbuilds/cef_binary_3.3029.1619.geeeb5d7_windows32_minimal.tar.bz2")
    set(CEF_URL_HASH "33c4c9c9d6d1242123eea04db080ee66ba449851")
    set(CEF_PRECOMPILED_BINARIES "libcef.dll" "natives_blob.bin" "snapshot_blob.bin" "d3dcompiler_47.dll" "libEGL.dll" "libGLESv2.dll" "chrome_elf.dll")
    set(CEF_LIB libcef${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSE ()
    set(CEF_URL "http://opensource.spotify.com/cefbuilds/cef_binary_3.3029.1619.geeeb5d7_linux64_minimal.tar.bz2")
    set(CEF_URL_HASH "f809ce65b0b532fcbba6cec5f0e60f731fd3cbbd")
    set(CEF_PRECOMPILED_BINARIES "libcef.so" "natives_blob.bin" "snapshot_blob.bin" "chrome-sandbox")
    set(CEF_LIB libcef${CMAKE_SHARED_LIBRARY_SUFFIX})
ENDIF()

ExternalProject_Add(cef
                    PREFIX "external"
                    URL ${CEF_URL}
                    URL_HASH SHA1=${CEF_URL_HASH}
                    PATCH_COMMAND ${CMAKE_COMMAND} -DMSVC_STATIC_RUNTIME=${MSVC_STATIC_RUNTIME} -P "${CMAKE_SOURCE_DIR}/scripts/cmake/configure_cef.cmake"
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


##############################
# General Settings
##############################

set (LOOT_GUI_SRC "${CMAKE_BINARY_DIR}/generated/version.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/main.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/helpers.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_handler.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_app.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_scheme_handler_factory.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/query_handler.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game_settings.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/resource.rc"
                  "${CMAKE_SOURCE_DIR}/src/gui/window_delegate.cpp")

set (LOOT_GUI_HEADERS "${CMAKE_SOURCE_DIR}/src/gui/editor_message.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/helpers.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_handler.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_app.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/loot_scheme_handler_factory.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/json.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/apply_sort_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/cancel_find_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/cancel_sort_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/change_game_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/clear_all_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/clear_plugin_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/clipboard_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/close_settings_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/copy_content_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/copy_load_order_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/copy_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/discard_unapplied_changes_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/editor_opened_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/editor_closed_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/get_conflicting_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/get_game_data_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/get_game_types_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/get_init_errors_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/get_installed_games_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/get_languages_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/get_settings_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/get_version_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/open_log_location_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/open_readme_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/redate_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/save_filter_state_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/sort_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/update_masterlist_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query_handler.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game_detection_error.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game_settings.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/resource.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/version.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/window_delegate.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/yaml_simple_message_helpers.h")

set(LOOT_GUI_TESTS_SRC "${CMAKE_BINARY_DIR}/generated/version.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/game.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/game_settings.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.cpp"
                       "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.cpp"
                       "${CMAKE_SOURCE_DIR}/src/tests/gui/main.cpp")

set (LOOT_GUI_TESTS_HEADERS "${CMAKE_SOURCE_DIR}/src/gui/state/game.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/game_settings.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
                            "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game_settings_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_paths_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_settings_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_state_test.h")

source_group("Header Files\\gui" FILES ${LOOT_GUI_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_TESTS_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_GUI_TESTS_HEADERS})

source_group("Source Files\\gui" FILES ${LOOT_GUI_SRC})
source_group("Source Files\\tests" FILES ${LOOT_TESTS_SRC})
source_group("Header Files\\tests" FILES ${LOOT_GUI_TESTS_SRC})

# Include source and library directories.
include_directories ("${CMAKE_SOURCE_DIR}/src"
                     ${LIBLOADORDER_INCLUDE_DIRS}
                     ${LIBGIT2_INCLUDE_DIRS}
                     ${CEF_INCLUDE_DIRS}
                     ${LIBESPM_INCLUDE_DIRS}
                     ${Boost_INCLUDE_DIRS}
                     ${YAML_CPP_INCLUDE_DIRS}
                     ${GTEST_INCLUDE_DIRS}
                     ${PSEUDOSEM_INCLUDE_DIRS})

##############################
# System-Specific Settings
##############################

# Settings when compiling for Windows.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions (-DUNICODE -D_UNICODE -DLIBLO_STATIC)
    set(LOOT_API_LINK_LIBRARY ${LOOT_API_STATIC_LIBRARY})
else()
    set(LOOT_API_LINK_LIBRARY ${LOOT_API_SHARED_LIBRARY})
endif()

IF (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};.")
    set (CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c++14")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14")

    set (LOOT_GUI_LIBS X11)
ENDIF ()

IF (MSVC)
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
add_dependencies     (LOOT cef loot_api yaml-cpp)
target_link_libraries(LOOT ${Boost_LIBRARIES} ${CEF_LIBRARIES} ${LOOT_API_LINK_LIBRARY} ${LOOT_GUI_LIBS} ${YAML_CPP_LIBRARIES})

# Build application tests.
add_executable       (loot_gui_tests ${LOOT_GUI_TESTS_SRC} ${LOOT_GUI_TESTS_HEADERS})
add_dependencies     (loot_gui_tests loot_api yaml-cpp GTest testing-metadata testing-plugins)
target_link_libraries(loot_gui_tests ${Boost_LIBRARIES} ${LOOT_API_LINK_LIBRARY} ${GTEST_LIBRARIES} ${YAML_CPP_LIBRARIES})

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
        "${LOOT_API_EXTRACTED_PATH}/${LOOT_API_SHARED_LIBRARY}"
        "$<TARGET_FILE_DIR:LOOT>/${LOOT_API_SHARED_LIBRARY}")
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LOOT_API_EXTRACTED_PATH}/${LOOT_API_SHARED_LIBRARY}"
        "$<TARGET_FILE_DIR:loot_gui_tests>/${LOOT_API_SHARED_LIBRARY}")

# Build the UI HTML.
add_custom_command(TARGET LOOT POST_BUILD
    COMMAND "node" "${CMAKE_SOURCE_DIR}/scripts/build_ui.js"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Copy testing metadata
ExternalProject_Get_Property(testing-metadata SOURCE_DIR)
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        "$<TARGET_FILE_DIR:loot_gui_tests>/testing-metadata")

# Copy testing plugins
ExternalProject_Get_Property(testing-plugins SOURCE_DIR)
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        $<TARGET_FILE_DIR:loot_gui_tests>)
