cmake_minimum_required(VERSION 3.24)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
project(LOOT)
include(ExternalProject)
include(FetchContent)

option(LOOT_RUN_CLANG_TIDY "Whether or not to run clang-tidy during build. Has no effect when using CMake's MSVC generator." OFF)
option(LOOT_BUILD_TESTS "Whether or not to build LOOT's tests." ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

if(MSVC)
    add_compile_options("/MP")
endif()

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
# Generate ICO file
##############################

if(WIN32)
    add_custom_target(loot_ico_icon
        COMMAND svg_to_ico
            -i "${CMAKE_SOURCE_DIR}/resources/icons/loot.svg"
            -o "${CMAKE_BINARY_DIR}/icon/icon.ico"
        BYPRODUCTS "${CMAKE_BINARY_DIR}/icon/icon.ico")
endif()

##############################
# External Projects
##############################

# Allow use of shared Boost libs, e.g. if installed through the Linux distribution's package manager.
if(NOT DEFINED Boost_USE_STATIC_LIBS)
    set(Boost_USE_STATIC_LIBS ON)
endif()
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost REQUIRED COMPONENTS locale CONFIG)

if(LINUX)
    find_package(ICU REQUIRED COMPONENTS data uc i18n)
    find_package(TBB REQUIRED)
endif()

find_package(Qt6 6.4 COMPONENTS Widgets Network Concurrent REQUIRED)

if(WIN32 AND CMAKE_HOST_LINUX)
    # The MinGW build of Qt6 expects you to link to bz2.
    find_package(BZip2 MODULE REQUIRED)

    # The MinGW build of Qt6::Network expects you to link to brotlicommon.
    set(BROTLI_BUILD_TOOLS OFF)
    FetchContent_Declare(
        Brotli
        URL "https://github.com/google/brotli/archive/refs/tags/v1.2.0.tar.gz"
        URL_HASH "SHA256=816c96e8e8f193b40151dad7e8ff37b1221d019dbcb9c35cd3fadbfe6477dfec"
        FIND_PACKAGE_ARGS)

    FetchContent_MakeAvailable(Brotli)
endif()

if(NOT DEFINED EP_PREFIX)
    set_directory_properties(PROPERTIES "EP_PREFIX" "external")
endif()

find_package(libloot CONFIG)
if(NOT libloot_FOUND)
    set(LIBLOOT_GIT_REPOSITORY_DEFAULT "https://github.com/loot/libloot.git")
    set(LIBLOOT_GIT_COMMIT_DEFAULT "7de8bdb3a88ddb65b19f7c1ca5924e38fea06bbb")
    if(NOT DEFINED LIBLOOT_GIT_REPOSITORY)
        set(LIBLOOT_GIT_REPOSITORY ${LIBLOOT_GIT_REPOSITORY_DEFAULT})
    endif()
    if(NOT DEFINED LIBLOOT_GIT_COMMIT)
        set(LIBLOOT_GIT_COMMIT ${LIBLOOT_GIT_COMMIT_DEFAULT})
    endif()

    if(NOT MSVC OR (GIT_FOUND AND (NOT LIBLOOT_GIT_REPOSITORY STREQUAL LIBLOOT_GIT_REPOSITORY_DEFAULT OR NOT LIBLOOT_GIT_COMMIT STREQUAL LIBLOOT_GIT_COMMIT_DEFAULT)))
        set(LIBLOOT_USE_PREBUILT_MSVC_BINARY FALSE)
    elseif(NOT DEFINED LIBLOOT_USE_PREBUILT_MSVC_BINARY)
        set(LIBLOOT_USE_PREBUILT_MSVC_BINARY TRUE)
    endif()

    # If a URL is set and a Git source is not, prefer the URL over the Git or default source.
    if(LIBLOOT_USE_PREBUILT_MSVC_BINARY)
        if(DEFINED LIBLOOT_URL)
            cmake_path(CONVERT "${LIBLOOT_URL}" TO_CMAKE_PATH_LIST LIBLOOT_URL)
        else()
            set(LIBLOOT_URL "https://github.com/loot/libloot/releases/download/0.29.0/libloot-0.29.0-win64.7z")
            set(LIBLOOT_HASH "SHA256=d4f3025de4eb3361465de527467fd1d2aea92110174d12399c1ad24673a5c1af")
        endif()
        set(LIBLOOT_DOWNLOAD_ARGS
            URL ${LIBLOOT_URL}
            URL_HASH ${LIBLOOT_HASH})
    elseif(GIT_FOUND)
        set(LIBLOOT_DOWNLOAD_ARGS
            GIT_REPOSITORY ${LIBLOOT_GIT_REPOSITORY}
            GIT_TAG ${LIBLOOT_GIT_COMMIT})
    else()
        message(WARNING "Git not found: building libloot from a source archive")
        set(LIBLOOT_DOWNLOAD_ARGS
            URL "https://github.com/loot/libloot/archive/refs/tags/0.29.0.tar.gz"
            URL_HASH "SHA256=e8cfb4b0fcb76f36f1313ea94519667fc43ec949ad3adb1aa0ba8b02338978b3")
    endif()

    message(DEBUG "LIBLOOT_USE_PREBUILT_MSVC_BINARY: ${LIBLOOT_USE_PREBUILT_MSVC_BINARY}")
    message(DEBUG "LIBLOOT_DOWNLOAD_ARGS: ${LIBLOOT_DOWNLOAD_ARGS}")
