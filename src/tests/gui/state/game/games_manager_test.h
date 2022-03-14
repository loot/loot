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
  std::optional<GamePaths> FindGamePaths(
      const GameSettings& gameSettings) const override {
    if (gameSettings.Type() == GameType::tes5 ||
        gameSettings.Type() == GameType::fonv) {
      GamePaths gamePaths;
      // This install path matches the testing plugins folder paths.
      gamePaths.installPath =
          gameSettings.GamePath() / gameSettings.FolderName();
      gamePaths.localPath = gameSettings.GameLocalPath();
      return gamePaths;
    }

    return std::nullopt;
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

TEST(GamesManager,
     loadInstalledGamesShouldLeaveGameSettingsUnchangedIfNoGamesAreInstalled) {
  TestGamesManager manager;
  auto settings = manager.LoadInstalledGames({GameSettings(GameType::tes4)},
                                             std::filesystem::path(),
                                             std::filesystem::path());

  ASSERT_TRUE(manager.GetInstalledGameFolderNames().empty());

  EXPECT_EQ(GameSettings(GameType::tes4).GamePath(), settings[0].GamePath());
}

TEST(GamesManager, loadInstalledGamesShouldSetTheGamePathsOfInstalledGames) {
  TestGamesManager manager;
  auto settings = manager.LoadInstalledGames(
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  ASSERT_EQ(std::vector<std::string>({"Skyrim", "FalloutNV"}),
            manager.GetInstalledGameFolderNames());

  EXPECT_EQ(GameSettings(GameType::tes4).GamePath(), settings[0].GamePath());

  EXPECT_NE(GameSettings(GameType::tes5).GamePath(), settings[1].GamePath());
  EXPECT_EQ("Skyrim", settings[1].GamePath());

  EXPECT_NE(GameSettings(GameType::fonv).GamePath(), settings[2].GamePath());
  EXPECT_EQ("FalloutNV", settings[2].GamePath());
}

TEST(
    GamesManager,
    loadInstalledGamesShouldThrowAnExceptionIfTheCurrentGameIsNoLongerInstalled) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  manager.SetCurrentGame(GameSettings(GameType::tes5).FolderName());

  std::vector<GameSettings> newGames({
      GameSettings(GameType::tes4),
      GameSettings(GameType::fonv),
  });

  EXPECT_THROW(manager.LoadInstalledGames(
                   newGames, std::filesystem::path(), std::filesystem::path()),
               GameDetectionError);
}

TEST(
    GamesManager,
    loadInstalledGamesShouldReinitialiseTheCurrentGameObjectIfItsPathHasChanged) {
  TestGamesManager manager;
  manager.LoadInstalledGames(
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  auto currentFolderName = GameSettings(GameType::tes5).FolderName();
  manager.SetCurrentGame(currentFolderName);

  auto settings = manager.LoadInstalledGames(
      {
          GameSettings(GameType::tes5).SetGamePath("different"),
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
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  auto currentFolderName = GameSettings(GameType::tes5).FolderName();
  manager.SetCurrentGame(currentFolderName);

  auto settings = manager.LoadInstalledGames(
      {
          GameSettings(GameType::tes5).SetGameLocalPath("different"),
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
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  auto currentFolderName = GameSettings(GameType::tes5).FolderName();
  manager.SetCurrentGame(currentFolderName);

  auto settings = manager.LoadInstalledGames(
      {
          GameSettings(GameType::tes5).SetMaster("different"),
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
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  auto currentFolderName = GameSettings(GameType::tes5).FolderName();
  manager.SetCurrentGame(currentFolderName);

  auto newGameSettings = GameSettings(GameType::tes5)
                             .SetName("different")
                             .SetIsBaseGameInstance(false)
                             .SetMinimumHeaderVersion(100.0f)
                             .SetRegistryKeys({"different"})
                             .SetMasterlistSource("different");
  auto settings = manager.LoadInstalledGames(
      {newGameSettings}, std::filesystem::path(), std::filesystem::path());

  EXPECT_EQ(currentFolderName,
            manager.GetCurrentGame().GetSettings().FolderName());
  EXPECT_EQ(0, manager.GetInitialiseCount(currentFolderName));

  EXPECT_EQ(newGameSettings.Name(),
            manager.GetCurrentGame().GetSettings().Name());
  EXPECT_EQ(newGameSettings.IsBaseGameInstance(),
            manager.GetCurrentGame().GetSettings().IsBaseGameInstance());
  EXPECT_EQ(newGameSettings.MinimumHeaderVersion(),
            manager.GetCurrentGame().GetSettings().MinimumHeaderVersion());
  EXPECT_EQ(newGameSettings.RegistryKeys(),
            manager.GetCurrentGame().GetSettings().RegistryKeys());
  EXPECT_EQ(newGameSettings.MasterlistSource(),
            manager.GetCurrentGame().GetSettings().MasterlistSource());

  ASSERT_EQ(1, settings.size());
  EXPECT_EQ(newGameSettings.Name(), settings[0].Name());
  EXPECT_EQ(newGameSettings.MinimumHeaderVersion(),
            settings[0].MinimumHeaderVersion());
  EXPECT_EQ(newGameSettings.RegistryKeys(), settings[0].RegistryKeys());
  EXPECT_EQ(newGameSettings.MasterlistSource(), settings[0].MasterlistSource());
}

TEST(GamesManager, getCurrentGameShouldThrowIfNoGamesAreInstalled) {
  TestGamesManager manager;
  EXPECT_THROW(manager.GetCurrentGame(), std::runtime_error);
}

TEST(GamesManager, getCurrentGameShouldThrowIfNoCurrentGameIsSet) {
  TestGamesManager manager;
  manager.LoadInstalledGames({GameSettings(GameType::tes5)},
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
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  manager.SetCurrentGame(GameSettings(GameType::tes5).FolderName());

  EXPECT_EQ(GameSettings(GameType::tes5).FolderName(),
            manager.GetCurrentGame().GetSettings().FolderName());
}

TEST(GamesManager, setCurrentGameShouldNotInitialiseGameData) {
  TestGamesManager manager;
  auto settings = manager.LoadInstalledGames(
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  manager.SetCurrentGame(GameSettings(GameType::tes5).FolderName());

  EXPECT_EQ(
      0, manager.GetInitialiseCount(GameSettings(GameType::tes5).FolderName()));
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
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  EXPECT_EQ(GameSettings(GameType::tes5).FolderName(),
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
      {
          GameSettings(GameType::tes4),
          GameSettings(GameType::tes5),
          GameSettings(GameType::fonv),
      },
      std::filesystem::path(),
      std::filesystem::path());

  std::vector<std::string> expectedFolderNames({
      GameSettings(GameType::tes5).FolderName(),
      GameSettings(GameType::fonv).FolderName(),
  });
  EXPECT_EQ(expectedFolderNames, manager.GetInstalledGameFolderNames());
}
}
}

#endif
