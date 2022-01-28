cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0015 NEW)
project(LOOT)
include(ExternalProject)

option(RUN_CLANG_TIDY "Whether or not to run clang-tidy during build. Has no effect when using CMake's MSVC generator." OFF)

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

if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE GIT_COMMIT_STRING
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(GIT_COMMIT_STRING "unknown")
endif()

message(STATUS "Git revision: ${GIT_COMMIT_STRING}")

# Write to file.
configure_file("${CMAKE_SOURCE_DIR}/src/gui/version.cpp.in"
    "${CMAKE_BINARY_DIR}/generated/version.cpp"
    @ONLY)

##############################
# External Projects
##############################

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost REQUIRED COMPONENTS locale system)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(ICU REQUIRED COMPONENTS uc i18n)
endif()

find_package(Qt6 COMPONENTS Widgets Network Test)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets Network Test REQUIRED)
endif()

ExternalProject_Add(GTest
    PREFIX "external"
    URL "https://github.com/google/googletest/archive/release-1.11.0.tar.gz"
    CMAKE_ARGS -Dgtest_force_shared_crt=ON
    INSTALL_COMMAND "")
ExternalProject_Get_Property(GTest SOURCE_DIR BINARY_DIR)
set(GTEST_INCLUDE_DIRS "${SOURCE_DIR}/googletest/include")
set(GTEST_LIBRARIES "${BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}")

if(NOT DEFINED LIBLOOT_URL)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.17.3/libloot-0.17.3-0-g27d8c9b_0.17.3-win32.7z")
        else()
            set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.17.3/libloot-0.17.3-0-g27d8c9b_0.17.3-win64.7z")
        endif()
    else()
        set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.17.3/libloot-0.17.3-0-g27d8c9b_0.17.3-Linux.tar.xz")
    endif()
endif()

