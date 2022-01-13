cmake_minimum_required (VERSION 3.16)
cmake_policy(SET CMP0015 NEW)
project (LOOT)
include(ExternalProject)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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
set (Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost REQUIRED COMPONENTS locale system)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_package(ICU REQUIRED COMPONENTS uc)
    include_directories($ICU_INCLUDE_DIRS)
    link_directories($ICU_LIBRARY_DIRS)
endif()

find_package(Qt6 COMPONENTS Widgets Network)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
endif()

ExternalProject_Add(GTest
                    PREFIX "external"
                    URL "https://github.com/google/googletest/archive/release-1.11.0.tar.gz"
                    CMAKE_ARGS -Dgtest_force_shared_crt=ON
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(GTest SOURCE_DIR BINARY_DIR)
set (GTEST_INCLUDE_DIRS "${SOURCE_DIR}/googletest/include")
set (GTEST_LIBRARIES "${BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}")

if (NOT DEFINED LIBLOOT_URL)
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        if ("${CMAKE_GENERATOR_PLATFORM}" MATCHES "Win32")
            set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.17.3/libloot-0.17.3-0-g27d8c9b_0.17.3-win32.7z")
        else()
            set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.17.3/libloot-0.17.3-0-g27d8c9b_0.17.3-win64.7z")
        endif()
    else()
        set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.17.3/libloot-0.17.3-0-g27d8c9b_0.17.3-Linux.tar.xz")
    endif()
endif()

ExternalProject_Add(json
                    PREFIX "external"
                    URL "https://github.com/nlohmann/json/releases/download/v3.10.4/json.hpp"
                    URL_HASH SHA256=c9ac7589260f36ea7016d4d51a6c95809803298c7caec9f55830a0214c5f9140
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
                    URL "https://github.com/Ortham/testing-plugins/archive/1.4.1.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    if ("${CMAKE_GENERATOR_PLATFORM}" MATCHES "Win32")
        set(CEF_URL "https://cef-builds.spotifycdn.com/cef_binary_90.6.5%2Bg7a604aa%2Bchromium-90.0.4430.93_windows32_minimal.tar.bz2")
        set(CEF_URL_HASH "71a8a4a643d28a9711574ffdf0e96c6b6fee9be3")
        set(CEF_PRECOMPILED_BINARIES "libcef.dll" "snapshot_blob.bin" "v8_context_snapshot.bin" "d3dcompiler_47.dll" "libEGL.dll" "libGLESv2.dll" "chrome_elf.dll")
        set(CEF_LIB libcef${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        set(CEF_URL "https://cef-builds.spotifycdn.com/cef_binary_90.6.5%2Bg7a604aa%2Bchromium-90.0.4430.93_windows64_minimal.tar.bz2")
        set(CEF_URL_HASH "ff215765fb431bc70eb4058a728c5bfa914f60b3")
        set(CEF_PRECOMPILED_BINARIES "libcef.dll" "snapshot_blob.bin" "v8_context_snapshot.bin" "d3dcompiler_47.dll" "libEGL.dll" "libGLESv2.dll" "chrome_elf.dll")
        set(CEF_LIB libcef${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
ELSE ()
    set(CEF_URL "https://cef-builds.spotifycdn.com/cef_binary_90.6.5%2Bg7a604aa%2Bchromium-90.0.4430.93_linux64_minimal.tar.bz2")
    set(CEF_URL_HASH "c81f998e45d2abbdfb4fb138f0f0593baac1ecf7")
    set(CEF_PRECOMPILED_BINARIES "libcef.so" "snapshot_blob.bin" "v8_context_snapshot.bin" "chrome-sandbox")
    set(CEF_LIB libcef${CMAKE_SHARED_LIBRARY_SUFFIX})
ENDIF()


IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CEF_BUILD_COMMAND, "${CMAKE_COMMAND} --build . --target libcef_dll_wrapper --config $(CONFIGURATION)")
ELSE ()
    set(CEF_BUILD_COMMAND, "${CMAKE_COMMAND} --build . --target libcef_dll_wrapper")
ENDIF()
ExternalProject_Add(cef
                    PREFIX "external"
                    URL ${CEF_URL}
                    URL_HASH SHA1=${CEF_URL_HASH}
                    CMAKE_ARGS -DUSE_SANDBOX=OFF -DCEF_RUNTIME_LIBRARY_FLAG=/MD
                    BUILD_COMMAND ${CEF_BUILD_COMMAND}
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(cef SOURCE_DIR BINARY_DIR)
set(CEF_INCLUDE_DIRS "${SOURCE_DIR}")
set(CEF_LIBRARIES "${SOURCE_DIR}/Release/${CEF_LIB}"
                  "${BINARY_DIR}/libcef_dll_wrapper/${CMAKE_CFG_INTDIR}/libcef_dll_wrapper${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(CEF_RESOURCES_DIR "${SOURCE_DIR}/Resources")
set(CEF_RESOURCES "chrome_100_percent.pak"
                  "chrome_200_percent.pak"
                  "icudtl.dat"
                  "resources.pak")
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
                    URL "https://github.com/gabime/spdlog/archive/v1.9.2.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(spdlog SOURCE_DIR)
set(SPDLOG_INCLUDE_DIRS "${SOURCE_DIR}/include")

##############################
# General Settings
##############################

set (LOOT_QT_GUI_SRC "${CMAKE_BINARY_DIR}/generated/version.cpp"
                "${CMAKE_SOURCE_DIR}/resources/resources.qrc"
                "${CMAKE_SOURCE_DIR}/src/gui/helpers.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/counters.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/filters_states.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/filters_widget.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/general_info.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/general_info_card.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/edge.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/groups_editor_dialog.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/graph_view.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/node.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/helpers.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/icon_factory.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/main.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/main_window.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/messages_widget.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_card.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/delegates.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/group_tab.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/message_content_editor.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/cleaning_data_table_model.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/file_table_model.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/location_table_model.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/message_content_table_model.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/message_table_model.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/tag_table_model.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/plugin_editor_widget.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/table_tabs.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/plugin_item.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_item_model.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_item_filter_model.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/search_dialog.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/game_tab.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/general_tab.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/new_game_dialog.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/settings_dialog.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/sidebar_plugin_name_delegate.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/qt/style.cpp"
                "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/logging.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.cpp"
                  "${CMAKE_SOURCE_DIR}/src/gui/resource.rc")

set (LOOT_QT_GUI_HEADERS "${CMAKE_SOURCE_DIR}/src/gui/application_mutex.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/helpers.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/counters.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/filters_states.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/filters_widget.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/general_info.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/general_info_card.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/edge.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/groups_editor_dialog.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/graph_view.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/node.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/helpers.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/icon_factory.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/main_window.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/messages_widget.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_card.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/delegates.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/group_tab.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/message_content_editor.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/cleaning_data_table_model.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/file_table_model.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/location_table_model.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/message_content_table_model.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/message_table_model.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/tag_table_model.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/plugin_editor_widget.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/table_tabs.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/plugin_item.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_item_model.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_item_filter_model.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/query_worker_thread.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/search_dialog.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/game_tab.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/general_tab.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/new_game_dialog.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/settings_dialog.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/sidebar_plugin_name_delegate.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/qt/style.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_detection_error.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/games_manager.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/logging.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/unapplied_change_counter.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/resource.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/version.h")

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

set (LOOT_GUI_HEADERS "${CMAKE_SOURCE_DIR}/src/gui/application_mutex.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/helpers.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_handler.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_app.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/loot_scheme_handler_factory.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/window_delegate.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/derived_plugin_metadata.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/json.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/query_executor.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/apply_sort_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/cancel_sort_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/change_game_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/clear_all_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/clear_plugin_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/close_settings_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/copy_content_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/copy_load_order_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/copy_metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/discard_unapplied_changes_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/editor_opened_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/editor_closed_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_auto_sort_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_conflicting_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_game_data_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_game_types_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_init_errors_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_installed_games_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_prelude_info_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_settings_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_themes_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_version_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/metadata_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/open_log_location_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/open_readme_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/redate_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/save_filter_state_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/save_user_groups_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/sort_plugins_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/update_masterlist_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/query/types/update_prelude_query.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/cef/query/query_handler.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_detection_error.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.h"
                  "${CMAKE_SOURCE_DIR}/src/gui/state/game/games_manager.h"
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
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/query/json_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/query/types/close_settings_query_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/query/types/editor_closed_query_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/query/types/get_prelude_info_query_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/query/types/get_settings_query_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/query/types/get_themes_query_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/game_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/game_settings_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/games_manager_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/helpers_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_paths_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_settings_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/state/unapplied_change_counter_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/helpers_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/gui/test_helpers.h")

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
    add_definitions (-DUNICODE -D_UNICODE -DNOMINMAX -DBOOST_UUID_FORCE_AUTO_LINK)
    set(LIBLOOT_LINK_LIBRARY ${LIBLOOT_STATIC_LIBRARY})
else()
    set(LIBLOOT_LINK_LIBRARY ${LIBLOOT_SHARED_LIBRARY})
endif()

IF (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};.")
    set (CMAKE_BUILD_WITH_INSTALL_RPATH ON)

    set (LOOT_LIBS pthread http_parser ssh2 stdc++fs icui18n)
    set (LOOT_GUI_LIBS X11 ${LOOT_LIBS})
    set (LOOT_TEST_LIBS ${LOOT_LIBS})
ENDIF ()

IF (MSVC)
    list(APPEND LOOT_QT_GUI_SRC "${CMAKE_SOURCE_DIR}/resources/resources.windows.qrc")

    # Turn off permissive mode to be more standards-compliant and avoid compiler errors.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")

    # Set /bigobj to allow building Debug tests
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")

    set (LOOT_GUI_LIBS comctl32
                       Psapi)
ENDIF ()

##############################
# Define Targets
##############################

# Build application.
add_executable       (LOOT ${LOOT_GUI_SRC} ${LOOT_GUI_HEADERS})
add_dependencies     (LOOT cef cpptoml json libloot spdlog)
target_link_libraries(LOOT ${Boost_LIBRARIES} ${CEF_LIBRARIES} ${LIBLOOT_LINK_LIBRARY} ${LOOT_GUI_LIBS} ${ICU_LIBRARIES})

# Build Qt application.
add_executable(LOOT_qt ${LOOT_QT_GUI_SRC}  ${LOOT_QT_GUI_HEADERS})
add_dependencies     (LOOT_qt cpptoml json libloot spdlog)
target_link_libraries(LOOT_qt PRIVATE Qt::Widgets Qt::Network ${Boost_LIBRARIES} ${LIBLOOT_LINK_LIBRARY} ${LOOT_GUI_LIBS} ${ICU_LIBRARIES})

# Build application tests.
add_executable       (loot_gui_tests ${LOOT_GUI_TESTS_SRC} ${LOOT_GUI_TESTS_HEADERS})
add_dependencies     (loot_gui_tests cpptoml json libloot spdlog GTest testing-plugins)
target_link_libraries(loot_gui_tests ${Boost_LIBRARIES} ${LIBLOOT_LINK_LIBRARY} ${GTEST_LIBRARIES} ${LOOT_TEST_LIBS} ${ICU_LIBRARIES})

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

    set_target_properties(LOOT_qt PROPERTIES WIN32_EXECUTABLE TRUE)
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

    IF (Qt6_FOUND)
        set(Qt_DIR ${Qt6_DIR})
    ELSE()
        set(Qt_DIR ${Qt5_DIR})
    ENDIF()

    # Copy Qt binaries and resources.
    add_custom_command(TARGET LOOT_qt POST_BUILD
        COMMAND ${Qt_DIR}/../../../bin/windeployqt --release $<TARGET_FILE:LOOT_qt>
        COMMENT "Running windeployqt...")
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
    COMMAND npm run build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Copy testing plugins
ExternalProject_Get_Property(testing-plugins SOURCE_DIR)
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        $<TARGET_FILE_DIR:loot_gui_tests>)
