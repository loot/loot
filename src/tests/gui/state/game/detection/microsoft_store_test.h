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

namespace loot::test {
class Microsoft_FindGameInstallsTest : public CommonGameTestFixture {
protected:
  static std::filesystem::path GetGamePath(
      const std::filesystem::path& xboxGamingRootPath) {
    switch (GetParam()) {
      case GameType::tes3:
        return xboxGamingRootPath / "The Elder Scrolls III- Morrowind (PC)" /
               "Content" / "Morrowind GOTY English";
      case GameType::tes4:
        return xboxGamingRootPath / "The Elder Scrolls IV- Oblivion (PC)" /
               "Content" / "Oblivion GOTY English";
      case GameType::tes5se:
        return xboxGamingRootPath /
               "The Elder Scrolls V- Skyrim Special Edition (PC)" / "Content";
      case GameType::fo3:
        return xboxGamingRootPath / "Fallout 3- Game of the Year Edition (PC)" /
               "Content" / "Fallout 3 GOTY English";
      case GameType::fonv:
        return xboxGamingRootPath / "Fallout- New Vegas Ultimate Edition (PC)" /
               "Content" / "Fallout New Vegas English";
      case GameType::fo4:
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
      std::filesystem::create_directories(gamePath);
      std::filesystem::copy(dataPath,
                            gamePath / dataPath.filename(),
                            std::filesystem::copy_options::recursive);

      if (isExecutableNeeded(GetParam())) {
        touch(gamePath / getGameExecutable(GetParam()));
      }

      gamesPaths.push_back(gamePath);
    }

    return gamesPaths;
  }
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(,
                         Microsoft_FindGameInstallsTest,
                         ::testing::Values(GameType::tes3,
                                           GameType::tes4,
                                           GameType::tes5se,
                                           GameType::fo3,
                                           GameType::fonv,
                                           GameType::fo4));

TEST_P(Microsoft_FindGameInstallsTest, shouldFindNewMSGamePathIfPresent) {
  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();
  const auto gamePath = GetGamePath(xboxGamingRootPath);
  std::filesystem::create_directories(gamePath);
  std::filesystem::copy(dataPath,
                        gamePath / dataPath.filename(),
                        std::filesystem::copy_options::recursive);

  if (isExecutableNeeded(GetParam())) {
    touch(gamePath / getGameExecutable(GetParam()));
  }

  const auto gameId = getTestGameId(GetParam());
  const auto gameInstalls =
      loot::microsoft::FindGameInstalls(gameId, {xboxGamingRootPath}, {});

  std::filesystem::path expectedLocalPath;
  if (GetParam() == GameType::tes5se) {
    expectedLocalPath = getLocalAppDataPath() / "Skyrim Special Edition MS";
  } else if (GetParam() == GameType::fo4) {
    expectedLocalPath = getLocalAppDataPath() / "Fallout4 MS";
  } else {
    expectedLocalPath = "";
  }

  ASSERT_EQ(1, gameInstalls.size());
  EXPECT_EQ(gameId, gameInstalls[0].gameId);
  EXPECT_EQ(InstallSource::microsoft, gameInstalls[0].source);
  EXPECT_EQ(gamePath, gameInstalls[0].installPath);
  EXPECT_EQ(expectedLocalPath, gameInstalls[0].localPath);
}

TEST_P(Microsoft_FindGameInstallsTest,
       shouldTryLocalisationDirectoriesInTurnIfNoPreferredLanguagesAreGiven) {
  if (GetParam() != GameType::tes4) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY Spanish", "Oblivion GOTY Italian"});

  const auto gameId = getTestGameId(GetParam());
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
  if (GetParam() != GameType::tes4) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY Spanish", "Oblivion GOTY Italian"});

  const auto gameId = getTestGameId(GetParam());
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
  if (GetParam() != GameType::tes4) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY English", "Oblivion GOTY Spanish"});

  const auto gameId = getTestGameId(GetParam());
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
