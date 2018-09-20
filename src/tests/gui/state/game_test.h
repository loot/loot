/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2018    WrinklyNinja

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

#ifndef LOOT_TESTS_GUI_STATE_GAME_TEST
#define LOOT_TESTS_GUI_STATE_GAME_TEST

#include <fstream>

#include "gui/state/game.h"

#include "gui/state/game_detection_error.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace gui {
namespace test {
class GameTest : public loot::test::CommonGameTestFixture {
protected:
  GameTest() :
      loadOrderToSet_({
          masterFile,
          blankEsm,
          blankMasterDependentEsm,
          blankDifferentEsm,
          blankDifferentMasterDependentEsm,
          blankDifferentEsp,
          blankDifferentPluginDependentEsp,
          blankEsp,
          blankMasterDependentEsp,
          blankDifferentMasterDependentEsp,
          blankPluginDependentEsp,
      }),
      info_(std::vector<MessageContent>({
          MessageContent("info"),
      })),
      loadOrderBackupFile0("loadorder.bak.0"),
      loadOrderBackupFile1("loadorder.bak.1"),
      loadOrderBackupFile2("loadorder.bak.2"),
      loadOrderBackupFile3("loadorder.bak.3") {}

  void TearDown() { CommonGameTestFixture::TearDown(); }

  std::vector<std::string> loadOrderToSet_;
  const std::string loadOrderBackupFile0;
  const std::string loadOrderBackupFile1;
  const std::string loadOrderBackupFile2;
  const std::string loadOrderBackupFile3;

  const std::vector<MessageContent> info_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        GameTest,
                        ::testing::Values(GameType::tes4,
                                          GameType::tes5,
                                          GameType::fo3,
                                          GameType::fonv,
                                          GameType::fo4,
                                          GameType::tes5se));

TEST_P(GameTest, constructingFromGameSettingsShouldUseTheirValues) {
  GameSettings settings = GameSettings(GetParam(), "folder");
  settings.SetName("foo");
  settings.SetMaster(blankEsm);
  settings.SetRegistryKey("foo");
  settings.SetRepoURL("foo");
  settings.SetRepoBranch("foo");
  settings.SetGamePath(dataPath.parent_path());
  settings.SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);

  EXPECT_EQ(GetParam(), game.Type());
  EXPECT_EQ(settings.Name(), game.Name());
  EXPECT_EQ(settings.FolderName(), game.FolderName());
  EXPECT_EQ(settings.Master(), game.Master());
  EXPECT_EQ(settings.RegistryKey(), game.RegistryKey());
  EXPECT_EQ(settings.RepoURL(), game.RepoURL());
  EXPECT_EQ(settings.RepoBranch(), game.RepoBranch());

  EXPECT_EQ(settings.GamePath(), game.GamePath());
  EXPECT_EQ(lootDataPath / "folder" / "masterlist.yaml", game.MasterlistPath());
  EXPECT_EQ(lootDataPath / "folder" / "userlist.yaml", game.UserlistPath());
}

#ifndef _WIN32
// Testing on Windows will find real game installs in the Registry, so cannot
// test autodetection fully unless on Linux.
TEST_P(GameTest, constructingShouldThrowOnLinuxIfGamePathIsNotGiven) {
  EXPECT_THROW(Game(GameSettings(GetParam()).SetGameLocalPath(localPath), ""),
               GameDetectionError);
}

TEST_P(GameTest, constructingShouldThrowOnLinuxIfLocalPathIsNotGiven) {
  auto settings = GameSettings(GetParam()).SetGamePath(dataPath.parent_path());
  EXPECT_THROW(Game(settings, lootDataPath), std::system_error);
}
#else
TEST_P(GameTest, constructingShouldNotThrowOnWindowsIfLocalPathIsNotGiven) {
  auto settings = GameSettings(GetParam()).SetGamePath(dataPath.parent_path());
  EXPECT_NO_THROW(Game(settings, lootDataPath));
}
#endif

