/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014 WrinklyNinja

This file is part of LOOT.

LOOT is free software: you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

LOOT is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LOOT.  If not, see
<https://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TESTS_GUI_STATE_LOOT_PATHS_TEST
#define LOOT_TESTS_GUI_STATE_LOOT_PATHS_TEST

#include <gtest/gtest.h>

#include "gui/state/loot_paths.h"

namespace loot {
namespace test {
TEST(LootPaths, getReadmePathShouldUseLootAppPath) {
#ifdef _WIN32
  LootPaths paths("app", "");

  EXPECT_EQ(std::filesystem::u8path("app") / "docs", paths.getReadmePath());
#else
  LootPaths paths("prefix/app", "");

  EXPECT_EQ(std::filesystem::u8path("prefix") / "share" / "doc" / "loot",
            paths.getReadmePath());
#endif
}

TEST(LootPaths, getL10nPathShouldUseLootAppPath) {
#ifdef _WIN32
  LootPaths paths("app", "");

  EXPECT_EQ(std::filesystem::u8path("app") / "resources" / "l10n",
            paths.getL10nPath());
#else
  LootPaths paths("prefix/app", "");

  EXPECT_EQ(std::filesystem::u8path("prefix") / "share" / "locale",
            paths.getL10nPath());
#endif
}

TEST(LootPaths, getSettingsPathShouldUseLootDataPath) {
  LootPaths paths("", "");

  EXPECT_EQ(paths.getLootDataPath() / "settings.toml", paths.getSettingsPath());
}

TEST(LootPaths, getLogPathShouldUseLootDataPath) {
  LootPaths paths("", "");

  EXPECT_EQ(paths.getLootDataPath() / "LOOTDebugLog.txt", paths.getLogPath());
}

TEST(LootPaths, getPreludePathShouldUseLootDataPath) {
  LootPaths paths("", "");

  EXPECT_EQ(paths.getLootDataPath() / "prelude" / "prelude.yaml",
            paths.getPreludePath());
}

#ifdef _WIN32
TEST(LootPaths,
     constructorShouldSetAppPathToExecutableDirectoryIfGivenPathIsEmpty) {
  LootPaths paths("", "");

  // The current path is the build directory, not the config-specific
  // subdirectory.
  EXPECT_EQ(std::filesystem::current_path(),
            paths.getReadmePath().parent_path().parent_path());
}
#else
TEST(LootPaths,
     constructorShouldSetAppPathToExecutableParentDirectoryIfGivenPathIsEmpty) {
  LootPaths paths("", "");

  EXPECT_EQ(std::filesystem::current_path().parent_path(),
            paths.getL10nPath().parent_path().parent_path());
}
#endif

TEST(
    LootPaths,
    constructorShouldSetTheDataPathToTheLocalAppDataPathSlashLootIfGivenAnEmptyString) {
  LootPaths paths("app", "");

  // Can't actually know what the path should be, but we can check
  // its properties.
  EXPECT_EQ("LOOT", paths.getLootDataPath().filename());
  EXPECT_FALSE(paths.getLootDataPath().parent_path().empty());
}

TEST(LootPaths, constructorShouldSetTheDataPathToGivenStringIfNonEmpty) {
  LootPaths paths("", "foo");

  EXPECT_EQ("foo", paths.getLootDataPath());
}
}
}

#endif
