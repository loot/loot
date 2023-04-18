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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_STEAM_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_STEAM_TEST

#include "gui/state/game/detection/steam.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {
class Steam_FindGameInstallsTest
    : public CommonGameTestFixture,
      public ::testing::WithParamInterface<GameId> {
protected:
  Steam_FindGameInstallsTest() : CommonGameTestFixture(GetParam()) {}

  std::string GetSteamGameId() {
    switch (GetParam()) {
      case GameId::tes3:
        return "22320";
      case GameId::tes4:
        return "22330";
      case GameId::nehrim:
        return "1014940";
      case GameId::tes5:
        return "72850";
      case GameId::enderal:
        return "933480";
      case GameId::tes5se:
        return "489830";
      case GameId::enderalse:
        return "976620";
      case GameId::tes5vr:
        return "611670";
      case GameId::fo3:
        return "22300";
      case GameId::fonv:
        return "22380";
      case GameId::fo4:
        return "377160";
      case GameId::fo4vr:
        return "611660";
      default:
        throw std::logic_error("Unsupported Steam game");
    }
  }
};

INSTANTIATE_TEST_SUITE_P(,
                         Steam_FindGameInstallsTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(Steam_FindGameInstallsTest,
       shouldReturnAnEmptyVectorIfNoRegistryEntriesExist) {
  const auto installs =
      loot::steam::FindGameInstalls(TestRegistry(), GetParam());

  EXPECT_TRUE(installs.empty());
}

TEST_P(
    Steam_FindGameInstallsTest,
    shouldReturnAnEmptyVectorIfARegistryEntryExistsButItIsNotAValidGamePath) {
  TestRegistry registry;
  const auto subKey =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
      "Steam App " +
      GetSteamGameId();
  registry.SetStringValue(subKey, "invalid");

  const auto installs = loot::steam::FindGameInstalls(registry, GetParam());

  EXPECT_TRUE(installs.empty());
}

TEST_P(Steam_FindGameInstallsTest,
       shouldReturnANonEmptyVectorIfARegistryEntryExistsWithAValidGamePath) {
  auto gamePath = dataPath.parent_path();
  if (GetParam() == GameId::nehrim) {
    // Steam's version of Nehrim puts its files in a NehrimFiles subfolder,
    // so move them there.
    std::filesystem::create_directory(gamePath / "NehrimFiles");
    std::filesystem::copy(dataPath,
                          gamePath / "NehrimFiles" / dataPath.filename());
    gamePath /= "NehrimFiles";
  }

  TestRegistry registry;
  const auto subKey =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
      "Steam App " +
      GetSteamGameId();
  registry.SetStringValue(subKey, dataPath.parent_path().u8string());

  const auto installs = loot::steam::FindGameInstalls(registry, GetParam());

  ASSERT_EQ(1, installs.size());
  EXPECT_EQ(GetParam(), installs[0].gameId);
  EXPECT_EQ(InstallSource::steam, installs[0].source);
  EXPECT_EQ(gamePath, installs[0].installPath);
  EXPECT_EQ("", installs[0].localPath);
}
}

#endif