TEST_P(GameTest, copyConstructorShouldCopyGameData) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game1(settings, lootDataPath);
  game1.AppendMessage(Message(MessageType::say, "1"));

  Game game2(game1);

  EXPECT_EQ(game1.MasterlistPath(), game2.MasterlistPath());
  EXPECT_EQ(game1.ArePluginsFullyLoaded(), game2.ArePluginsFullyLoaded());
  EXPECT_EQ(game1.GetMessages(), game2.GetMessages());
}

TEST_P(GameTest, assignmentOperatorShouldCopyGameData) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game1(settings, lootDataPath);
  game1.AppendMessage(Message(MessageType::say, "1"));

  Game game2 = game1;

  EXPECT_EQ(game1.MasterlistPath(), game2.MasterlistPath());
  EXPECT_EQ(game1.ArePluginsFullyLoaded(), game2.ArePluginsFullyLoaded());
  EXPECT_EQ(game1.GetMessages(), game2.GetMessages());
}

TEST_P(GameTest, isInstalledShouldBeFalseIfGamePathIsNotSet) {
  EXPECT_FALSE(Game::IsInstalled(GameSettings(GetParam())));
}

TEST_P(GameTest, isInstalledShouldBeTrueIfGamePathIsValid) {
  EXPECT_TRUE(Game::IsInstalled(
      GameSettings(GetParam()).SetGamePath(dataPath.parent_path())));
}

TEST_P(GameTest, isInstalledShouldBeTrueForOnlyOneSiblingGameAtATime) {
  auto currentPath = std::filesystem::current_path();

  std::filesystem::create_directory(dataPath / ".." / "LOOT");
  std::filesystem::current_path(dataPath / ".." / "LOOT");
  if (GetParam() == GameType::tes5) {
    std::ofstream out(std::filesystem::path("..") / "TESV.exe");
    // out << "";
    out.close();
  } else if (GetParam() == GameType::tes5se) {
    std::ofstream out(std::filesystem::path("..") /
                                    "SkyrimSE.exe");
    // out << "";
    out.close();
  }

  GameType gameTypes[6] = {
      GameType::tes4,
      GameType::tes5,
      GameType::fo3,
      GameType::fonv,
      GameType::fo4,
      GameType::tes5se,
  };
  for (int i = 0; i < 6; ++i) {
    if (gameTypes[i] == GetParam()) {
      EXPECT_TRUE(Game::IsInstalled(GameSettings(gameTypes[i])));
    } else {
      EXPECT_FALSE(Game::IsInstalled(GameSettings(gameTypes[i])));
    }
  }

  std::filesystem::current_path(currentPath);
  std::filesystem::remove_all(dataPath / ".." / "LOOT");
  if (GetParam() == GameType::tes5) {
    std::filesystem::remove(dataPath / ".." / "TESV.exe");
  } else if (GetParam() == GameType::tes5se) {
    std::filesystem::remove(dataPath / ".." / "SkyrimSE.exe");
  }
}

TEST_P(GameTest, toMessageShouldOutputAllNonZeroCounts) {
  Message message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 2, 10, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 2 ITM records, 10 deleted references and 30 deleted "
      "navmeshes. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 0, 0, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found dirty edits. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 0, 10, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 10 deleted references and 30 deleted navmeshes. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 0, 0, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 30 deleted navmeshes. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 0, 10, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 10 deleted references. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 2, 0, 30));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records and 30 deleted navmeshes. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 2, 0, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 2, 10, 0));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records and 10 deleted references. info",
            message.GetContent(MessageContent::defaultLanguage).GetText());
}

TEST_P(GameTest, toMessageShouldDistinguishBetweenSingularAndPluralCounts) {
  Message message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 1, 2, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 1 ITM record, 2 deleted references and 3 deleted "
      "navmeshes. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 2, 1, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 2 ITM records, 1 deleted reference and 3 deleted "
      "navmeshes. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());

  message = Game::ToMessage(
      PluginCleaningData(0x12345678, "cleaner", info_, 3, 2, 1));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 3 ITM records, 2 deleted references and 1 deleted "
      "navmesh. info",
      message.GetContent(MessageContent::defaultLanguage).GetText());
}