endif()

if(MSVC AND LIBLOOT_USE_PREBUILT_MSVC_BINARY AND NOT libloot_FOUND)
    # Download a prebuilt binary and use it.
    # For some reason if we use FetchContent's OVERRIDE_FIND_PACKAGE with find_package, find_package will find libloot but not import its target, so instead explicitly make it available and give an explicit search path to find_package.
    FetchContent_Declare(
        libloot
        ${LIBLOOT_DOWNLOAD_ARGS})

    FetchContent_MakeAvailable(libloot)

    find_package(libloot REQUIRED CONFIG
        PATHS "${FETCHCONTENT_BASE_DIR}/libloot-src/lib/cmake/libloot")
else()
    set(LIBLOOT_BUILD_SHARED ON)
    set(LIBLOOT_BUILD_TESTS OFF)
    set(LIBLOOT_INSTALL_DOCS OFF)
    FetchContent_Declare(
        libloot
        ${LIBLOOT_DOWNLOAD_ARGS}
        SOURCE_SUBDIR "cpp"
        FIND_PACKAGE_ARGS)

    FetchContent_MakeAvailable(libloot)
endif()


if(NOT DEFINED ZLIB_REPOSITORY)
    set(ZLIB_REPOSITORY "https://github.com/madler/zlib")
endif()

if(NOT DEFINED ZLIB_TAG)
    set(ZLIB_TAG "570720b0c24f9686c33f35a1b3165c1f568b96be") # v1.3.1.2
endif()

set(ZLIB_BUILD_EXAMPLES OFF)
set(ZLIB_COMPAT ON)

set(MZ_ZLIB ON)
set(MZ_BZIP2 OFF)
set(MZ_LZMA OFF)
set(MZ_ZSTD OFF)
set(MZ_FETCH_LIBS ON)
set(MZ_PKCRYPT OFF)
set(MZ_WZAES OFF)
set(MZ_OPENSSL OFF)
set(MZ_LIBBSD OFF)
set(MZ_SIGNING OFF)
set(MZ_COMPRESS_ONLY ON)
FetchContent_Declare(
    MINIZIP
    URL "https://github.com/zlib-ng/minizip-ng/archive/refs/tags/4.1.0.tar.gz"
    URL_HASH "SHA256=85417229bb0cd56403e811c316150eea1a3643346d9cec7512ddb7ea291b06f2"
    FIND_PACKAGE_ARGS NAMES MINIZIP minizip-ng)

FetchContent_Declare(
    tomlplusplus
    URL "https://github.com/marzer/tomlplusplus/archive/v3.4.0.tar.gz"
    URL_HASH "SHA256=8517f65938a4faae9ccf8ebb36631a38c1cadfb5efa85d9a72e15b9e97d25155"
    FIND_PACKAGE_ARGS)

FetchContent_Declare(
    fmt
    URL "https://github.com/fmtlib/fmt/archive/refs/tags/12.0.0.tar.gz"
    URL_HASH "SHA256=aa3e8fbb6a0066c03454434add1f1fc23299e85758ceec0d7d2d974431481e40"
    FIND_PACKAGE_ARGS)

set(SPDLOG_FMT_EXTERNAL ON)
if(WIN32)
    set(SPDLOG_WCHAR_FILENAMES ON)
endif()
FetchContent_Declare(
    spdlog
    URL "https://github.com/gabime/spdlog/archive/v1.16.0.tar.gz"
    URL_HASH "SHA256=8741753e488a78dd0d0024c980e1fb5b5c85888447e309d9cb9d949bdb52aa3e"
    FIND_PACKAGE_ARGS)