ExternalProject_Add(libloot
    PREFIX "external"
    URL ${LIBLOOT_URL}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")
ExternalProject_Get_Property(libloot SOURCE_DIR)
set(LIBLOOT_INCLUDE_DIRS "${SOURCE_DIR}/include")
set(LIBLOOT_STATIC_LIBRARY "${SOURCE_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}loot${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(LIBLOOT_SHARED_LIBRARY "${SOURCE_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}loot${CMAKE_SHARED_LIBRARY_SUFFIX}")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(ZLIB_LIB_NAME "zlibstatic$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(ZLIB_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

ExternalProject_Add(zlib
    PREFIX "external"
    URL "https://github.com/madler/zlib/archive/refs/tags/v1.2.11.tar.gz"
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE TRUE)
ExternalProject_Get_Property(zlib SOURCE_DIR BINARY_DIR)
set(ZLIB_ROOT ${SOURCE_DIR})
set(ZLIB_LIBRARY
    "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${ZLIB_LIB_NAME}")

ExternalProject_Add(minizip-ng
    PREFIX "external"
    URL "https://github.com/zlib-ng/minizip-ng/archive/refs/tags/3.0.4.tar.gz"
    CMAKE_ARGS
        -DMZ_ZLIB=ON
        -DMZ_BZIP2=OFF
        -DMZ_LZMA=OFF
        -DMZ_ZSTD=OFF
        -DMZ_FETCH_LIBS=OFF
        -DMZ_PKCRYPT=OFF
        -DMZ_WZAES=OFF
        -DMZ_OPENSSL=OFF
        -DMZ_LIBBSD=OFF
        -DMZ_SIGNING=OFF
        -DMZ_COMPRESS_ONLY=ON
        -DZLIB_ROOT=${ZLIB_ROOT}
        -DZLIB_LIBRARY=${ZLIB_LIBRARY}
    INSTALL_COMMAND ""
    DEPENDS zlib)
ExternalProject_Get_Property(minizip-ng SOURCE_DIR BINARY_DIR)
set(MINIZIP_NG_INCLUDE_DIRS "${SOURCE_DIR}")
set(MINIZIP_NG_LIBRARIES
    "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/libminizip${CMAKE_STATIC_LIBRARY_SUFFIX}"
    ${ZLIB_LIBRARY})

ExternalProject_Add(testing-plugins
    PREFIX "external"
    URL "https://github.com/Ortham/testing-plugins/archive/1.4.1.tar.gz"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

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

set(LOOT_SRC_GUI_CPP_FILES
    "${CMAKE_SOURCE_DIR}/src/gui/backup.cpp"
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
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/tasks.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/update_masterlist_task.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/logging.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/resource.rc")

set(LOOT_SRC_GUI_H_FILES
    "${CMAKE_SOURCE_DIR}/src/gui/application_mutex.h"
    "${CMAKE_SOURCE_DIR}/src/gui/backup.h"
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
    "${CMAKE_SOURCE_DIR}/src/gui/qt/search_dialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/game_tab.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/general_tab.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/new_game_dialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/settings_dialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/sidebar_plugin_name_delegate.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/style.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/tasks.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/update_masterlist_task.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/apply_sort_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/cancel_sort_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/change_game_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/clear_all_metadata_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/clear_plugin_metadata_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/copy_load_order_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/copy_metadata_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_conflicting_plugins_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_game_data_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/metadata_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/open_log_location_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/open_readme_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/sort_plugins_query.h"
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

set(LOOT_SRC_TESTS_GUI_CPP_FILES
    "${CMAKE_SOURCE_DIR}/src/tests/gui/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/qt/tasks/non_blocking_test_task.cpp")

set(LOOT_SRC_TESTS_GUI_H_FILES
    "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/game_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/game_settings_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/games_manager_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/helpers_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_paths_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_settings_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/state/unapplied_change_counter_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/qt/helpers_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/qt/tasks/non_blocking_test_task.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/qt/tasks/tasks_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/backup_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/helpers_test.h"
    "${CMAKE_SOURCE_DIR}/src/tests/gui/test_helpers.h")

source_group(TREE "${CMAKE_SOURCE_DIR}/src/gui"
    PREFIX "Header Files"
    FILES ${LOOT_SRC_GUI_H_FILES})

source_group(TREE "${CMAKE_SOURCE_DIR}/src/tests/gui"
    PREFIX "Header Files"
    FILES ${LOOT_SRC_TESTS_GUI_H_FILES})

source_group(TREE "${CMAKE_SOURCE_DIR}/src/gui"
    PREFIX "Source Files"
    FILES ${LOOT_SRC_GUI_CPP_FILES})

source_group(TREE "${CMAKE_SOURCE_DIR}/src/tests/gui"
    PREFIX "Source Files"
    FILES ${LOOT_SRC_TESTS_GUI_CPP_FILES})

set(LOOT_ALL_SOURCES
    ${LOOT_SRC_GUI_CPP_FILES}
    ${LOOT_SRC_GUI_H_FILES}
    "${CMAKE_BINARY_DIR}/generated/version.cpp"
    "${CMAKE_SOURCE_DIR}/resources/resources.qrc")

set(LOOT_GUI_TESTS_ALL_SOURCES
    ${LOOT_SRC_TESTS_GUI_CPP_FILES}
    ${LOOT_SRC_TESTS_GUI_H_FILES}
    "${CMAKE_BINARY_DIR}/generated/version.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/backup.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/plugin_item.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/tasks.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/logging.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/backup.h"
    "${CMAKE_SOURCE_DIR}/src/gui/helpers.h"
    "${CMAKE_SOURCE_DIR}/src/gui/plugin_item.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/helpers.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/tasks.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/games_manager.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/unapplied_change_counter.h")

##############################
# System-Specific Settings
##############################

if(MSVC)
    list(APPEND LOOT_ALL_SOURCES "${CMAKE_SOURCE_DIR}/resources/resources.windows.qrc")
endif()

##############################
# Define Targets
##############################

# Build Qt application.
add_executable(LOOT ${LOOT_ALL_SOURCES})
add_dependencies(LOOT cpptoml libloot minizip-ng spdlog)
target_link_libraries(LOOT PRIVATE
    Qt::Widgets Qt::Network Boost::system Boost::locale ${MINIZIP_NG_LIBRARIES})

# Build application tests.
add_executable(loot_gui_tests ${LOOT_GUI_TESTS_ALL_SOURCES})
add_dependencies(loot_gui_tests
    cpptoml libloot minizip-ng spdlog GTest testing-plugins)
target_link_libraries(loot_gui_tests PRIVATE
    Qt::Core Qt::Network Qt::Test Boost::system Boost::locale ${MINIZIP_NG_LIBRARIES} ${GTEST_LIBRARIES})

##############################
# Set Target-Specific Flags
##############################

# Include source and library directories.
target_include_directories(LOOT PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(LOOT SYSTEM PRIVATE
    ${CPPTOML_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${ICU_INCLUDE_DIRS}
    ${LIBLOOT_INCLUDE_DIRS}
    ${MINIZIP_NG_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS})

target_include_directories(loot_gui_tests PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(loot_gui_tests SYSTEM PRIVATE
    ${CPPTOML_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${ICU_INCLUDE_DIRS}
    ${LIBLOOT_INCLUDE_DIRS}
    ${MINIZIP_NG_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(LOOT PRIVATE UNICODE _UNICODE NOMINMAX)
    target_compile_definitions(loot_gui_tests PRIVATE
        UNICODE _UNICODE NOMINMAX BOOST_UUID_FORCE_AUTO_LINK)

    target_link_libraries(LOOT PRIVATE ${LIBLOOT_STATIC_LIBRARY})
    target_link_libraries(loot_gui_tests PRIVATE ${LIBLOOT_STATIC_LIBRARY})
else()
    target_link_libraries(LOOT PRIVATE
        ${LIBLOOT_SHARED_LIBRARY} ICU::uc ICU::i18n)
    target_link_libraries(loot_gui_tests PRIVATE
        ${LIBLOOT_SHARED_LIBRARY} ICU::uc ICU::i18n)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(LOOT loot_gui_tests
        PROPERTIES
            INSTALL_RPATH "${CMAKE_INSTALL_RPATH};."
            BUILD_WITH_INSTALL_RPATH ON)

    target_compile_options(LOOT PRIVATE "-Wall" "-Wextra")

    set(LOOT_LIBS pthread http_parser ssh2 stdc++fs icui18n)
    target_link_libraries(LOOT PRIVATE X11 ${LOOT_LIBS})
    target_link_libraries(loot_gui_tests PRIVATE ${LOOT_LIBS})
endif()

if(MSVC)
    set_target_properties(LOOT PROPERTIES WIN32_EXECUTABLE TRUE)

    # Turn off permissive mode to be more standards-compliant and avoid compiler errors.
    target_compile_options(LOOT PRIVATE "/permissive-" "/W4")

    # Set /bigobj to allow building Debug tests
    target_compile_options(loot_gui_tests PRIVATE
        "/permissive-" "/W4" "$<$<CONFIG:DEBUG>:/bigobj>")

    target_link_libraries(LOOT PRIVATE comctl32 psapi)
endif()

##############################
# Configure clang-tidy
##############################

if(RUN_CLANG_TIDY)
    set(CLANG_TIDY_COMMON_CHECKS
        "cppcoreguidelines-avoid-c-arrays"
        "cppcoreguidelines-c-copy-assignment-signature"
        "cppcoreguidelines-explicit-virtual-functions"
        "cppcoreguidelines-init-variables"
        "cppcoreguidelines-interfaces-global-init"
        "cppcoreguidelines-macro-usage"
        "cppcoreguidelines-narrowing-conventions"
        "cppcoreguidelines-no-malloc"
        "cppcoreguidelines-pro-bounds-array-to-pointer-decay"
        "cppcoreguidelines-pro-bounds-constant-array-index"
        "cppcoreguidelines-pro-bounds-pointer-arithmetic"
        "cppcoreguidelines-pro-type-const-cast"
        "cppcoreguidelines-pro-type-cstyle-cast"
        "cppcoreguidelines-pro-type-reinterpret-cast"
        "cppcoreguidelines-pro-type-static-cast-downcast"
        "cppcoreguidelines-pro-type-union-access"
        "cppcoreguidelines-pro-type-vararg"
        "cppcoreguidelines-pro-type-slicing")

    set(CLANG_TIDY_APP_CHECKS
        ${CLANG_TIDY_COMMON_CHECKS}
        "cppcoreguidelines-avoid-goto"
        "cppcoreguidelines-avoid-magic-numbers"
        "cppcoreguidelines-non-private-member-variables-in-classes"
        "cppcoreguidelines-special-member-functions")

    # Skip some checks for tests because they're not worth the noise (e.g. GTest
    # happens to use goto, lots of tests use magic numbers as expected values).
    set(CLANG_TIDY_TEST_CHECKS
        ${CLANG_TIDY_COMMON_CHECKS})

    list(JOIN CLANG_TIDY_APP_CHECKS "," CLANG_TIDY_APP_CHECKS_JOINED)
    list(JOIN CLANG_TIDY_TEST_CHECKS "," CLANG_TIDY_TEST_CHECKS_JOINED)

    set(CLANG_TIDY_APP
        clang-tidy "-header-filter=.*" "-checks=${CLANG_TIDY_APP_CHECKS_JOINED}")
    set(CLANG_TIDY_TEST
        clang-tidy "-header-filter=.*" "-checks=${CLANG_TIDY_TEST_CHECKS_JOINED}")

    set_target_properties(LOOT PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_APP}")

    set_target_properties(loot_gui_tests PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_TEST}")
endif()

##############################
# Post-Build Steps
##############################

if(Qt6_FOUND)
    set(QT_DIR ${Qt6_DIR}/../../..)
else()
    set(QT_DIR ${Qt5_DIR}/../../..)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(NOT Qt6_FOUND)
        # Copy the OpenSSL binaries, they're not included in what windeployqt
        # deploys.
        if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(QT_OPENSSL_PLATFORM "x86")
            set(QT_OPENSSL_SUFFIX "")
        else()
            set(QT_OPENSSL_PLATFORM "x64")
            set(QT_OPENSSL_SUFFIX "-x64")
        endif()
        set(QT_OPENSSL_BIN_PATH "${QT_DIR}/../../Tools/OpenSSL/Win_${QT_OPENSSL_PLATFORM}/bin")

        add_custom_command(TARGET LOOT POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_OPENSSL_BIN_PATH}/libcrypto-1_1${QT_OPENSSL_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:LOOT>/libcrypto-1_1${QT_OPENSSL_SUFFIX}.dll")

        add_custom_command(TARGET LOOT POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_OPENSSL_BIN_PATH}/libssl-1_1${QT_OPENSSL_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:LOOT>/libssl-1_1${QT_OPENSSL_SUFFIX}.dll")
    endif()

    # Copy Qt binaries and resources.
    add_custom_command(TARGET LOOT POST_BUILD
        COMMAND ${QT_DIR}/bin/windeployqt $<TARGET_FILE:LOOT>
        COMMENT "Running windeployqt...")

    add_custom_command(TARGET loot_gui_tests POST_BUILD
        COMMAND ${QT_DIR}/bin/windeployqt $<TARGET_FILE:loot_gui_tests>
        COMMENT "Running windeployqt...")
else()
    set(LOOT_QT_TEST_RESOURCES
        libicudata.so.56
        libicudata.so.56.1
        libicui18n.so.56
        libicui18n.so.56.1
        libicuuc.so.56
        libicuuc.so.56.1
        libQt6Core.so.6
        libQt6Core.so.6.2.2
        libQt6Network.so.6
        libQt6Network.so.6.2.2
        libQt6Test.so.6
        libQt6Test.so.6.2.2)

    foreach(TEST_RESOURCE ${LOOT_QT_TEST_RESOURCES})
        add_custom_command(TARGET loot_gui_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_DIR}/lib/${TEST_RESOURCE}"
                $<TARGET_FILE_DIR:loot_gui_tests>/${TEST_RESOURCE})
    endforeach()
endif()

# Copy the API binary to the build directory.
get_filename_component(LIBLOOT_SHARED_LIBRARY_FILENAME ${LIBLOOT_SHARED_LIBRARY} NAME)
add_custom_command(TARGET LOOT POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIBLOOT_SHARED_LIBRARY}
        "$<TARGET_FILE_DIR:LOOT>/${LIBLOOT_SHARED_LIBRARY_FILENAME}")
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIBLOOT_SHARED_LIBRARY}
        "$<TARGET_FILE_DIR:loot_gui_tests>/${LIBLOOT_SHARED_LIBRARY_FILENAME}")

# Copy testing plugins
ExternalProject_Get_Property(testing-plugins SOURCE_DIR)
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        $<TARGET_FILE_DIR:loot_gui_tests>)
