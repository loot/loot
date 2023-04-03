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

namespace loot {
namespace test {
class FindGamePathsTest : public CommonGameTestFixture {
protected:
  static void CreateGameExecutable(const std::filesystem::path& gamePath) {
    // The executable check only cares about some of the games' executables,
    // so skip creating the others.
    if (GetParam() == GameType::tes5) {
      touch(gamePath / "TESV.exe");
    } else if (GetParam() == GameType::tes5se) {
      touch(gamePath / "SkyrimSE.exe");
    } else if (GetParam() == GameType::tes5vr) {
      touch(gamePath / "SkyrimVR.exe");
    } else if (GetParam() == GameType::fo4) {
      touch(gamePath / "Fallout4.exe");
    } else if (GetParam() == GameType::fo4vr) {
      touch(gamePath / "Fallout4VR.exe");
    }
  }

  static bool IsOnMicrosoftStore() {
    switch (GetParam()) {
      case GameType::tes3:
      case GameType::tes4:
      case GameType::tes5se:
      case GameType::fo3:
      case GameType::fonv:
      case GameType::fo4:
        return true;
      default:
        return false;
    }
  }

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

      CreateGameExecutable(gamePath);

      gamesPaths.push_back(gamePath);
    }

    return gamesPaths;
  }
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(,
                         FindGamePathsTest,
                         ::testing::Values(GameType::tes3,
                                           GameType::tes4,
                                           GameType::tes5,
                                           GameType::fo3,
                                           GameType::fonv,
                                           GameType::fo4,
                                           GameType::tes5se));

TEST_P(FindGamePathsTest, shouldBeNulloptIfGameIsNotInstalled) {
  const auto settings = GameSettings(GetParam()).SetRegistryKeys({});

  EXPECT_FALSE(FindGamePaths(settings, {}, {}).has_value());
}

TEST_P(FindGamePathsTest,
       shouldReturnExistingGamePathAndGameLocalPathIfGamePathIsValid) {
  const auto settings = GameSettings(GetParam())
                            .SetGamePath(dataPath.parent_path())
                            .SetGameLocalPath(localPath);

  const auto gamePaths = FindGamePaths(settings, {}, {});

  EXPECT_EQ(settings.GamePath(), gamePaths.value().installPath);
  EXPECT_EQ(settings.GameLocalPath(), gamePaths.value().localPath);
}

TEST_P(FindGamePathsTest, shouldSupportNonAsciiGameMasters) {
  const auto settings = GameSettings(GetParam(), u8"non\u00C1sciiFolder")
                            .SetGamePath(dataPath.parent_path())
                            .SetGameLocalPath(localPath)
                            .SetMaster(nonAsciiEsp);

  const auto gamePaths = FindGamePaths(settings, {}, {});

  EXPECT_EQ(settings.GamePath(), gamePaths.value().installPath);
  EXPECT_EQ(settings.GameLocalPath(), gamePaths.value().localPath);
}

TEST_P(FindGamePathsTest, shouldFindGameInParentOfCurrentDirectory) {
  const auto currentPath = std::filesystem::current_path();
  const auto gamePath = dataPath / "..";
  const auto lootPath = gamePath / "LOOT";

  // Change the current path into a game subfolder.
  std::filesystem::create_directory(lootPath);
  std::filesystem::current_path(lootPath);

  CreateGameExecutable(gamePath);

  const auto gamePaths = FindGamePaths(GameSettings(GetParam()), {}, {});

  // Restore the previous current path.
  std::filesystem::current_path(currentPath);

  EXPECT_EQ("..", gamePaths.value().installPath);
  EXPECT_EQ("", gamePaths.value().localPath);
}

TEST_P(FindGamePathsTest, shouldFindNewMSGamePathIfPresent) {
  const auto settings = GameSettings(GetParam()).SetRegistryKeys({});

  if (!IsOnMicrosoftStore()) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();
  const auto gamePath = GetGamePath(xboxGamingRootPath);
  std::filesystem::create_directories(gamePath);
  std::filesystem::copy(dataPath,
                        gamePath / dataPath.filename(),
                        std::filesystem::copy_options::recursive);

  CreateGameExecutable(gamePath);

  auto gamePaths = FindGamePaths(settings, {xboxGamingRootPath}, {});

  std::filesystem::path expectedLocalPath;
  if (settings.Type() == GameType::tes5se) {
    expectedLocalPath = getLocalAppDataPath() / "Skyrim Special Edition MS";
  } else if (settings.Type() == GameType::fo4) {
    expectedLocalPath = getLocalAppDataPath() / "Fallout4 MS";
  } else {
    expectedLocalPath = "";
  }

  EXPECT_EQ(gamePath, gamePaths.value().installPath);
  EXPECT_EQ(expectedLocalPath, gamePaths.value().localPath);
}

TEST_P(FindGamePathsTest,
       shouldNotFindNewMSGamePathIfPresentButGameIsNotAnInstanceOfItsBase) {
  const auto settings =
      GameSettings(GetParam()).SetRegistryKeys({}).SetIsBaseGameInstance(false);

  if (!IsOnMicrosoftStore()) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();
  const auto gamePath = GetGamePath(xboxGamingRootPath);
  std::filesystem::create_directories(gamePath);
  std::filesystem::copy(dataPath,
                        gamePath / dataPath.filename(),
                        std::filesystem::copy_options::recursive);

  CreateGameExecutable(gamePath);

  auto gamePaths = FindGamePaths(settings, {xboxGamingRootPath}, {});

  EXPECT_FALSE(gamePaths.has_value());
}

TEST_P(FindGamePathsTest,
       shouldNotOverrideExistingNonEmptyLocalPathIfAMSGameIsFound) {
  const auto settings =
      GameSettings(GetParam()).SetGameLocalPath(localPath).SetRegistryKeys({});

  if (!IsOnMicrosoftStore()) {
    return;
  }

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();
  const auto gamePath = GetGamePath(xboxGamingRootPath);
  std::filesystem::create_directories(gamePath);
  std::filesystem::copy(dataPath,
                        gamePath / dataPath.filename(),
                        std::filesystem::copy_options::recursive);

  CreateGameExecutable(gamePath);

  auto gamePaths = FindGamePaths(settings, {xboxGamingRootPath}, {});

  EXPECT_EQ(gamePath, gamePaths.value().installPath);
  EXPECT_EQ(settings.GameLocalPath(), gamePaths.value().localPath);
}

TEST_P(FindGamePathsTest,
       shouldTryLocalisationDirectoriesInTurnIfNoPreferredLanguagesAreGiven) {
  if (GetParam() != GameType::tes4) {
    return;
  }

  const auto settings = GameSettings(GetParam()).SetRegistryKeys({});

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY Spanish", "Oblivion GOTY Italian"});

  const auto gamePaths = FindGamePaths(settings, {xboxGamingRootPath}, {});

  EXPECT_EQ(gamesPaths[1], gamePaths.value().installPath);
  EXPECT_EQ("", gamePaths.value().localPath);
}

TEST_P(FindGamePathsTest,
       shouldTryLocalisationDirectoriesInPreferredLanguageOrder) {
  if (GetParam() != GameType::tes4) {
    return;
  }

  const auto settings = GameSettings(GetParam()).SetRegistryKeys({});

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY Spanish", "Oblivion GOTY Italian"});

  const auto gamePaths =
      FindGamePaths(settings, {xboxGamingRootPath}, {"es", "it"});

  EXPECT_EQ(gamesPaths[0], gamePaths.value().installPath);
  EXPECT_EQ("", gamePaths.value().localPath);
}

TEST_P(FindGamePathsTest,
       shouldTryLocalisationDirectoriesNotInPreferredLanguagesLast) {
  if (GetParam() != GameType::tes4) {
    return;
  }

  const auto settings = GameSettings(GetParam()).SetRegistryKeys({});

  const auto xboxGamingRootPath = dataPath.parent_path().parent_path();

  const auto gamesPaths = SetUpLocalisedGamePaths(
      xboxGamingRootPath, {"Oblivion GOTY English", "Oblivion GOTY Spanish"});

  const auto gamePaths = FindGamePaths(settings, {xboxGamingRootPath}, {"es"});

  EXPECT_EQ(gamesPaths[1], gamePaths.value().installPath);
  EXPECT_EQ("", gamePaths.value().localPath);
}
}
}
#endif
