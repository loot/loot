/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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
  LootPaths::initialise("");

  EXPECT_EQ(std::filesystem::current_path() / "docs",
            LootPaths::getReadmePath());
}

TEST(LootPaths, getResourcesPathShouldUseLootAppPath) {
  LootPaths::initialise("");

  EXPECT_EQ(std::filesystem::current_path() / "resources",
            LootPaths::getResourcesPath());
}

TEST(LootPaths, getL10nPathShouldUseLootAppPath) {
  LootPaths::initialise("");

  EXPECT_EQ(std::filesystem::current_path() / "resources" / "l10n",
            LootPaths::getL10nPath());
}

TEST(LootPaths, getSettingsPathShouldUseLootDataPath) {
  LootPaths::initialise("");

  EXPECT_EQ(LootPaths::getLootDataPath() / "settings.toml",
            LootPaths::getSettingsPath());
}

TEST(LootPaths, getLogPathShouldUseLootDataPath) {
  LootPaths::initialise("");

  EXPECT_EQ(LootPaths::getLootDataPath() / "LOOTDebugLog.txt",
            LootPaths::getLogPath());
}

TEST(LootPaths, initialiseShouldSetTheAppPathToTheCurrentPath) {
  LootPaths::initialise("");

  EXPECT_EQ(std::filesystem::current_path(),
            LootPaths::getReadmePath().parent_path());
}

TEST(
    LootPaths,
    initialiseShouldSetTheDataPathToTheLocalAppDataPathSlashLootIfGivenAnEmptyString) {
  LootPaths::initialise("");

  // Can't actually know what the path should be, but we can check
  // its properties.
  EXPECT_EQ("LOOT", LootPaths::getLootDataPath().filename());
  EXPECT_FALSE(LootPaths::getLootDataPath().parent_path().empty());
  EXPECT_TRUE(
      std::filesystem::exists(LootPaths::getLootDataPath().parent_path()));
}

TEST(LootPaths, initialiseShouldSetTheDataPathToGivenStringIfNonEmpty) {
  LootPaths::initialise("foo");

  EXPECT_EQ("foo", LootPaths::getLootDataPath());
}
}
}

#endif
