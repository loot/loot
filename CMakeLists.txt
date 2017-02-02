cmake_minimum_required (VERSION 2.8.12.1)
cmake_policy(SET CMP0015 NEW)
project (LOOT)
include(ExternalProject)

option(BUILD_SHARED_LIBS "Build a shared library" ON)
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
configure_file("${CMAKE_SOURCE_DIR}/src/api/loot_version.cpp.in" "${CMAKE_BINARY_DIR}/generated/loot_version.cpp" @ONLY)

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

ExternalProject_Add(libespm
                    PREFIX "external"
                    URL "https://github.com/WrinklyNinja/libespm/archive/2.5.5.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(libespm SOURCE_DIR)
set (LIBESPM_INCLUDE_DIRS "${SOURCE_DIR}/include")

ExternalProject_Add(libgit2
                    PREFIX "external"
                    URL "https://github.com/libgit2/libgit2/archive/v0.25.1.tar.gz"
                    CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DBUILD_CLAR=OFF -DSTATIC_CRT=${MSVC_STATIC_RUNTIME}
                    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target git2 --config $(CONFIGURATION)
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(libgit2 SOURCE_DIR BINARY_DIR)
set(LIBGIT2_INCLUDE_DIRS "${SOURCE_DIR}/include")
set(LIBGIT2_LIBRARIES "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}git2${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(libloadorder
                    PREFIX "external"
                    DEPENDS libespm
                    URL "https://github.com/WrinklyNinja/libloadorder/archive/9.5.4.tar.gz"
                    CMAKE_ARGS -DBOOST_ROOT=${BOOST_ROOT} -DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR} -DMSVC_STATIC_RUNTIME=${MSVC_STATIC_RUNTIME}
                    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target loadorder --config $(CONFIGURATION)
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(libloadorder SOURCE_DIR BINARY_DIR)
set(LIBLOADORDER_INCLUDE_DIRS "${SOURCE_DIR}/include")
set(LIBLOADORDER_LIBRARIES "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}loadorder${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(pseudosem
                    PREFIX "external"
                    URL "https://github.com/WrinklyNinja/pseudosem/archive/1.1.0.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(pseudosem SOURCE_DIR)
set(PSEUDOSEM_INCLUDE_DIRS "${SOURCE_DIR}/include")

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
    set(CEF_URL "http://opensource.spotify.com/cefbuilds/cef_binary_3.2924.1561.g06fde99_windows32_minimal.tar.bz2")
    set(CEF_URL_HASH "6ad9d9280f0b7d28e4cc8816b87f28dd3f8b0453")
    set(CEF_PRECOMPILED_BINARIES "libcef.dll" "natives_blob.bin" "snapshot_blob.bin" "d3dcompiler_47.dll" "libEGL.dll" "libGLESv2.dll" "chrome_elf.dll")
    set(CEF_LIB libcef${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSE ()
    set(CEF_URL "http://opensource.spotify.com/cefbuilds/cef_binary_3.2924.1561.g06fde99_linux64_minimal.tar.bz2")
    set(CEF_URL_HASH "62e008195d7891134e12b7dd58ee42c3c8f316b3")
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


set (LOOT_API_SRC "${CMAKE_BINARY_DIR}/generated/loot_version.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/api.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/api_database.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/error_categories.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/condition_evaluator.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/conditional_metadata.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/file.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/location.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/message.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/message_content.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/plugin_cleaning_data.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/plugin_metadata.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/priority.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/tag.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/game/game.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/game/game_cache.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/game/load_order_handler.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata_list.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/masterlist.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/plugin/plugin.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/plugin/plugin_sorter.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/helpers/crc.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/helpers/git_helper.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/helpers/language.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/helpers/version.cpp")

set (LOOT_API_HEADERS "${CMAKE_SOURCE_DIR}/include/loot/api.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/api_decorator.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/database_interface.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/error_categories.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/condition_syntax_error.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/cyclic_interaction_error.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/file_access_error.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/git_state_error.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/enum/game_type.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/enum/language_code.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/enum/log_verbosity.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/enum/message_type.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/enum/plugin_cleanliness.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/game_interface.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/language.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/loot_version.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/conditional_metadata.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/file.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/location.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/message.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/message_content.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/plugin_cleaning_data.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/plugin_metadata.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/priority.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/tag.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/file.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/location.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/message.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/message_content.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/plugin_cleaning_data.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/plugin_metadata.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/set.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/tag.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/plugin_interface.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/struct/masterlist_info.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/struct/simple_message.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/windows_encoding_converters.h"
                      "${CMAKE_SOURCE_DIR}/src/api/api_database.h"
                      "${CMAKE_SOURCE_DIR}/src/api/metadata/condition_evaluator.h"
                      "${CMAKE_SOURCE_DIR}/src/api/metadata/condition_grammar.h"
                      "${CMAKE_SOURCE_DIR}/src/api/game/game.h"
                      "${CMAKE_SOURCE_DIR}/src/api/game/game_cache.h"
                      "${CMAKE_SOURCE_DIR}/src/api/game/load_order_handler.h"
                      "${CMAKE_SOURCE_DIR}/src/api/metadata_list.h"
                      "${CMAKE_SOURCE_DIR}/src/api/masterlist.h"
                      "${CMAKE_SOURCE_DIR}/src/api/plugin/plugin.h"
                      "${CMAKE_SOURCE_DIR}/src/api/plugin/plugin_sorter.h"
                      "${CMAKE_SOURCE_DIR}/src/api/helpers/git_helper.h"
                      "${CMAKE_SOURCE_DIR}/src/api/helpers/crc.h"
                      "${CMAKE_SOURCE_DIR}/src/api/helpers/version.h")

set (LOOT_GUI_SRC "${CMAKE_SOURCE_DIR}/src/gui/main.cpp"
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
                  "${CMAKE_SOURCE_DIR}/src/resource.rc")

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
                  "${CMAKE_SOURCE_DIR}/src/gui/yaml_simple_message_helpers.h")

set (LOOT_TESTS_SRC "${CMAKE_SOURCE_DIR}/src/tests/api/internals/main.cpp")

set (LOOT_TESTS_HEADERS "${CMAKE_SOURCE_DIR}/src/tests/api/internals/game/game_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/game/game_cache_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/game/load_order_handler_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/git_helper_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/crc_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/language_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/version_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/yaml_set_helpers_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/condition_evaluator_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/condition_grammar_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/conditional_metadata_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/file_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/location_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/message_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/message_content_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/plugin_cleaning_data_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/plugin_metadata_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/priority_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/tag_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/plugin/plugin_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/plugin/plugin_sorter_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/masterlist_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata_list_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/common_game_test_fixture.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/printers.h")

set(LOOT_API_TESTS_SRC "${CMAKE_SOURCE_DIR}/src/tests/api/interface/main.cpp")

set(LOOT_API_TESTS_HEADERS  "${CMAKE_SOURCE_DIR}/src/tests/api/interface/api_game_operations_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/api/interface/create_game_handle_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/api/interface/database_interface_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/api/interface/game_interface_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/api/interface/is_compatible_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/common_game_test_fixture.h")

set(LOOT_GUI_TESTS_SRC "${CMAKE_SOURCE_DIR}/src/gui/state/game.cpp"
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
source_group("Header Files\\api" FILES ${LOOT_API_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_TESTS_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_API_TESTS_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_GUI_TESTS_HEADERS})

source_group("Source Files\\gui" FILES ${LOOT_GUI_SRC})
source_group("Source Files\\api" FILES ${LOOT_API_SRC})
source_group("Source Files\\tests" FILES ${LOOT_TESTS_SRC})
source_group("Source Files\\tests" FILES ${LOOT_API_TESTS_SRC})
source_group("Header Files\\tests" FILES ${LOOT_GUI_TESTS_SRC})

# Include source and library directories.
include_directories ("${CMAKE_SOURCE_DIR}/src"
                     "${CMAKE_SOURCE_DIR}/include"
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
    set (LOOT_API_SRC ${LOOT_API_SRC} "${CMAKE_SOURCE_DIR}/src/resource.rc")
ENDIF ()

IF (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};.")
    set (CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c++14")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14")

    set (LOOT_LIBS ssl
                   curl
                   z
                   crypto
                   rt)
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

    set (LOOT_LIBS  version
                    ws2_32
                    shlwapi
                    winhttp
                    crypt32
                    Rpcrt4)
    set (LOOT_GUI_LIBS comctl32
                       Psapi)
ENDIF ()


##############################
# Define Targets
##############################

# Build tests.
add_executable       (loot_api_internals_tests ${LOOT_API_SRC} ${LOOT_API_HEADERS} ${LOOT_TESTS_SRC} ${LOOT_TESTS_HEADERS})
add_dependencies     (loot_api_internals_tests libespm libgit2 libloadorder pseudosem yaml-cpp GTest testing-metadata testing-plugins)
target_link_libraries(loot_api_internals_tests ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES} ${LIBLOADORDER_LIBRARIES} ${LOOT_LIBS} ${YAML_CPP_LIBRARIES} ${GTEST_LIBRARIES} ${LIBLOADORDER_LIBRARIES})

# Build API.
add_library          (loot_api ${LOOT_API_SRC} ${LOOT_API_HEADERS})
add_dependencies     (loot_api libespm libgit2 libloadorder pseudosem yaml-cpp)
target_link_libraries(loot_api ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES} ${LIBLOADORDER_LIBRARIES} ${LOOT_LIBS} ${YAML_CPP_LIBRARIES})

# Build API tests.
add_executable       (loot_api_tests ${LOOT_API_TESTS_SRC} ${LOOT_API_TESTS_HEADERS})
add_dependencies     (loot_api_tests loot_api GTest testing-metadata testing-plugins)
target_link_libraries(loot_api_tests loot_api ${GTEST_LIBRARIES})

# Build application.
add_executable       (LOOT ${LOOT_GUI_SRC} ${LOOT_GUI_HEADERS})
add_dependencies     (LOOT cef loot_api)
target_link_libraries(LOOT ${CEF_LIBRARIES} loot_api ${LOOT_GUI_LIBS})

# Build application tests.
add_executable       (loot_gui_tests ${LOOT_GUI_TESTS_SRC} ${LOOT_GUI_TESTS_HEADERS})
add_dependencies     (loot_gui_tests loot_api GTest testing-metadata testing-plugins)
target_link_libraries(loot_gui_tests loot_api ${GTEST_LIBRARIES})

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
    set_target_properties (loot_api_internals_tests PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} LOOT_STATIC")
    IF (BUILD_SHARED_LIBS)
        set_target_properties (loot_api PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} LOOT_EXPORT")
    ELSE ()
        set_target_properties (loot_api PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} LOOT_STATIC")
    ENDIF ()
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

# Build the UI HTML.
add_custom_command(TARGET LOOT POST_BUILD
    COMMAND "node" "${CMAKE_SOURCE_DIR}/scripts/build_ui.js"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Copy testing metadata
ExternalProject_Get_Property(testing-metadata SOURCE_DIR)
add_custom_command(TARGET loot_api_internals_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        "$<TARGET_FILE_DIR:loot_api_internals_tests>/testing-metadata")
add_custom_command(TARGET loot_api_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        "$<TARGET_FILE_DIR:loot_api_tests>/testing-metadata")

# Copy testing plugins
ExternalProject_Get_Property(testing-plugins SOURCE_DIR)
add_custom_command(TARGET loot_api_internals_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        $<TARGET_FILE_DIR:loot_api_internals_tests>)
add_custom_command(TARGET loot_api_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
      $<TARGET_FILE_DIR:loot_api_tests>)