TEST_P(
    GameTest,
    toMessageShouldReturnAMessageWithCountsButNoInfoStringIfInfoIsAnEmptyString) {
  Message message = Game::ToMessage(PluginCleaningData(
      0x12345678, "cleaner", std::vector<MessageContent>(), 1, 2, 3));
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ(
      "cleaner found 1 ITM record, 2 deleted references and 3 deleted "
      "navmeshes.",
      message.GetContent(MessageContent::defaultLanguage).GetText());
}

TEST_P(GameTest, initShouldNotCreateAGameFolderIfTheLootDataPathIsEmpty) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");

  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName()));
  EXPECT_NO_THROW(game.Init());

  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName()));
}

TEST_P(GameTest, initShouldCreateAGameFolderIfTheLootDataPathIsNotEmpty) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);

  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName()));
  EXPECT_NO_THROW(game.Init());

  EXPECT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName()));
}

TEST_P(GameTest, initShouldThrowIfTheLootGamePathExistsAndIsNotADirectory) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);

  std::ofstream out(lootDataPath / game.FolderName());
  out << "";
  out.close();

  ASSERT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName()));
  ASSERT_FALSE(std::filesystem::is_directory(lootDataPath / game.FolderName()));
  EXPECT_ANY_THROW(game.Init());
}

TEST_P(GameTest, initShouldNotThrowIfGameAndLocalPathsAreNotEmpty) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");

  EXPECT_NO_THROW(game.Init());
}

TEST_P(GameTest, checkInstallValidityShouldCheckThatRequirementsArePresent) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(missingEsp),
      File(blankEsp),
  });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
                Message(MessageType::error,
                        "This plugin requires \"" + missingEsp +
                            "\" to be installed, but it is missing."),
            }),
            messages);
}

TEST_P(
  GameTest,
  checkInstallValidityShouldUseDisplayNamesInRequirementMessagesIfPresent) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
    File(missingEsp, "foo"),
    File(blankEsp),
    });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
    Message(MessageType::error,
    "This plugin requires \"foo\" to be installed, but it is missing."),
    }),
    messages);
}

TEST_P(GameTest,
       checkInstallValidityShouldAddAMessageForActiveIncompatiblePlugins) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(missingEsp),
      File(masterFile),
  });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
                Message(MessageType::error,
                        "This plugin is incompatible with \"" + masterFile +
                            "\", but both files are present."),
            }),
            messages);
}

TEST_P(GameTest,
  checkInstallValidityShouldShowAMessageForIncompatibleNonPluginFilesThatArePresent) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.LoadAllInstalledPlugins(true);

  std::string incompatibleFilename = "incompatible.txt";
  std::ofstream out(dataPath / incompatibleFilename);
  out.close();

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(incompatibleFilename),
    });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
                Message(MessageType::error,
                        "This plugin is incompatible with \"" + incompatibleFilename +
                            "\", but both files are present."),
    }),
    messages);
}

TEST_P(GameTest,
  checkInstallValidityShouldUseDisplayNamesInIncompatibilityMessagesIfPresent) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
    File(missingEsp),
    File(masterFile, "foo"),
    });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
    Message(MessageType::error,
    "This plugin is incompatible with \"foo\", but both files are present."),
    }),
    messages);
}