FetchContent_Declare(
    ValveFileVDF
    URL "https://github.com/TinyTinni/ValveFileVDF/archive/refs/tags/v1.1.1.tar.gz"
    URL_HASH "SHA256=de16a199c535c3b49f2aa0bd17e3154e02b32fa7b0949053ba6d981f8c32197f")

FetchContent_MakeAvailable(fmt MINIZIP spdlog tomlplusplus ValveFileVDF)

find_package(OGDF CONFIG)
if(NOT OGDF_FOUND)
    set(OGDF_CONFIG $<IF:$<CONFIG:Release,RelWithDebInfo>,Release,Debug>)
    set(OGDF_CONFIG_LOWERCASE $<IF:$<CONFIG:Release,RelWithDebInfo>,release,debug>)
    set(OGDF_BASENAME_SUFFIX $<IF:$<CONFIG:Release,RelWithDebInfo>,,-debug>)

    if(WIN32)
        set(OGDF_BUILD_COMMAND
            ${CMAKE_COMMAND}
            --build .
            --target OGDF
            --config ${OGDF_CONFIG})
    else()
        set(OGDF_BUILD_COMMAND ${CMAKE_COMMAND} --build . --target OGDF)
    endif()

    if(NOT DEFINED OGDF_URL)
        set(OGDF_URL "https://ogdf.uos.de/wp-content/uploads/2025/10/ogdf.v2025.10.zip")
        set(OGDF_HASH "SHA256=54f8fc9c2b44b3e4f34ca85be3ae77ce2a309337397662c1432a960367522ccb")
    endif()

    if(WIN32 AND CMAKE_HOST_WIN32)
        set(OGDF_CFG_INTDIR "${OGDF_CONFIG}")
    else()
        set(OGDF_CFG_INTDIR ".")
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
        BUILD_BYPRODUCTS
            "<BINARY_DIR>/${OGDF_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}OGDF${OGDF_BASENAME_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}"
            "<BINARY_DIR>/${OGDF_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}COIN${OGDF_BASENAME_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}"
            "<BINARY_DIR>/include/ogdf-${OGDF_CONFIG_LOWERCASE}"
        INSTALL_COMMAND "")
    ExternalProject_Get_Property(OGDF SOURCE_DIR BINARY_DIR)

    add_library(OGDF_INTERFACE INTERFACE)
    add_dependencies(OGDF_INTERFACE OGDF)

    target_link_libraries(OGDF_INTERFACE INTERFACE
            "${BINARY_DIR}/${OGDF_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}OGDF${OGDF_BASENAME_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}"
            "${BINARY_DIR}/${OGDF_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}COIN${OGDF_BASENAME_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

    target_include_directories(OGDF_INTERFACE INTERFACE
            "${SOURCE_DIR}/include"
            "${BINARY_DIR}/include/ogdf-${OGDF_CONFIG_LOWERCASE}")
endif()

##############################
# General Settings
##############################

set(LOOT_SRC_GUI_CPP_FILES
    "${CMAKE_SOURCE_DIR}/src/gui/backup.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/back_up_load_order_dialog.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/card.cpp"
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
    "${CMAKE_SOURCE_DIR}/src/gui/qt/restore_load_order_dialog.cpp"
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
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_id.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/games_manager.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/group_node_positions.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/validation.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/logging.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/translate.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/resource.rc")

set(LOOT_SRC_GUI_H_FILES
    "${CMAKE_SOURCE_DIR}/src/gui/application_mutex.h"
    "${CMAKE_SOURCE_DIR}/src/gui/backup.h"
    "${CMAKE_SOURCE_DIR}/src/gui/helpers.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/back_up_load_order_dialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/card.h"
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
    "${CMAKE_SOURCE_DIR}/src/gui/qt/restore_load_order_dialog.h"
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
    "${CMAKE_SOURCE_DIR}/src/gui/state/change_count.h"
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
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_id.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/game_settings.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/games_manager.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/group_node_positions.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/helpers.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/load_order_backup.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/game/validation.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/logging.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
    "${CMAKE_SOURCE_DIR}/src/gui/translate.h"
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

if(WIN32)
    list(APPEND LOOT_ALL_SOURCES
        "${CMAKE_SOURCE_DIR}/resources/resources.windows.qrc"
        "${CMAKE_SOURCE_DIR}/src/gui/LOOT.manifest")
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
    libloot::libloot
    spdlog::spdlog
    tomlplusplus::tomlplusplus
    ValveFileVDF::ValveFileVDF)

##############################
# Set Target-Specific Flags
##############################

if(TARGET MINIZIP::minizip)
    target_link_libraries(LOOT PRIVATE MINIZIP::minizip)
elseif(TARGET MINIZIP::minizip-ng)
    target_link_libraries(LOOT PRIVATE MINIZIP::minizip-ng)
endif()

if(OGDF_FOUND)
    target_link_libraries(LOOT PRIVATE OGDF)
else()
    target_link_libraries(LOOT PRIVATE OGDF_INTERFACE)
endif()

# Include source and library directories.
target_include_directories(LOOT PRIVATE "${CMAKE_SOURCE_DIR}/src")

set_target_properties(LOOT PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON)

if(WIN32)
    add_dependencies(LOOT loot_ico_icon)
    target_compile_definitions(LOOT PRIVATE UNICODE _UNICODE NOMINMAX)

    if(CMAKE_HOST_LINUX)
        target_compile_definitions(LOOT PRIVATE LOOT_STATIC)

        target_link_libraries(LOOT PRIVATE BZip2::BZip2)
        target_link_libraries(Qt6::Widgets INTERFACE BZip2::BZip2)
        target_link_libraries(Qt6::Network INTERFACE BZip2::BZip2 brotlicommon)
    endif()
else()
    set(ICU_TARGETS ICU::data ICU::uc TBB::tbb)

    target_link_libraries(LOOT PRIVATE X11 ${ICU_TARGETS})
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(LOOT PRIVATE
        "-Wall"
        "-Wextra"
        "-Wpedantic"
        "-Wconversion"
        "-Wdeprecated"
        "-Wuninitialized"
        "-Winit-self"
        "-Wstringop-overflow=4"
        "-Wduplicated-branches"
        "-Wduplicated-cond"
        "-Wzero-as-null-pointer-constant"
        "-Wfloat-equal"
        "-Wcast-qual"
        "-Wcast-align"
        "-Wlogical-op"
        "-Wvla"
        "-Wno-xor-used-as-pow")
endif()

if(WIN32)
    set_target_properties(LOOT PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(MSVC)
    # Turn off permissive mode to be more standards-compliant and avoid compiler errors.
    target_compile_options(LOOT PRIVATE
        "/permissive-"
        "/W4"
        "/Wall"
        "/wd4371"
        "/wd4514"
        "/wd4623"
        "/wd4625"
        "/wd4626"
        "/wd4710"
        "/wd4711"
        "/wd4820"
        "/wd4866"
        "/wd5026"
        "/wd5027"
        "/wd5045"
        "/external:anglebrackets"
        "/external:W0"
        "/sdl"
        "$<$<CONFIG:Debug>:/RTC1>"
        "/Zc:__cplusplus"
        "/GL"
        "/MP")

    target_link_options(LOOT PRIVATE "/INCREMENTAL:NO" "/LTCG")
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

if(WIN32)
    if(CMAKE_HOST_WIN32)
        # Copy Qt binaries and resources.
        add_custom_command(TARGET LOOT POST_BUILD
            COMMAND ${QT_DIR}/bin/windeployqt $<TARGET_FILE:LOOT>
            COMMENT "Running windeployqt...")
    else()
        # Copy Qt binaries and resources.
        add_custom_command(TARGET LOOT POST_BUILD
                COMMAND wine ${QT_DIR}/bin/windeployqt.exe $<TARGET_FILE:LOOT>
                COMMENT "Running windeployqt..."
                VERBATIM)

        # Replace the libstdc++-6.dll because the one that windeployqt copies is too old.
        add_custom_command(TARGET LOOT POST_BUILD
                COMMAND "sh" "${CMAKE_SOURCE_DIR}/scripts/replace_libstdc++.sh" "${CMAKE_BINARY_DIR}"
                VERBATIM)
    endif()
endif()

# Copy the API binary to the build directory.
add_custom_command(TARGET LOOT POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
       $<TARGET_FILE:libloot::libloot>
       "$<TARGET_FILE_DIR:LOOT>/$<TARGET_FILE_NAME:libloot::libloot>")

if(LINUX)
    add_custom_command(TARGET LOOT POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_SONAME_FILE:libloot::libloot>
            "$<TARGET_FILE_DIR:LOOT>/$<TARGET_SONAME_FILE_NAME:libloot::libloot>")
endif()

if(LOOT_BUILD_TESTS)
    include("cmake/tests.cmake")
endif()
