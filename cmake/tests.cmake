##############################
# Dependencies
##############################

find_package(Qt6 6.4 COMPONENTS Widgets Network Test REQUIRED)

set(BUILD_GMOCK OFF)
set(gtest_force_shared_crt ON)
FetchContent_Declare(
    GTest
    URL "https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz"
    URL_HASH "SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7"
    FIND_PACKAGE_ARGS)

FetchContent_Declare(
    testing-plugins
    URL "https://github.com/Ortham/testing-plugins/archive/1.6.2.tar.gz"
    URL_HASH "SHA256=f6e5b55e2669993ab650ba470424b725d1fab71ace979134a77de3373bd55620")

FetchContent_MakeAvailable(GTest testing-plugins)


##############################
# General Settings
##############################

set(LOOT_SRC_TESTS_GUI_CPP_FILES
"${CMAKE_SOURCE_DIR}/src/tests/gui/main.cpp"
"${CMAKE_SOURCE_DIR}/src/tests/gui/qt/tasks/non_blocking_test_task.cpp")

set(LOOT_SRC_TESTS_GUI_H_FILES
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/common_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/detail_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/epic_games_store_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/generic_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/gog_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/heroic_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/microsoft_store_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/steam_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection/test_registry.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/detection_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/game_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/game_settings_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/games_manager_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/group_node_positions_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/game/helpers_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_paths_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/loot_settings_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/state/unapplied_change_counter_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/qt/helpers_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/qt/tasks/non_blocking_test_task.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/qt/tasks/tasks_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/backup_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/helpers_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/sourced_message_test.h"
"${CMAKE_SOURCE_DIR}/src/tests/gui/test_helpers.h")

source_group(TREE "${CMAKE_SOURCE_DIR}/src/tests/gui"
    PREFIX "Header Files"
    FILES ${LOOT_SRC_TESTS_GUI_H_FILES})

source_group(TREE "${CMAKE_SOURCE_DIR}/src/tests/gui"
    PREFIX "Source Files"
    FILES ${LOOT_SRC_TESTS_GUI_CPP_FILES})

set(LOOT_GUI_TESTS_ALL_SOURCES
    ${LOOT_SRC_TESTS_GUI_CPP_FILES}
    ${LOOT_SRC_TESTS_GUI_H_FILES}
    "${CMAKE_BINARY_DIR}/generated/version.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/backup.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/plugin_item.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/sourced_message.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/tasks.cpp"
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
    "${CMAKE_SOURCE_DIR}/src/gui/backup.h"
    "${CMAKE_SOURCE_DIR}/src/gui/helpers.h"
    "${CMAKE_SOURCE_DIR}/src/gui/plugin_item.h"
    "${CMAKE_SOURCE_DIR}/src/gui/sourced_message.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/helpers.h"
    "${CMAKE_SOURCE_DIR}/src/gui/qt/tasks/tasks.h"
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
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_paths.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_settings.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/loot_state.h"
    "${CMAKE_SOURCE_DIR}/src/gui/state/unapplied_change_counter.h")

##############################
# Define Targets
##############################

# Build application tests.
add_executable(loot_gui_tests ${LOOT_GUI_TESTS_ALL_SOURCES})
add_dependencies(loot_gui_tests ValveFileVDF)
target_link_libraries(loot_gui_tests PRIVATE
    Qt::Widgets Qt::Network Qt::Concurrent Qt::Test
    Boost::headers Boost::locale
    GTest::gtest_main
    fmt::fmt
    spdlog::spdlog
    tomlplusplus::tomlplusplus
    ValveFileVDF::ValveFileVDF)

##############################
# Set Target-Specific Flags
##############################

if(libloot_FOUND)
    target_link_libraries(loot_gui_tests PRIVATE libloot::loot)
else()
    add_dependencies(loot_gui_tests libloot)

    target_include_directories(loot_gui_tests SYSTEM PRIVATE ${LIBLOOT_INCLUDE_DIRS})

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_link_libraries(loot_gui_tests PRIVATE ${LIBLOOT_STATIC_LIBRARY})
    else()
        target_link_libraries(loot_gui_tests PRIVATE ${LIBLOOT_SHARED_LIBRARY})
    endif()
endif()

if(ZLIB_FOUND)
    target_link_libraries(loot_gui_tests PRIVATE MINIZIP::minizip)
else()
    add_dependencies(loot_gui_tests minizip-ng)
    target_link_libraries(loot_gui_tests PRIVATE ${MINIZIP_NG_LIBRARIES})
    target_include_directories(loot_gui_tests SYSTEM PRIVATE ${MINIZIP_NG_INCLUDE_DIRS})
endif()

target_include_directories(loot_gui_tests PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(loot_gui_tests SYSTEM PRIVATE
    ${VALVE_FILE_VDF_INCLUDE_DIRS})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(loot_gui_tests PRIVATE
        UNICODE _UNICODE NOMINMAX BOOST_UUID_FORCE_AUTO_LINK)

    if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(loot_gui_tests PRIVATE LOOT_STATIC)

        target_link_libraries(loot_gui_tests PRIVATE tbb_static bz2)
    endif()
else()
    target_link_libraries(loot_gui_tests PRIVATE ${ICU_TARGETS})
endif()

if(MSVC)
    # Set /bigobj to allow building Debug tests
    target_compile_options(loot_gui_tests PRIVATE
        "/permissive-" "/W4" "/bigobj")
endif()


##############################
# Configure clang-tidy
##############################

if(LOOT_RUN_CLANG_TIDY)
    # Skip some checks for tests because they're not worth the noise (e.g. GTest
    # happens to use goto, lots of tests use magic numbers as expected values).
    set(CLANG_TIDY_TEST_CHECKS
        ${CLANG_TIDY_COMMON_CHECKS})

    list(JOIN CLANG_TIDY_TEST_CHECKS "," CLANG_TIDY_TEST_CHECKS_JOINED)

    set(CLANG_TIDY_TEST
        clang-tidy "-header-filter=.*" "-checks=${CLANG_TIDY_TEST_CHECKS_JOINED}")

    set_target_properties(loot_gui_tests PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_TEST}")
endif()

##############################
# Post-Build Steps
##############################

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        # Copy Qt binaries and resources.
        add_custom_command(TARGET loot_gui_tests POST_BUILD
            COMMAND ${QT_DIR}/bin/windeployqt $<TARGET_FILE:loot_gui_tests>
            COMMENT "Running windeployqt...")

        # Copy the API binary to the build directory.
        get_filename_component(LIBLOOT_SHARED_LIBRARY_FILENAME ${LIBLOOT_SHARED_LIBRARY} NAME)
        add_custom_command(TARGET loot_gui_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${LIBLOOT_SHARED_LIBRARY}
                "$<TARGET_FILE_DIR:loot_gui_tests>/${LIBLOOT_SHARED_LIBRARY_FILENAME}")
    endif()
endif()

# Copy testing plugins
add_custom_command(TARGET loot_gui_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${testing-plugins_SOURCE_DIR}
        $<TARGET_FILE_DIR:loot_gui_tests>)

##############################
# CTest
##############################

enable_testing()
gtest_discover_tests(loot_gui_tests DISCOVERY_TIMEOUT 10)