TEST_P(GameTest, checkInstallValidityShouldGenerateMessagesFromDirtyInfo) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  const std::vector<MessageContent> info = std::vector<MessageContent>({
      MessageContent("info", MessageContent::defaultLanguage),
  });

  metadata.SetDirtyInfo({
      PluginCleaningData(blankEsmCrc, "utility1", info, 0, 1, 2),
      PluginCleaningData(0xDEADBEEF, "utility2", info, 0, 5, 10),
  });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
                Game::ToMessage(
                    PluginCleaningData(blankEsmCrc, "utility1", info, 0, 1, 2)),
                Game::ToMessage(
                    PluginCleaningData(0xDEADBEEF, "utility2", info, 0, 5, 10)),
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldCheckIfAPluginsMastersAreAllPresentAndActiveIfNoFilterTagIsPresent) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);

  auto messages = game.CheckInstallValidity(
      game.GetPlugin(blankDifferentMasterDependentEsp), metadata);
  EXPECT_EQ(std::vector<Message>({
                Message(MessageType::error,
                        "This plugin requires \"" + blankDifferentEsm +
                            "\" to be active, but it is inactive."),
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotCheckIfAPluginsMastersAreAllActiveIfAFilterTagIsPresent) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);
  metadata.SetTags({Tag("Filter")});

  auto messages = game.CheckInstallValidity(
      game.GetPlugin(blankDifferentMasterDependentEsp), metadata);
  EXPECT_TRUE(messages.empty());
}

TEST_P(
    GameTest,
    redatePluginsShouldRedatePluginsForSkyrimAndSkyrimSEAndDoNothingForOtherGames) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.Init();
  game.LoadAllInstalledPlugins(true);

  std::vector<std::pair<std::string, bool>> loadOrder = getInitialLoadOrder();

  // First set reverse timestamps to be sure.
  auto time = std::filesystem::last_write_time(dataPath / masterFile);
  for (size_t i = 1; i < loadOrder.size(); ++i) {
    if (!std::filesystem::exists(dataPath / loadOrder[i].first))
      loadOrder[i].first += ".ghost";

    std::filesystem::last_write_time(dataPath / loadOrder[i].first,
                                       time - i * std::chrono::seconds(60));
    ASSERT_EQ(
        time - i * std::chrono::seconds(60),
        std::filesystem::last_write_time(dataPath / loadOrder[i].first));
  }

  EXPECT_NO_THROW(game.RedatePlugins());

  auto interval = std::chrono::seconds(60);
  if (GetParam() != GameType::tes5 && GetParam() != GameType::tes5se)
    interval *= -1;

  for (size_t i = 0; i < loadOrder.size(); ++i) {
    EXPECT_EQ(
        time + i * interval,
        std::filesystem::last_write_time(dataPath / loadOrder[i].first));
  }
}

TEST_P(
    GameTest,
    loadAllInstalledPluginsWithHeadersOnlyTrueShouldLoadTheHeadersOfAllInstalledPlugins) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  ASSERT_NO_THROW(game.Init());

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(true));
  EXPECT_EQ(11, game.GetPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(game.GetPlugin(masterFile));
  auto plugin = game.GetPlugin(masterFile);
  EXPECT_EQ("5.0", plugin->GetVersion());

  // Check that only the header has been read.
  EXPECT_EQ(0, plugin->GetCRC());
}

TEST_P(
    GameTest,
    loadAllInstalledPluginsWithHeadersOnlyFalseShouldFullyLoadAllInstalledPlugins) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  ASSERT_NO_THROW(game.Init());

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(false));
  EXPECT_EQ(11, game.GetPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(game.GetPlugin(blankEsm));
  auto plugin = game.GetPlugin(blankEsm);
  EXPECT_EQ("5.0", plugin->GetVersion());

  // Check that not only the header has been read.
  EXPECT_EQ(blankEsmCrc, plugin->GetCRC());
}

TEST_P(GameTest,
  loadAllInstalledPluginsShouldNotGenerateWarningsForGhostedPlugins) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  ASSERT_NO_THROW(game.Init());

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_EQ(1, game.GetMessages().size());
  EXPECT_EQ("You have not sorted your load order this session.", game.GetMessages()[0].GetContent()[0].GetText());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedByDefault) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedAfterLoadingHeadersOnly) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldBeFullyLoadedAfterFullyLoadingThem) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_TRUE(game.ArePluginsFullyLoaded());
}

