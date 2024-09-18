cmake_minimum_required(VERSION 3.19)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 OLD)
endif()
project(LOOT)
include(ExternalProject)
include(FetchContent)

option(LOOT_RUN_CLANG_TIDY "Whether or not to run clang-tidy during build. Has no effect when using CMake's MSVC generator." OFF)
option(LOOT_BUILD_TESTS "Whether or not to build LOOT's tests." ON)

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
endif()

if(NOT GIT_COMMIT_STRING)
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

# Allow use of shared Boost libs, e.g. if installed through the Linux distribution's package manager.
if(NOT DEFINED Boost_USE_STATIC_LIBS)
    set(Boost_USE_STATIC_LIBS ON)
endif()
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost REQUIRED COMPONENTS locale)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(ICU 71.1 REQUIRED COMPONENTS data uc i18n)
    find_package(TBB REQUIRED)
endif()

find_package(Qt6 6.4 COMPONENTS Widgets Network Concurrent REQUIRED)

if(NOT DEFINED EP_PREFIX)
    set_directory_properties(PROPERTIES "EP_PREFIX" "external")
endif()

find_package(libloot CONFIG)
if(libloot_FOUND)
    get_target_property(LIBLOOT_INCLUDE_DIRS libloot::loot INTERFACE_INCLUDE_DIRECTORIES)
    set(LIBLOOT_SHARED_LIBRARY $<TARGET_LINKER_FILE:libloot::loot>)
