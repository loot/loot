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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_GENERIC_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_GENERIC_TEST

#include "gui/state/game/detection/generic.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {
class Generic_FindGameInstallsTest
    : public CommonGameTestFixture,
      public testing::WithParamInterface<GameId> {
protected:
  Generic_FindGameInstallsTest() : CommonGameTestFixture(GetParam()) {}

  void SetUp() override {
    CommonGameTestFixture::SetUp();

    initialCurrentPath = std::filesystem::current_path();

    const auto gamePath = dataPath / "..";
    const auto lootPath = gamePath / "LOOT";

    // Change the current path into a game subfolder.
    std::filesystem::create_directory(lootPath);
    std::filesystem::current_path(lootPath);
  }

  void TearDown() override {
    // Restore the previous current path.
    std::filesystem::current_path(initialCurrentPath);

    CommonGameTestFixture::TearDown();
  }

private:
  std::filesystem::path initialCurrentPath;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(,
                         Generic_FindGameInstallsTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(Generic_FindGameInstallsTest, shouldFindGameInParentOfCurrentDirectory) {
  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::generic::FindGameInstalls(TestRegistry(), gameId);

  auto expectedSource = InstallSource::unknown;
  if (GetParam() == GameId::tes5 || GetParam() == GameId::tes5vr ||
      GetParam() == GameId::fo4vr) {
    expectedSource = InstallSource::steam;
  }

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(expectedSource, gameInstalls[0].source);
  EXPECT_EQ("..", gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}
}

#endif
