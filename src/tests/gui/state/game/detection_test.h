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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_TEST

#include "gui/helpers.h"
#include "gui/state/game/detection.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {

class IsInstalledTest : public CommonGameTestFixture,
                        public testing::WithParamInterface<GameId> {
protected:
  IsInstalledTest() : CommonGameTestFixture(GetParam()) {}
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(, IsInstalledTest, ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(IsInstalledTest, shouldSupportNonAsciiGameMasters) {
  const auto settings = GameSettings(getGameType())
                            .SetMaster(nonAsciiEsp)
                            .SetGamePath(dataPath.parent_path());
  EXPECT_TRUE(IsInstalled(settings));
}

class UpdateInstalledGamesSettingsTest : public CommonGameTestFixture {
protected:
  UpdateInstalledGamesSettingsTest() : CommonGameTestFixture(GameId::tes3) {}

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

TEST_F(UpdateInstalledGamesSettingsTest,
       shouldReturnSettingsForGameInParentOfCurrentDirectory) {
  std::vector<GameSettings> gamesSettings;
  UpdateInstalledGamesSettings(gamesSettings, TestRegistry(), {}, {});

  ASSERT_EQ(1, gamesSettings.size());
  EXPECT_EQ(getGameType(), gamesSettings[0].Type());
  EXPECT_EQ("..", gamesSettings[0].GamePath());
  EXPECT_EQ("", gamesSettings[0].GameLocalPath());
}
}
#endif
