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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_MICROSOFT_STORE_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_MICROSOFT_STORE_TEST

#include "gui/helpers.h"
#include "gui/state/game/detection/microsoft_store.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/state/game/detection/test_registry.h"

namespace loot::test {
class Microsoft_FindGameInstallsTest
    : public CommonGameTestFixture,
      public testing::WithParamInterface<GameId> {
protected:
  Microsoft_FindGameInstallsTest() : CommonGameTestFixture(GetParam()) {}

  static std::filesystem::path GetGamePath(
      const std::filesystem::path& xboxGamingRootPath) {
    switch (GetParam()) {
      case GameId::tes3:
        return xboxGamingRootPath / "The Elder Scrolls III- Morrowind (PC)" /
               "Content" / "Morrowind GOTY English";
      case GameId::tes4:
        return xboxGamingRootPath / "The Elder Scrolls IV- Oblivion (PC)" /
               "Content" / "Oblivion GOTY English";
      case GameId::tes5se:
        return xboxGamingRootPath /
               "The Elder Scrolls V- Skyrim Special Edition (PC)" / "Content";
      case GameId::fo3:
        return xboxGamingRootPath / "Fallout 3- Game of the Year Edition (PC)" /
               "Content" / "Fallout 3 GOTY English";
      case GameId::fonv:
        return xboxGamingRootPath / "Fallout- New Vegas Ultimate Edition (PC)" /
               "Content" / "Fallout New Vegas English";
      case GameId::fo4:
        return xboxGamingRootPath / "Fallout 4 (PC)" / "Content";
      default:
        throw std::runtime_error("Unsupported Microsoft Store game");
    }
  }

  std::vector<std::filesystem::path> SetUpLocalisedGamePaths(
      const std::filesystem::path& xboxGamingRootPath,
      const std::initializer_list<const char*>& gameFolders) {
    std::vector<std::filesystem::path> gamesPaths;

    for (const auto& gameFolder : gameFolders) {
      const auto gamePath = xboxGamingRootPath /
                            "The Elder Scrolls IV- Oblivion (PC)" / "Content" /
                            gameFolder;
      std::filesystem::create_directories(gamePath.parent_path());
      std::filesystem::copy(dataPath.parent_path(),
                            gamePath,
                            std::filesystem::copy_options::recursive);

      gamesPaths.push_back(gamePath);
    }

    return gamesPaths;
  }

  std::string GetPackageName() const {
    switch (GetParam()) {
      case GameId::tes3:
        return "BethesdaSoftworks.TESMorrowind-PC_3275kfvn8vcwc";
      case GameId::tes4:
        return "BethesdaSoftworks.TESOblivion-PC_3275kfvn8vcwc";
      case GameId::tes5se:
        return "BethesdaSoftworks.SkyrimSE-PC_3275kfvn8vcwc";
      case GameId::fo3:
        return "BethesdaSoftworks.Fallout3_3275kfvn8vcwc";
      case GameId::fonv:
        return "BethesdaSoftworks.FalloutNewVegas_3275kfvn8vcwc";
      case GameId::fo4:
        return "BethesdaSoftworks.Fallout4-PC_3275kfvn8vcwc";
      default:
        throw std::logic_error("Unexpected game ID");
    }
  }
};

// Pass an empty first argument, as it's a prefix for the test instantiation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(,
                         Microsoft_FindGameInstallsTest,
                         ::testing::Values(GameId::tes3,
                                           GameId::tes4,
                                           GameId::tes5se,
                                           GameId::fo3,
                                           GameId::fonv,
                                           GameId::fo4));

TEST_P(Microsoft_FindGameInstallsTest, shouldFindNewMSGamePathIfPresent) {
  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();
  const auto gamePath = GetGamePath(xboxGamingRootPath);
  std::filesystem::create_directories(gamePath.parent_path());
  std::filesystem::copy(dataPath.parent_path(),
                        gamePath,
                        std::filesystem::copy_options::recursive);

  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::microsoft::FindGameInstalls(gameId, {xboxGamingRootPath}, {});

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(InstallSource::microsoft, gameInstalls[0].source);
  EXPECT_EQ(gamePath, gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}

TEST_P(Microsoft_FindGameInstallsTest,
       shouldTryLocalisationDirectoriesInTurnIfNoPreferredLanguagesAreGiven) {
  if (GetParam() != GameId::tes4) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY Spanish", "Oblivion GOTY Italian"});

  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::microsoft::FindGameInstalls(gameId, {xboxGamingRootPath}, {});

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(InstallSource::microsoft, gameInstalls[0].source);
  EXPECT_EQ(gamesPaths[1], gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}

TEST_P(Microsoft_FindGameInstallsTest,
       shouldTryLocalisationDirectoriesInPreferredLanguageOrder) {
  if (GetParam() != GameId::tes4) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY Spanish", "Oblivion GOTY Italian"});

  const auto gameId = GetParam();
  const auto gameInstalls = loot::microsoft::FindGameInstalls(
      gameId, {xboxGamingRootPath}, {"es", "it"});

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(InstallSource::microsoft, gameInstalls[0].source);
  EXPECT_EQ(gamesPaths[0], gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}

TEST_P(Microsoft_FindGameInstallsTest,
       shouldTryLocalisationDirectoriesNotInPreferredLanguagesLast) {
  if (GetParam() != GameId::tes4) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY English", "Oblivion GOTY Spanish"});

  const auto gameId = GetParam();
  const auto gameInstalls =
      loot::microsoft::FindGameInstalls(gameId, {xboxGamingRootPath}, {"es"});

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(InstallSource::microsoft, gameInstalls[0].source);
  EXPECT_EQ(gamesPaths[1], gameInstalls[0].installPath);
  EXPECT_EQ("", gameInstalls[0].localPath);
}
}
#endif
