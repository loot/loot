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

#include "gui/state/loot_paths.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(LootPaths, getReadmePathShouldUseLootAppPath) {
  LootPaths paths("");

  EXPECT_EQ(std::filesystem::current_path() / "docs",
            paths.getReadmePath());
}

TEST(LootPaths, getResourcesPathShouldUseLootAppPath) {
  LootPaths paths("");

  EXPECT_EQ(std::filesystem::current_path() / "resources",
            paths.getResourcesPath());
}

TEST(LootPaths, getL10nPathShouldUseLootAppPath) {
  LootPaths paths("");

  EXPECT_EQ(std::filesystem::current_path() / "resources" / "l10n",
            paths.getL10nPath());
}

TEST(LootPaths, getSettingsPathShouldUseLootDataPath) {
  LootPaths paths("");

  EXPECT_EQ(paths.getLootDataPath() / "settings.toml",
            paths.getSettingsPath());
}

TEST(LootPaths, getLogPathShouldUseLootDataPath) {
  LootPaths paths("");

  EXPECT_EQ(paths.getLootDataPath() / "LOOTDebugLog.txt",
            paths.getLogPath());
}

TEST(LootPaths, initialiseShouldSetTheAppPathToTheCurrentPath) {
  LootPaths paths("");

  EXPECT_EQ(std::filesystem::current_path(),
            paths.getReadmePath().parent_path());
}

TEST(
    LootPaths,
    initialiseShouldSetTheDataPathToTheLocalAppDataPathSlashLootIfGivenAnEmptyString) {
  LootPaths paths("");

  // Can't actually know what the path should be, but we can check
  // its properties.
  EXPECT_EQ("LOOT", paths.getLootDataPath().filename());
  EXPECT_FALSE(paths.getLootDataPath().parent_path().empty());
  EXPECT_TRUE(
      std::filesystem::exists(paths.getLootDataPath().parent_path()));
}

TEST(LootPaths, initialiseShouldSetTheDataPathToGivenStringIfNonEmpty) {
  LootPaths paths("foo");

  EXPECT_EQ("foo", paths.getLootDataPath());
}
}
}

#endif
