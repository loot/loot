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
  TestGamesManager() :
      GamesManager(std::filesystem::path(), std::filesystem::path()) {}

  int getInitialiseCount(const std::string& folderName) {
    auto it = initialiseCounts_.find(folderName);
    if (it == initialiseCounts_.end()) {
      return 0;
    } else {
      return it->second;
    }
  }

private:
  bool isInstalled(const GameSettings& gameSettings) const override {
    return gameSettings.id() == GameId::tes5 ||
           gameSettings.id() == GameId::fonv;
  }

  void initialiseGameData(gui::Game& game) override {
    auto it = initialiseCounts_.find(game.getSettings().folderName());
    if (it == initialiseCounts_.end()) {
      initialiseCounts_.emplace(game.getSettings().folderName(), 1);
    } else {
      it->second++;
    }
  }

  std::map<std::string, unsigned int> initialiseCounts_;
};

GameSettings createSettings(GameId gameId) {
  return GameSettings(gameId, getDefaultLootFolderName(gameId));
}

static const std::vector<GameSettings> TEST_GAMES_SETTINGS = {
    createSettings(GameId::tes4),
    createSettings(GameId::tes5),
    createSettings(GameId::fonv),
};

TEST(GamesManager,
     setInstalledGamesShouldNotInitialiseAnyGamesThatAreNotInstalled) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  ASSERT_EQ(std::vector<std::string>({"Skyrim", "FalloutNV"}),
            manager.getInstalledGameFolderNames());
}

TEST(
    GamesManager,
    setInstalledGamesShouldThrowAnExceptionIfTheCurrentGameIsNoLongerInstalled) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  manager.setCurrentGame(TEST_GAMES_SETTINGS[1].folderName());

  std::vector<GameSettings> newGames(
      {TEST_GAMES_SETTINGS[0], TEST_GAMES_SETTINGS[2]});

  EXPECT_THROW(manager.setInstalledGames(newGames), GameDetectionError);
}

TEST(
    GamesManager,
    setInstalledGamesShouldReinitialiseTheCurrentGameObjectIfItsPathHasChanged) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  auto currentFolderName = TEST_GAMES_SETTINGS[1].folderName();
  manager.setCurrentGame(currentFolderName);

  manager.setInstalledGames(
      {
          createSettings(GameId::tes5).setGamePath("different"),
      });

  EXPECT_EQ(currentFolderName,
            manager.getCurrentGame().getSettings().folderName());
  EXPECT_EQ(1, manager.getInitialiseCount(currentFolderName));
}

TEST(
    GamesManager,
    setInstalledGamesShouldReinitialiseTheCurrentGameObjectIfItsLocalPathHasChanged) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  auto currentFolderName = TEST_GAMES_SETTINGS[1].folderName();
  manager.setCurrentGame(currentFolderName);

  manager.setInstalledGames(
      {
          createSettings(GameId::tes5).setGameLocalPath("different"),
      });

  EXPECT_EQ(currentFolderName,
            manager.getCurrentGame().getSettings().folderName());
  EXPECT_EQ(1, manager.getInitialiseCount(currentFolderName));
}

TEST(
    GamesManager,
    setInstalledGamesShouldReinitialiseTheCurrentGameObjectIfItsMasterHasChanged) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  auto currentFolderName = TEST_GAMES_SETTINGS[1].folderName();
  manager.setCurrentGame(currentFolderName);

  manager.setInstalledGames(
      {
          createSettings(GameId::tes5).setMaster("different"),
      });

  EXPECT_EQ(currentFolderName,
            manager.getCurrentGame().getSettings().folderName());
  EXPECT_EQ(1, manager.getInitialiseCount(currentFolderName));
}

TEST(
    GamesManager,
    setInstalledGamesShouldUpdateTheCurrentGameIfItsPathsAndMasterAreUnchanged) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  auto currentFolderName = TEST_GAMES_SETTINGS[1].folderName();
  manager.setCurrentGame(currentFolderName);

  GameSettings newGameSettings = createSettings(GameId::tes5)
                             .setName("different")
                             .setMinimumHeaderVersion(100.0f)
                             .setMasterlistSource("different");
  manager.setInstalledGames({newGameSettings});

  EXPECT_EQ(currentFolderName,
            manager.getCurrentGame().getSettings().folderName());
  EXPECT_EQ(0, manager.getInitialiseCount(currentFolderName));

  EXPECT_EQ(newGameSettings.name(),
            manager.getCurrentGame().getSettings().name());
  EXPECT_EQ(newGameSettings.minimumHeaderVersion(),
            manager.getCurrentGame().getSettings().minimumHeaderVersion());
  EXPECT_EQ(newGameSettings.masterlistSource(),
            manager.getCurrentGame().getSettings().masterlistSource());
}

TEST(GamesManager, getCurrentGameShouldThrowIfNoGamesAreInstalled) {
  TestGamesManager manager;
  EXPECT_THROW(manager.getCurrentGame(), std::runtime_error);
}

TEST(GamesManager, getCurrentGameShouldThrowIfNoCurrentGameIsSet) {
  TestGamesManager manager;
  manager.setInstalledGames({TEST_GAMES_SETTINGS[1]});

  EXPECT_THROW(manager.getCurrentGame(), std::runtime_error);
}

TEST(GamesManager, setCurrentGameShouldThrowIfTheGivenGameIsNotInstalled) {
  TestGamesManager manager;

  EXPECT_THROW(manager.setCurrentGame("invalid"), GameDetectionError);
}

TEST(GamesManager, setCurrentGameShouldUpdateStoredReference) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  manager.setCurrentGame(TEST_GAMES_SETTINGS[1].folderName());

  EXPECT_EQ(TEST_GAMES_SETTINGS[1].folderName(),
            manager.getCurrentGame().getSettings().folderName());
}

TEST(GamesManager, setCurrentGameShouldNotInitialiseGameData) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  manager.setCurrentGame(TEST_GAMES_SETTINGS[1].folderName());

  EXPECT_EQ(0, manager.getInitialiseCount(TEST_GAMES_SETTINGS[1].folderName()));
}

TEST(GamesManager,
     getFirstInstalledGameFolderNameShouldReturnNulloptIfNoGamesAreInstalled) {
  TestGamesManager manager;
  EXPECT_EQ(std::nullopt, manager.getFirstInstalledGameFolderName());
}

TEST(GamesManager,
     getFirstInstalledGameFolderNameShouldReturnAFolderNameIfAGameIsInstalled) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  EXPECT_EQ(TEST_GAMES_SETTINGS[1].folderName(),
            manager.getFirstInstalledGameFolderName());
}

TEST(
    GamesManager,
    getInstalledGameFolderNamesShouldReturnAnEmptyVectorIfNoGamesAreInstalled) {
  TestGamesManager manager;
  EXPECT_TRUE(manager.getInstalledGameFolderNames().empty());
}

TEST(
    GamesManager,
    getInstalledGameFolderNamesShouldReturnANonEmptyVectorIfNoGamesAreInstalled) {
  TestGamesManager manager;
  manager.setInstalledGames(TEST_GAMES_SETTINGS);

  std::vector<std::string> expectedFolderNames({
      TEST_GAMES_SETTINGS[1].folderName(),
      TEST_GAMES_SETTINGS[2].folderName(),
  });
  EXPECT_EQ(expectedFolderNames, manager.getInstalledGameFolderNames());
}
}
}

#endif
