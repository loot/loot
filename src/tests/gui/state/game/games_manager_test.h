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

#ifndef LOOT_TESTS_GUI_STATE_GAME_GAMES_MANAGER_TEST
#define LOOT_TESTS_GUI_STATE_GAME_GAMES_MANAGER_TEST

#include "gui/state/game/games_manager.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class TestGamesManager : public GamesManager {
public:
  int GetInitialiseCount(const std::string& folderName) {
    auto it = initialiseCounts_.find(folderName);
    if (it == initialiseCounts_.end()) {
      return 0;
    } else {
      return it->second;
    }
  }

private:
  std::vector<GameSettings> FindInstalledGames(
      const std::vector<GameSettings>& gamesSettings) const override {
    std::vector<GameSettings> updatedSettings;

    for (auto gameSettings : gamesSettings) {
      if (gameSettings.Id() == GameId::tes5 ||
          gameSettings.Id() == GameId::fonv) {
        gameSettings.SetGamePath(gameSettings.GamePath() /
                                 gameSettings.FolderName());
      }

      updatedSettings.push_back(gameSettings);
    }

    return updatedSettings;
  }

  bool IsInstalled(const GameSettings& gameSettings) const override {
    return gameSettings.Id() == GameId::tes5 ||
           gameSettings.Id() == GameId::fonv;
  }

  void InitialiseGameData(gui::Game& game) override {
    auto it = initialiseCounts_.find(game.GetSettings().FolderName());
    if (it == initialiseCounts_.end()) {
      initialiseCounts_.emplace(game.GetSettings().FolderName(), 1);
    } else {
      it->second++;
    }
  }

  mutable std::map<std::string, unsigned int> initialiseCounts_;
};

GameSettings createSettings(GameId gameId) {
  return GameSettings(gameId, GetDefaultLootFolderName(gameId));
}

static const std::vector<GameSettings> TEST_GAMES_SETTINGS = {
    createSettings(GameId::tes4),
    createSettings(GameId::tes5),
    createSettings(GameId::fonv),
};

TEST(
    GamesManager,
    loadInstalledGamesShouldFilterGamesSettingsThroughFindInstalledGamesBeforeUsingThem) {
  TestGamesManager manager;
  const auto settings = manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  EXPECT_EQ(TEST_GAMES_SETTINGS[0].GamePath(), settings[0].GamePath());

  EXPECT_NE(TEST_GAMES_SETTINGS[1].GamePath(), settings[1].GamePath());
  EXPECT_EQ("Skyrim", settings[1].GamePath());

  EXPECT_NE(TEST_GAMES_SETTINGS[2].GamePath(), settings[2].GamePath());
  EXPECT_EQ("FalloutNV", settings[2].GamePath());
}

TEST(GamesManager,
     loadInstalledGamesShouldNotInitialiseAnyGamesThatAreNotInstalled) {
  TestGamesManager manager;
  auto settings = manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  ASSERT_EQ(std::vector<std::string>({"Skyrim", "FalloutNV"}),
            manager.GetInstalledGameFolderNames());
}

TEST(
    GamesManager,
    loadInstalledGamesShouldThrowAnExceptionIfTheCurrentGameIsNoLongerInstalled) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  manager.SetCurrentGame(TEST_GAMES_SETTINGS[1].FolderName());

  std::vector<GameSettings> newGames(
      {TEST_GAMES_SETTINGS[0], TEST_GAMES_SETTINGS[2]});

  EXPECT_THROW(manager.LoadInstalledGames(
                   newGames, std::filesystem::path(), std::filesystem::path()),
               GameDetectionError);
}

TEST(
    GamesManager,
    loadInstalledGamesShouldReinitialiseTheCurrentGameObjectIfItsPathHasChanged) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  auto currentFolderName = TEST_GAMES_SETTINGS[1].FolderName();
  manager.SetCurrentGame(currentFolderName);

  auto settings = manager.LoadInstalledGames(
      {
          createSettings(GameId::tes5).SetGamePath("different"),
      },
      std::filesystem::path(),
      std::filesystem::path());

  EXPECT_EQ(currentFolderName,
            manager.GetCurrentGame().GetSettings().FolderName());
  EXPECT_EQ(1, manager.GetInitialiseCount(currentFolderName));

  ASSERT_EQ(1, settings.size());
  EXPECT_EQ("different/Skyrim", settings[0].GamePath());
}

TEST(
    GamesManager,
    loadInstalledGamesShouldReinitialiseTheCurrentGameObjectIfItsLocalPathHasChanged) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  auto currentFolderName = TEST_GAMES_SETTINGS[1].FolderName();
  manager.SetCurrentGame(currentFolderName);

  auto settings = manager.LoadInstalledGames(
      {
          createSettings(GameId::tes5).SetGameLocalPath("different"),
      },
      std::filesystem::path(),
      std::filesystem::path());

  EXPECT_EQ(currentFolderName,
            manager.GetCurrentGame().GetSettings().FolderName());
  EXPECT_EQ(1, manager.GetInitialiseCount(currentFolderName));

  ASSERT_EQ(1, settings.size());
  EXPECT_EQ("different", settings[0].GameLocalPath());
}