TEST_P(
    GameTest,
    GetActiveLoadOrderIndexShouldReturnNegativeOneForAPluginThatIsNotActive) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.Init();
  game.LoadAllInstalledPlugins(true);

  short index = game.GetActiveLoadOrderIndex(game.GetPlugin(blankEsp),
                                             game.GetLoadOrder());
  EXPECT_EQ(-1, index);
}

TEST_P(
    GameTest,
    GetActiveLoadOrderIndexShouldReturnTheLoadOrderIndexOmittingInactivePlugins) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, "");
  game.Init();
  game.LoadAllInstalledPlugins(true);

  short index = game.GetActiveLoadOrderIndex(game.GetPlugin(masterFile),
                                             game.GetLoadOrder());
  EXPECT_EQ(0, index);

  index = game.GetActiveLoadOrderIndex(game.GetPlugin(blankEsm),
                                       game.GetLoadOrder());
  EXPECT_EQ(1, index);

  index = game.GetActiveLoadOrderIndex(
      game.GetPlugin(blankDifferentMasterDependentEsp), game.GetLoadOrder());
  EXPECT_EQ(2, index);
}

TEST_P(GameTest, setLoadOrderWithoutLoadedPluginsShouldIgnoreCurrentState) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  game.Init();

  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile0));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile1));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile2));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile3));

  auto initialLoadOrder = getLoadOrder();
  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  EXPECT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                        loadOrderBackupFile0));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile1));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile2));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile3));

  auto loadOrder =
      readFileLines(lootDataPath / game.FolderName() / loadOrderBackupFile0);

  EXPECT_TRUE(loadOrder.empty());
}

TEST_P(GameTest, setLoadOrderShouldCreateABackupOfTheCurrentLoadOrder) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  game.Init();
  game.LoadAllInstalledPlugins(true);

  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile0));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile1));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile2));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile3));

  auto initialLoadOrder = getLoadOrder();
  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  EXPECT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                        loadOrderBackupFile0));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile1));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile2));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile3));

  auto loadOrder =
      readFileLines(lootDataPath / game.FolderName() / loadOrderBackupFile0);

  EXPECT_EQ(initialLoadOrder, loadOrder);
}

TEST_P(GameTest, setLoadOrderShouldRollOverExistingBackups) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  game.Init();
  game.LoadAllInstalledPlugins(true);

  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile0));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile1));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile2));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile3));

  auto initialLoadOrder = getLoadOrder();
  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  auto firstSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankPluginDependentEsp, loadOrderToSet_[9]);
  ASSERT_NE(blankDifferentMasterDependentEsp, loadOrderToSet_[10]);
  loadOrderToSet_[9] = blankPluginDependentEsp;
  loadOrderToSet_[10] = blankDifferentMasterDependentEsp;

  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  EXPECT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                        loadOrderBackupFile0));
  EXPECT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                        loadOrderBackupFile1));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile2));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile3));

  auto loadOrder =
      readFileLines(lootDataPath / game.FolderName() / loadOrderBackupFile0);
  EXPECT_EQ(firstSetLoadOrder, loadOrder);

  loadOrder =
      readFileLines(lootDataPath / game.FolderName() / loadOrderBackupFile1);
  EXPECT_EQ(initialLoadOrder, loadOrder);
}

