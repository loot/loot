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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_GOG_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_GOG_TEST

#include "gui/state/game/detection/gog.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {
class GetGogGameIdsTest : public ::testing::TestWithParam<GameId> {};

INSTANTIATE_TEST_SUITE_P(,
                         GetGogGameIdsTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(GetGogGameIdsTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(loot::gog::GetGogGameIds(GetParam()));
}

class GOG_FindGameInstallsTest : public CommonGameTestFixture,
                                 public ::testing::WithParamInterface<GameId> {
protected:
  GOG_FindGameInstallsTest() : CommonGameTestFixture(GetParam()) {}
};

INSTANTIATE_TEST_SUITE_P(,
                         GOG_FindGameInstallsTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(GOG_FindGameInstallsTest,
       shouldReturnAnEmptyVectorIfNoRegistryEntriesExist) {
  const auto installs = loot::gog::FindGameInstalls(TestRegistry(), GetParam());

  EXPECT_TRUE(installs.empty());
}

TEST_P(
    GOG_FindGameInstallsTest,
    shouldReturnAnEmptyVectorIfARegistryEntryExistsButItIsNotAValidGamePath) {
  TestRegistry registry;

  const auto gogGameIds = gog::GetGogGameIds(GetParam());
  if (!gogGameIds.empty()) {
    const auto subKey = "Software\\GOG.com\\Games\\" + gogGameIds[0];
    registry.SetStringValue(subKey, "invalid");
  }

  const auto installs = loot::gog::FindGameInstalls(registry, GetParam());

  EXPECT_TRUE(installs.empty());
}

TEST_P(GOG_FindGameInstallsTest,
       shouldReturnANonEmptyVectorIfARegistryEntryExistsWithAValidGamePath) {
  TestRegistry registry;

  const auto gogGameIds = gog::GetGogGameIds(GetParam());
  if (!gogGameIds.empty()) {
    const auto subKey = "Software\\GOG.com\\Games\\" + gogGameIds[0];
    registry.SetStringValue(subKey, dataPath.parent_path().u8string());
  }

  const auto installs = gog::FindGameInstalls(registry, GetParam());

  if (gogGameIds.empty()) {
    ASSERT_TRUE(installs.empty());
  } else {
    ASSERT_EQ(1, installs.size());
    EXPECT_EQ(GetParam(), installs[0].gameId);
    EXPECT_EQ(InstallSource::gog, installs[0].source);
    EXPECT_EQ(dataPath.parent_path(), installs[0].installPath);
    EXPECT_EQ("", installs[0].localPath);
  }
}
}

#endif