TEST(
    GamesManager,
    loadInstalledGamesShouldReinitialiseTheCurrentGameObjectIfItsMasterHasChanged) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  auto currentFolderName = TEST_GAMES_SETTINGS[1].FolderName();
  manager.SetCurrentGame(currentFolderName);

  auto settings = manager.LoadInstalledGames(
      {
          createSettings(GameId::tes5).SetMaster("different"),
      },
      std::filesystem::path(),
      std::filesystem::path());

  EXPECT_EQ(currentFolderName,
            manager.GetCurrentGame().GetSettings().FolderName());
  EXPECT_EQ(1, manager.GetInitialiseCount(currentFolderName));

  ASSERT_EQ(1, settings.size());
  EXPECT_EQ("different", settings[0].Master());
}

TEST(
    GamesManager,
    loadInstalledGamesShouldUpdateTheCurrentGameIfItsPathsAndMasterAreUnchanged) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  auto currentFolderName = TEST_GAMES_SETTINGS[1].FolderName();
  manager.SetCurrentGame(currentFolderName);

  auto newGameSettings = createSettings(GameId::tes5)
                             .SetName("different")
                             .SetMinimumHeaderVersion(100.0f)
                             .SetMasterlistSource("different");
  auto settings = manager.LoadInstalledGames(
      {newGameSettings}, std::filesystem::path(), std::filesystem::path());

  EXPECT_EQ(currentFolderName,
            manager.GetCurrentGame().GetSettings().FolderName());
  EXPECT_EQ(0, manager.GetInitialiseCount(currentFolderName));

  EXPECT_EQ(newGameSettings.Name(),
            manager.GetCurrentGame().GetSettings().Name());
  EXPECT_EQ(newGameSettings.MinimumHeaderVersion(),
            manager.GetCurrentGame().GetSettings().MinimumHeaderVersion());
  EXPECT_EQ(newGameSettings.MasterlistSource(),
            manager.GetCurrentGame().GetSettings().MasterlistSource());

  ASSERT_EQ(1, settings.size());
  EXPECT_EQ(newGameSettings.Name(), settings[0].Name());
  EXPECT_EQ(newGameSettings.MinimumHeaderVersion(),
            settings[0].MinimumHeaderVersion());
  EXPECT_EQ(newGameSettings.MasterlistSource(), settings[0].MasterlistSource());
}

TEST(GamesManager, getCurrentGameShouldThrowIfNoGamesAreInstalled) {
  TestGamesManager manager;
  EXPECT_THROW(manager.GetCurrentGame(), std::runtime_error);
}

TEST(GamesManager, getCurrentGameShouldThrowIfNoCurrentGameIsSet) {
  TestGamesManager manager;
  manager.LoadInstalledGames({TEST_GAMES_SETTINGS[1]},
                             std::filesystem::path(),
                             std::filesystem::path());

  EXPECT_THROW(manager.GetCurrentGame(), std::runtime_error);
}

TEST(GamesManager, setCurrentGameShouldThrowIfTheGivenGameIsNotInstalled) {
  TestGamesManager manager;

  EXPECT_THROW(manager.SetCurrentGame("invalid"), GameDetectionError);
}

TEST(GamesManager, setCurrentGameShouldUpdateStoredReference) {
  TestGamesManager manager;
  auto settings = manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  manager.SetCurrentGame(TEST_GAMES_SETTINGS[1].FolderName());

  EXPECT_EQ(TEST_GAMES_SETTINGS[1].FolderName(),
            manager.GetCurrentGame().GetSettings().FolderName());
}

TEST(GamesManager, setCurrentGameShouldNotInitialiseGameData) {
  TestGamesManager manager;
  auto settings = manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  manager.SetCurrentGame(TEST_GAMES_SETTINGS[1].FolderName());

  EXPECT_EQ(0, manager.GetInitialiseCount(TEST_GAMES_SETTINGS[1].FolderName()));
}

TEST(GamesManager,
     getFirstInstalledGameFolderNameShouldReturnNulloptIfNoGamesAreInstalled) {
  TestGamesManager manager;
  EXPECT_EQ(std::nullopt, manager.GetFirstInstalledGameFolderName());
}

TEST(GamesManager,
     getFirstInstalledGameFolderNameShouldReturnAFolderNameIfAGameIsInstalled) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  EXPECT_EQ(TEST_GAMES_SETTINGS[1].FolderName(),
            manager.GetFirstInstalledGameFolderName());
}

TEST(
    GamesManager,
    getInstalledGameFolderNamesShouldReturnAnEmptyVectorIfNoGamesAreInstalled) {
  TestGamesManager manager;
  EXPECT_TRUE(manager.GetInstalledGameFolderNames().empty());
}

TEST(
    GamesManager,
    getInstalledGameFolderNamesShouldReturnANonEmptyVectorIfNoGamesAreInstalled) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      TEST_GAMES_SETTINGS, std::filesystem::path(), std::filesystem::path());

  std::vector<std::string> expectedFolderNames({
      TEST_GAMES_SETTINGS[1].FolderName(),
      TEST_GAMES_SETTINGS[2].FolderName(),
  });
  EXPECT_EQ(expectedFolderNames, manager.GetInstalledGameFolderNames());
}
}
}

#endif