TEST_P(GameTest, setLoadOrderShouldKeepUpToThreeBackups) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  game.Init();

  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile0));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile1));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile2));
  ASSERT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile3));

  auto initialLoadOrder = getLoadOrder();
  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  auto firstSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankPluginDependentEsp, loadOrderToSet_[9]);
  ASSERT_NE(blankDifferentMasterDependentEsp, loadOrderToSet_[10]);
  loadOrderToSet_[9] = blankPluginDependentEsp;
  loadOrderToSet_[10] = blankDifferentMasterDependentEsp;

  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  auto secondSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankMasterDependentEsp, loadOrderToSet_[7]);
  ASSERT_NE(blankEsp, loadOrderToSet_[8]);
  loadOrderToSet_[7] = blankMasterDependentEsp;
  loadOrderToSet_[8] = blankEsp;

  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  auto thirdSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankDifferentMasterDependentEsm, loadOrderToSet_[3]);
  ASSERT_NE(blankDifferentEsm, loadOrderToSet_[4]);
  loadOrderToSet_[3] = blankDifferentMasterDependentEsm;
  loadOrderToSet_[4] = blankDifferentEsm;

  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  EXPECT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                        loadOrderBackupFile0));
  EXPECT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                        loadOrderBackupFile1));
  EXPECT_TRUE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                        loadOrderBackupFile2));
  EXPECT_FALSE(std::filesystem::exists(lootDataPath / game.FolderName() /
                                         loadOrderBackupFile3));

  auto loadOrder =
      readFileLines(lootDataPath / game.FolderName() / loadOrderBackupFile0);
  EXPECT_EQ(thirdSetLoadOrder, loadOrder);

  loadOrder =
      readFileLines(lootDataPath / game.FolderName() / loadOrderBackupFile1);
  EXPECT_EQ(secondSetLoadOrder, loadOrder);

  loadOrder =
      readFileLines(lootDataPath / game.FolderName() / loadOrderBackupFile2);
  EXPECT_EQ(firstSetLoadOrder, loadOrder);
}

TEST_P(GameTest, aMessageShouldBeCachedByDefault) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);

  ASSERT_EQ(1, game.GetMessages().size());
}

TEST_P(GameTest,
       incrementLoadOrderSortCountShouldSupressTheDefaultCachedMessage) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  game.IncrementLoadOrderSortCount();

  EXPECT_TRUE(game.GetMessages().empty());
}

TEST_P(GameTest,
       decrementingLoadOrderSortCountToZeroShouldShowTheDefaultCachedMessage) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  auto expectedMessages = game.GetMessages();
  game.IncrementLoadOrderSortCount();
  game.DecrementLoadOrderSortCount();

  EXPECT_EQ(expectedMessages, game.GetMessages());
}

TEST_P(
    GameTest,
    decrementingLoadOrderSortCountThatIsAlreadyZeroShouldShowTheDefaultCachedMessage) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  auto expectedMessages = game.GetMessages();
  game.DecrementLoadOrderSortCount();

  EXPECT_EQ(expectedMessages, game.GetMessages());
}

TEST_P(
    GameTest,
    decrementingLoadOrderSortCountToANonZeroValueShouldSupressTheDefaultCachedMessage) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  auto expectedMessages = game.GetMessages();
  game.IncrementLoadOrderSortCount();
  game.IncrementLoadOrderSortCount();
  game.DecrementLoadOrderSortCount();

  EXPECT_TRUE(game.GetMessages().empty());
}

TEST_P(GameTest, appendingMessagesShouldStoreThemInTheGivenOrder) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  std::vector<Message> messages({
      Message(MessageType::say, "1"),
      Message(MessageType::error, "2"),
  });
  for (const auto& message : messages) game.AppendMessage(message);

  ASSERT_EQ(3, game.GetMessages().size());
  EXPECT_EQ(messages[0], game.GetMessages()[0]);
  EXPECT_EQ(messages[1], game.GetMessages()[1]);
}

TEST_P(GameTest, clearingMessagesShouldRemoveAllAppendedMessages) {
  GameSettings settings = GameSettings(GetParam())
    .SetGamePath(dataPath.parent_path())
    .SetGameLocalPath(localPath);
  Game game(settings, lootDataPath);
  std::vector<Message> messages({
      Message(MessageType::say, "1"),
      Message(MessageType::error, "2"),
  });
  for (const auto& message : messages) game.AppendMessage(message);

  auto previousSize = game.GetMessages().size();

  game.ClearMessages();

  EXPECT_EQ(previousSize - messages.size(), game.GetMessages().size());
}
}
}
}

#endif