else()
    if(NOT DEFINED LIBLOOT_URL)
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.24.1/libloot-0.24.1-win64.7z")
            set(LIBLOOT_HASH "SHA256=dcdd4b2107dab9f3c0bdc2cdef0c66fcaec0dc8c6076638d861d6681f18f4682")
        else()
            set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.24.1/libloot-0.24.1-Linux.tar.xz")
            set(LIBLOOT_HASH "SHA256=13b77400d2125ad6171ea9d7008049ad0897cb07d18182e22f9d89e050c2c9b8")
        endif()
    else()
        file(TO_CMAKE_PATH "${LIBLOOT_URL}" LIBLOOT_URL)
    endif()

    ExternalProject_Add(libloot
        URL ${LIBLOOT_URL}
        URL_HASH ${LIBLOOT_HASH}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "")
    ExternalProject_Get_Property(libloot SOURCE_DIR)
    set(LIBLOOT_INCLUDE_DIRS "${SOURCE_DIR}/include")
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(LIBLOOT_STATIC_LIBRARY "${SOURCE_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}loot.dll${CMAKE_STATIC_LIBRARY_SUFFIX}")
    else()
        set(LIBLOOT_STATIC_LIBRARY "${SOURCE_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}loot${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(LIBLOOT_SHARED_LIBRARY "${SOURCE_DIR}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}loot${CMAKE_SHARED_LIBRARY_SUFFIX}")
    else()
        # lib64 is used if the library was packaged on anything other than Debian (or a Debian-derived distro, e.g. Ubuntu)
        # see <https://stackoverflow.com/questions/76517286/how-does-cmake-decide-to-make-a-lib-or-lib64-directory-for-installations>
        # GitHub Actions runners use Ubuntu.
        if(NOT DEFINED LIBLOOT_LIB_DIR)
            set(LIBLOOT_LIB_DIR "lib")
        endif()
        set(LIBLOOT_SHARED_LIBRARY "${SOURCE_DIR}/${LIBLOOT_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}loot${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()
endif()


if(NOT DEFINED MINIZIP_NG_URL)
    set(MINIZIP_NG_URL "https://github.com/zlib-ng/minizip-ng/archive/refs/tags/4.0.7.tar.gz")
    set(MINIZIP_NG_HASH "SHA256=a87f1f734f97095fe1ef0018217c149d53d0f78438bcb77af38adc21dff2dfbc")
endif()

find_package(ZLIB)
if(ZLIB_FOUND)
    set(MZ_ZLIB ON)
    set(MZ_BZIP2 OFF)
    set(MZ_LZMA OFF)
    set(MZ_ZSTD OFF)
    set(MZ_FETCH_LIBS OFF)
    set(MZ_PKCRYPT OFF)
    set(MZ_WZAES OFF)
    set(MZ_OPENSSL OFF)
    set(MZ_LIBBSD OFF)
    set(MZ_SIGNING OFF)
    set(MZ_COMPRESS_ONLY ON)
    FetchContent_Declare(
        MINIZIP
        URL ${MINIZIP_NG_URL}
        URL_HASH ${MINIZIP_NG_HASH}
        FIND_PACKAGE_ARGS)

    FetchContent_MakeAvailable(MINIZIP)
else()
    # zlib and minizip-ng don't really work with FetchContent if zlib isn't already installed, as
    # minizip-ng errors if find_package(ZLIB) fails, and FetchContent(ZLIB) doesn't support
    # build-in-source so minizip-ng include won't find zconf.h.

    if(NOT DEFINED ZLIB_URL)
        set(ZLIB_URL "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz")
        set(ZLIB_HASH "SHA256=17e88863f3600672ab49182f217281b6fc4d3c762bde361935e436a95214d05c")
    endif()

    ExternalProject_Add(zlib
        URL ${ZLIB_URL}
        URL_HASH ${ZLIB_HASH}
        CMAKE_ARGS
            -DZLIB_BUILD_EXAMPLES=OFF
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        INSTALL_COMMAND ""
        BUILD_IN_SOURCE TRUE)
    ExternalProject_Get_Property(zlib SOURCE_DIR BINARY_DIR)

    # minizip-ng can't find zlib if they're both being built for the first time, so explicitly
    # provide the library path.
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(ZLIB_LIB_NAME "zlibstatic$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(ZLIB_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}")
    else()
        set(ZLIB_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endif()
    set(ZLIB_LIBRARY
        "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${ZLIB_LIB_NAME}")

    ExternalProject_Add(minizip-ng
        URL ${MINIZIP_NG_URL}
        URL_HASH ${MINIZIP_NG_HASH}
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
            -DZLIB_ROOT=${SOURCE_DIR}
            -DZLIB_LIBRARY=${ZLIB_LIBRARY}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        INSTALL_COMMAND ""
        DEPENDS zlib)
    ExternalProject_Get_Property(minizip-ng SOURCE_DIR BINARY_DIR)
    set(MINIZIP_NG_INCLUDE_DIRS "${SOURCE_DIR}")
    set(MINIZIP_NG_LIBRARIES
        "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}minizip${CMAKE_STATIC_LIBRARY_SUFFIX}"
        ${ZLIB_LIBRARY})
endif()

FetchContent_Declare(
    tomlplusplus
    URL "https://github.com/marzer/tomlplusplus/archive/v3.4.0.tar.gz"
    URL_HASH "SHA256=8517f65938a4faae9ccf8ebb36631a38c1cadfb5efa85d9a72e15b9e97d25155"
    FIND_PACKAGE_ARGS)

FetchContent_Declare(
    fmt
    URL "https://github.com/fmtlib/fmt/archive/refs/tags/11.0.2.tar.gz"
    URL_HASH "SHA256=6cb1e6d37bdcb756dbbe59be438790db409cdb4868c66e888d5df9f13f7c027f"
    FIND_PACKAGE_ARGS)

set(SPDLOG_FMT_EXTERNAL ON)
if(WIN32)
    set(SPDLOG_WCHAR_FILENAMES ON)
endif()
FetchContent_Declare(
    spdlog
    URL "https://github.com/gabime/spdlog/archive/v1.14.1.tar.gz"
    URL_HASH "SHA256=1586508029a7d0670dfcb2d97575dcdc242d3868a259742b69f100801ab4e16b"
    FIND_PACKAGE_ARGS)

if(NOT DEFINED VALVE_FILE_VDF_URL)
    set(VALVE_FILE_VDF_URL "https://github.com/TinyTinni/ValveFileVDF/archive/1a132f3b0b3cf501bdec03a99cdf009d99fc951c.tar.gz")
    set(VALVE_FILE_VDF_HASH "SHA256=28390fbc0f3d3250262c9229007d2989738c67c9c3a6ed47964fec8c7e84e073")
endif()

FetchContent_Declare(
    ValveFileVDF
    URL ${VALVE_FILE_VDF_URL}
    URL_HASH ${VALVE_FILE_VDF_HASH})

FetchContent_MakeAvailable(fmt spdlog tomlplusplus ValveFileVDF)

find_package(OGDF CONFIG)
if(NOT OGDF_FOUND)
    set(OGDF_CONFIG $<IF:$<CONFIG:Release,RelWithDebInfo>,Release,Debug>)

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(OGDF_BUILD_COMMAND
            ${CMAKE_COMMAND}
            --build .
            --target OGDF
            --config ${OGDF_CONFIG})
    else()
        set(OGDF_BUILD_COMMAND ${CMAKE_COMMAND} --build . --target OGDF)
    endif()

    if(NOT DEFINED OGDF_URL)
        set(OGDF_URL "https://ogdf.uos.de/wp-content/uploads/2023/09/ogdf.v2023.09.zip")
        set(OGDF_HASH "SHA256=a82298cfbcd9365d9650f3e5c09b49dc3e6cdf7ff80483133d02b9d371165436")
    endif()

    # Use ExternalProject for OGDF as otherwise there's no way to stop it from
    # creating and building loads of unnecessary targets.
    ExternalProject_Add(OGDF
        URL ${OGDF_URL}
        URL_HASH ${OGDF_HASH}
        CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        BUILD_COMMAND ${OGDF_BUILD_COMMAND}
        INSTALL_COMMAND "")
    ExternalProject_Get_Property(OGDF SOURCE_DIR BINARY_DIR)
    set(OGDF_INCLUDE_DIRS
        "${SOURCE_DIR}/include"
        "${BINARY_DIR}/include")

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(OGDF_LIBRARY_DIR "${BINARY_DIR}/${OGDF_CONFIG}")
    else()
        set(OGDF_LIBRARY_DIR "${BINARY_DIR}")
    endif()

    set(OGDF_LIBRARIES
        "${OGDF_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}OGDF${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${OGDF_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}COIN${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

##############################
# General Settings
##############################

set(LOOT_SRC_GUI_CPP_FILES
    "${CMAKE_SOURCE_DIR}/src/gui/backup.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/card_delegate.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/counters.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/filters_widget.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/general_info.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/general_info_card.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/edge.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/groups_editor_dialog.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/graph_view.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/layout.cpp"
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
    "${CMAKE_SOURCE_DIR}/src/gui/sourced_message.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_item_model.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_item_filter_model.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/search_dialog.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/game_tab.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/general_tab.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/new_game_dialog.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/settings_dialog.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/sidebar_plugin_name_delegate.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/style.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/check_for_update_task.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/network_task.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/tasks.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/update_masterlist_task.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/common.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/detail.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/epic_games_store.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/generic.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/gog.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/heroic.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/microsoft_store.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/registry.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/steam.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/group_node_positions.cpp"
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
    "${CMAKE_SOURCE_DIR}/src/gui/qt/card_delegate.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/counters.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/filters_states.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/filters_widget.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/general_info.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/general_info_card.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/edge.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/groups_editor_dialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/graph_view.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/groups_editor/layout.h"
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
    "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/metadata_table_model.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/models/tag_table_model.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/plugin_editor_widget.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_editor/table_tabs.h"
    "${CMAKE_SOURCE_DIR}/src/gui/plugin_item.h"
    "${CMAKE_SOURCE_DIR}/src/gui/sourced_message.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_item_model.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/plugin_item_filter_model.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/search_dialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/game_tab.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/general_tab.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/new_game_dialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/settings/settings_dialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/sidebar_plugin_name_delegate.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/style.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/check_for_update_task.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/network_task.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/tasks.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/update_masterlist_task.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/apply_sort_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/cancel_sort_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/change_game_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/clear_all_metadata_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/clear_plugin_metadata_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_overlapping_plugins_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/get_game_data_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/query/types/sort_plugins_query.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/common.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/detail.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/epic_games_store.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/game_install.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/generic.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/gog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/heroic.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/microsoft_store.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/registry.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection/steam.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/detection.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/games_manager.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/group_node_positions.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/logging.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/unapplied_change_counter.h"
    "${CMAKE_SOURCE_DIR}/src/gui/resource.h"
    "${CMAKE_SOURCE_DIR}/src/gui/version.h")

source_group(TREE "${CMAKE_SOURCE_DIR}/src/gui"
    PREFIX "Header Files"
    FILES ${LOOT_SRC_GUI_H_FILES})

source_group(TREE "${CMAKE_SOURCE_DIR}/src/gui"
    PREFIX "Source Files"
    FILES ${LOOT_SRC_GUI_CPP_FILES})

set(LOOT_ALL_SOURCES
    ${LOOT_SRC_GUI_CPP_FILES}
    ${LOOT_SRC_GUI_H_FILES}
    "${CMAKE_BINARY_DIR}/generated/version.cpp"
    "${CMAKE_SOURCE_DIR}/resources/resources.qrc")

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
target_link_libraries(LOOT PRIVATE
    Qt::Widgets Qt::Network Qt::Concurrent
    Boost::headers Boost::locale
    fmt::fmt
    spdlog::spdlog
    tomlplusplus::tomlplusplus
    ValveFileVDF::ValveFileVDF)

##############################
# Set Target-Specific Flags
##############################

if(libloot_FOUND)
    target_link_libraries(LOOT PRIVATE libloot::loot)
else()
    add_dependencies(LOOT libloot)

    target_include_directories(LOOT SYSTEM PRIVATE ${LIBLOOT_INCLUDE_DIRS})

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_link_libraries(LOOT PRIVATE ${LIBLOOT_STATIC_LIBRARY})
    else()
        target_link_libraries(LOOT PRIVATE ${LIBLOOT_SHARED_LIBRARY})
    endif()
endif()

if(ZLIB_FOUND)
    target_link_libraries(LOOT PRIVATE MINIZIP::minizip)
else()
    add_dependencies(LOOT minizip-ng)
    target_link_libraries(LOOT PRIVATE ${MINIZIP_NG_LIBRARIES})
    target_include_directories(LOOT SYSTEM PRIVATE ${MINIZIP_NG_INCLUDE_DIRS})
endif()

if(OGDF_FOUND)
    target_link_libraries(LOOT PRIVATE OGDF)
else()
    add_dependencies(LOOT OGDF)
    target_link_libraries(LOOT PRIVATE ${OGDF_LIBRARIES})
    target_include_directories(LOOT SYSTEM PRIVATE ${OGDF_INCLUDE_DIRS})
endif()

# Include source and library directories.
target_include_directories(LOOT PRIVATE "${CMAKE_SOURCE_DIR}/src")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(LOOT PRIVATE UNICODE _UNICODE NOMINMAX)

    if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(LOOT PRIVATE LOOT_STATIC)

        target_link_libraries(LOOT PRIVATE tbb_static bz2)
        target_link_libraries(Qt6::Widgets INTERFACE bz2)
        target_link_libraries(Qt6::Network INTERFACE brotlicommon)
    endif()
else()
    set(ICU_TARGETS ICU::data ICU::uc TBB::tbb)

    target_link_libraries(LOOT PRIVATE X11 ${ICU_TARGETS})
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(LOOT PRIVATE "-Wall" "-Wextra")
endif()

if(MSVC)
    set_target_properties(LOOT PROPERTIES WIN32_EXECUTABLE TRUE)

    # Turn off permissive mode to be more standards-compliant and avoid compiler errors.
    target_compile_options(LOOT PRIVATE "/permissive-" "/W4")
endif()

##############################
# Configure clang-tidy
##############################

if(LOOT_RUN_CLANG_TIDY)
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
        "cppcoreguidelines-pro-type-member-init"
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

    list(JOIN CLANG_TIDY_APP_CHECKS "," CLANG_TIDY_APP_CHECKS_JOINED)

    set(CLANG_TIDY_APP
        clang-tidy "-header-filter=.*" "-checks=${CLANG_TIDY_APP_CHECKS_JOINED}")

    set_target_properties(LOOT PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_APP}")
endif()

##############################
# Post-Build Steps
##############################

set(QT_DIR ${Qt6_DIR}/../../..)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        # Copy Qt binaries and resources.
        add_custom_command(TARGET LOOT POST_BUILD
            COMMAND ${QT_DIR}/bin/windeployqt $<TARGET_FILE:LOOT>
            COMMENT "Running windeployqt...")

        set(DESTINATION_LIBLOOT_SHARED_LIBRARY_FILENAME "loot.dll")
    else()
        set(DESTINATION_LIBLOOT_SHARED_LIBRARY_FILENAME "libloot.dll")
    endif()
else()
    set(DESTINATION_LIBLOOT_SHARED_LIBRARY_FILENAME "libloot.so")
endif()

# Copy the API binary to the build directory.
get_filename_component(LIBLOOT_SHARED_LIBRARY_FILENAME ${LIBLOOT_SHARED_LIBRARY} NAME)
add_custom_command(TARGET LOOT POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
       ${LIBLOOT_SHARED_LIBRARY}
       "$<TARGET_FILE_DIR:LOOT>/${DESTINATION_LIBLOOT_SHARED_LIBRARY_FILENAME}")

if(LOOT_BUILD_TESTS)
    include("cmake/tests.cmake")
endif()
