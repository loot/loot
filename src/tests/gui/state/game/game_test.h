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

#ifndef LOOT_TESTS_GUI_STATE_GAME_GAME_TEST
#define LOOT_TESTS_GUI_STATE_GAME_GAME_TEST

#include <fstream>

#include "gui/state/game/game.h"

#include "gui/state/game/game_detection_error.h"
#include "gui/state/game/helpers.h"
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
      loadOrderBackupFile3("loadorder.bak.3"),
      defaultGameSettings(GameSettings(GetParam(), u8"non\u00C1sciiFolder")
                              .SetMinimumHeaderVersion(0.0f)
                              .SetGamePath(dataPath.parent_path())
                              .SetGameLocalPath(localPath)) {}

  void TearDown() { CommonGameTestFixture::TearDown(); }

  Game CreateInitialisedGame(const std::filesystem::path& lootDataPath) {
    Game game(defaultGameSettings, lootDataPath);
    game.Init();
    return game;
  }

  std::vector<std::string> loadOrderToSet_;
  const std::string loadOrderBackupFile0;
  const std::string loadOrderBackupFile1;
  const std::string loadOrderBackupFile2;
  const std::string loadOrderBackupFile3;

  const std::vector<MessageContent> info_;

  const GameSettings defaultGameSettings;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        GameTest,
                        ::testing::Values(GameType::tes3,
                                          GameType::tes4,
                                          GameType::tes5,
                                          GameType::fo3,
                                          GameType::fonv,
                                          GameType::fo4,
                                          GameType::tes5se));

TEST_P(GameTest, constructingFromGameSettingsShouldUseTheirValues) {
  using std::filesystem::u8path;
  GameSettings settings = defaultGameSettings;
  settings.SetName("foo");
  settings.SetMaster(blankEsm);
  settings.SetRegistryKey("foo");
  settings.SetRepoURL("foo");
  settings.SetRepoBranch("foo");
  Game game(settings, lootDataPath);

  EXPECT_EQ(GetParam(), game.Type());
  EXPECT_EQ(settings.Name(), game.Name());
  EXPECT_EQ(settings.FolderName(), game.FolderName());
  EXPECT_EQ(settings.Master(), game.Master());
  EXPECT_EQ(settings.RegistryKey(), game.RegistryKey());
  EXPECT_EQ(settings.RepoURL(), game.RepoURL());
  EXPECT_EQ(settings.RepoBranch(), game.RepoBranch());

  EXPECT_EQ(settings.GamePath(), game.GamePath());
  auto lootGamePath = lootDataPath / u8path(defaultGameSettings.FolderName());
  EXPECT_EQ(lootGamePath / "masterlist.yaml", game.MasterlistPath());
  EXPECT_EQ(lootGamePath / "userlist.yaml", game.UserlistPath());
}

TEST_P(GameTest, copyConstructorShouldCopyGameData) {
  Game game1 = CreateInitialisedGame(lootDataPath);
  game1.AppendMessage(Message(MessageType::say, "1"));

  Game game2(game1);

  EXPECT_EQ(game1.MasterlistPath(), game2.MasterlistPath());
  EXPECT_EQ(game1.ArePluginsFullyLoaded(), game2.ArePluginsFullyLoaded());
  EXPECT_EQ(game1.GetMessages(), game2.GetMessages());
}

TEST_P(GameTest, assignmentOperatorShouldCopyGameData) {
  Game game1 = CreateInitialisedGame(lootDataPath);
  game1.AppendMessage(Message(MessageType::say, "1"));

  Game game2 = game1;

  EXPECT_EQ(game1.MasterlistPath(), game2.MasterlistPath());
  EXPECT_EQ(game1.ArePluginsFullyLoaded(), game2.ArePluginsFullyLoaded());
  EXPECT_EQ(game1.GetMessages(), game2.GetMessages());
}

#ifndef _WIN32
// Testing on Windows will find real game installs in the Registry, so cannot
// test autodetection fully unless on Linux.
TEST_P(GameTest, initShouldThrowOnLinuxIfGamePathWasNotGiven) {
  auto settings = GameSettings(GetParam()).SetGameLocalPath(localPath);
  Game game(settings, "");
  EXPECT_THROW(game.Init(), std::invalid_argument);
}

TEST_P(GameTest, initShouldThrowOnLinuxWasLocalPathIsNotGiven) {
  auto settings = GameSettings(GetParam()).SetGamePath(dataPath.parent_path());
  Game game(settings, lootDataPath);
  EXPECT_THROW(game.Init(), std::system_error);
}
#else
TEST_P(GameTest, initShouldNotThrowOnWindowsIfLocalPathWasNotGiven) {
  auto settings = GameSettings(GetParam()).SetGamePath(dataPath.parent_path());
  Game game(settings, lootDataPath);
  EXPECT_NO_THROW(game.Init());
}
#endif

TEST_P(GameTest, initShouldNotCreateAGameFolderIfTheLootDataPathIsEmpty) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, "");

  auto lootGamePath = lootDataPath / u8path(game.FolderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_NO_THROW(game.Init());

  EXPECT_FALSE(std::filesystem::exists(lootGamePath));
}

TEST_P(GameTest, initShouldCreateAGameFolderIfTheLootDataPathIsNotEmpty) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath);

  auto lootGamePath = lootDataPath / u8path(game.FolderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_NO_THROW(game.Init());

  EXPECT_TRUE(std::filesystem::exists(lootGamePath));
}

TEST_P(GameTest, initShouldThrowIfTheLootGamePathExistsAndIsNotADirectory) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath);

  auto lootGamePath = lootDataPath / u8path(game.FolderName());
  std::ofstream out(lootGamePath);
  out << "";
  out.close();

  ASSERT_TRUE(std::filesystem::exists(lootGamePath));
  ASSERT_FALSE(std::filesystem::is_directory(lootGamePath));
  EXPECT_ANY_THROW(game.Init());
}

TEST_P(GameTest, initShouldNotThrowIfGameAndLocalPathsAreNotEmpty) {
  Game game(defaultGameSettings, "");

  EXPECT_NO_THROW(game.Init());
}

TEST_P(GameTest, checkInstallValidityShouldCheckThatRequirementsArePresent) {
  Game game = CreateInitialisedGame("");
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

TEST_P(GameTest,
       checkInstallValidityShouldHandleNonAsciiFileMetadataCorrectly) {
  using std::filesystem::u8path;
  ASSERT_NO_THROW(std::filesystem::rename(
      dataPath / blankEsp, dataPath / u8path(u8"nonAsc\u00EDi.esp.ghost")));

  Game game = CreateInitialisedGame("");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(nonAsciiEsp),
      File(u8"nonAsc\u00EDi.esp"),
  });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_TRUE(messages.empty());
}

TEST_P(
    GameTest,
    checkInstallValidityShouldUseDisplayNamesInRequirementMessagesIfPresent) {
  Game game = CreateInitialisedGame("");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(missingEsp, "foo"),
      File(blankEsp),
  });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
                Message(MessageType::error,
                        "This plugin requires \"foo\" to be installed, but it "
                        "is missing."),
            }),
            messages);
}

TEST_P(GameTest,
       checkInstallValidityShouldAddAMessageForActiveIncompatiblePlugins) {
  Game game = CreateInitialisedGame("");
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

TEST_P(
    GameTest,
    checkInstallValidityShouldShowAMessageForIncompatibleNonPluginFilesThatArePresent) {
  Game game = CreateInitialisedGame("");
  game.LoadAllInstalledPlugins(true);

  std::string incompatibleFilename = "incompatible.txt";
  std::ofstream out(dataPath / incompatibleFilename);
  out.close();

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(incompatibleFilename),
  });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(
      std::vector<Message>({
          Message(MessageType::error,
                  "This plugin is incompatible with \"" + incompatibleFilename +
                      "\", but both files are present."),
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldUseDisplayNamesInIncompatibilityMessagesIfPresent) {
  Game game = CreateInitialisedGame("");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(missingEsp),
      File(masterFile, "foo"),
  });

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
                Message(MessageType::error,
                        "This plugin is incompatible with \"foo\", but both "
                        "files are present."),
            }),
            messages);
}

TEST_P(GameTest, checkInstallValidityShouldGenerateMessagesFromDirtyInfo) {
  Game game = CreateInitialisedGame("");
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
  EXPECT_EQ(
      std::vector<Message>({
          ToMessage(PluginCleaningData(blankEsmCrc, "utility1", info, 0, 1, 2)),
          ToMessage(PluginCleaningData(0xDEADBEEF, "utility2", info, 0, 5, 10)),
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldCheckIfAPluginsMastersAreAllPresentAndActiveIfNoFilterTagIsPresent) {
  Game game = CreateInitialisedGame("");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);

  auto messages = game.CheckInstallValidity(
      game.GetPlugin(blankDifferentMasterDependentEsp), metadata);
  EXPECT_EQ(std::vector<Message>({
                Message(MessageType::error,
                        "This plugin requires \"" +
                            EscapeMarkdownSpecialChars(blankDifferentEsm) +
                            "\" to be active, but it is inactive\\."),
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotCheckIfAPluginsMastersAreAllActiveIfAFilterTagIsPresent) {
  Game game = CreateInitialisedGame("");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);
  metadata.SetTags({Tag("Filter")});

  auto messages = game.CheckInstallValidity(
      game.GetPlugin(blankDifferentMasterDependentEsp), metadata);
  EXPECT_TRUE(messages.empty());
}

TEST_P(GameTest, checkInstallValidityShouldCheckThatAnEslIsValid) {
  if (GetParam() != GameType::tes5se) {
    return;
  }

  std::string blankEsl = "blank.esl";
  std::filesystem::copy(dataPath / blankEsm, dataPath / blankEsl);
  std::fstream out(
      dataPath / blankEsl,
      std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  out.seekp(0x10619, std::ios_base::beg);
  out.put(0xFF);
  out.close();

  Game game = CreateInitialisedGame("");
  game.LoadAllInstalledPlugins(false);

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsl),
                                            PluginMetadata(blankEsl));
  EXPECT_EQ(
      std::vector<Message>({
          Message(
              MessageType::error,
              "This plugin contains records that have FormIDs outside the "
              "valid range for an ESL plugin\\. Using this plugin will cause "
              "irreversible damage to your game saves\\."),
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldCheckThatAPluginHeaderVersionIsNotLessThanTheMinimum) {
  Game game = CreateInitialisedGame("");
  game.SetMinimumHeaderVersion(5.1f);
  game.LoadAllInstalledPlugins(false);

  auto messages = game.CheckInstallValidity(game.GetPlugin(blankEsm),
                                            PluginMetadata(blankEsm));

  std::string messageText;
  if (GetParam() == GameType::tes3) {
    messageText =
        "This plugin has a header version of 1\\.2, which is less than the "
        "game's minimum supported header version of 5\\.1\\.";
  } else if (GetParam() == GameType::tes4) {
    messageText =
        "This plugin has a header version of 0\\.8, which is less than the "
        "game's minimum supported header version of 5\\.1\\.";
  } else {
    messageText =
        "This plugin has a header version of 0\\.94, which is less than the "
        "game's minimum supported header version of 5\\.1\\.";
  }

  EXPECT_EQ(std::vector<Message>({Message(MessageType::error, messageText)}),
            messages);
}

TEST_P(GameTest,
  checkInstallValidityShouldCheckThatAPluginGroupExists) {
  Game game = CreateInitialisedGame("");
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetGroup("missing group");

  auto messages = game.CheckInstallValidity(
    game.GetPlugin(blankEsm), metadata);
  EXPECT_EQ(std::vector<Message>({
                Message(MessageType::error,
                        "This plugin belongs to the group \"missing group\", which does not exist\\."),
    }),
    messages);
}

TEST_P(
    GameTest,
    redatePluginsShouldRedatePluginsForSkyrimAndSkyrimSEAndDoNothingForOtherGames) {
  using std::filesystem::u8path;

  Game game = CreateInitialisedGame("");
  game.Init();
  game.LoadAllInstalledPlugins(true);

  std::vector<std::pair<std::string, bool>> loadOrder = getInitialLoadOrder();

  // First set reverse timestamps to be sure.
  auto time = std::filesystem::last_write_time(dataPath / u8path(masterFile));
  for (size_t i = 1; i < loadOrder.size(); ++i) {
    auto pluginPath = dataPath / u8path(loadOrder[i].first);
    if (!std::filesystem::exists(pluginPath))
      pluginPath += ".ghost";

    std::filesystem::last_write_time(pluginPath,
                                     time - i * std::chrono::seconds(60));
    ASSERT_EQ(time - i * std::chrono::seconds(60),
              std::filesystem::last_write_time(pluginPath));
  }

  EXPECT_NO_THROW(game.RedatePlugins());

  auto interval = std::chrono::seconds(60);
  if (GetParam() != GameType::tes5 && GetParam() != GameType::tes5se)
    interval *= -1;

  for (size_t i = 0; i < loadOrder.size(); ++i) {
    auto pluginPath = dataPath / u8path(loadOrder[i].first);
    if (!std::filesystem::exists(pluginPath))
      pluginPath += ".ghost";

    EXPECT_EQ(time + i * interval,
              std::filesystem::last_write_time(pluginPath));
  }
}

TEST_P(
    GameTest,
    loadAllInstalledPluginsWithHeadersOnlyTrueShouldLoadTheHeadersOfAllInstalledPlugins) {
  Game game = CreateInitialisedGame("");
  ASSERT_NO_THROW(game.Init());

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(true));
  EXPECT_EQ(12, game.GetPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(game.GetPlugin(masterFile));
  auto plugin = game.GetPlugin(masterFile);
  EXPECT_EQ("5.0", plugin->GetVersion().value());

  // Check that only the header has been read.
  EXPECT_FALSE(plugin->GetCRC().has_value());
}

TEST_P(
    GameTest,
    loadAllInstalledPluginsWithHeadersOnlyFalseShouldFullyLoadAllInstalledPlugins) {
  Game game = CreateInitialisedGame("");
  ASSERT_NO_THROW(game.Init());

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(false));
  EXPECT_EQ(12, game.GetPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(game.GetPlugin(blankEsm));
  auto plugin = game.GetPlugin(blankEsm);
  EXPECT_EQ("5.0", plugin->GetVersion().value());

  // Check that not only the header has been read.
  EXPECT_EQ(blankEsmCrc, plugin->GetCRC().value());
}

TEST_P(GameTest,
       loadAllInstalledPluginsShouldNotGenerateWarningsForGhostedPlugins) {
  Game game = CreateInitialisedGame("");
  ASSERT_NO_THROW(game.Init());

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_EQ(1, game.GetMessages().size());
  EXPECT_EQ("You have not sorted your load order this session\\.",
            game.GetMessages()[0].GetContent()[0].GetText());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedByDefault) {
  Game game = CreateInitialisedGame("");

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedAfterLoadingHeadersOnly) {
  Game game = CreateInitialisedGame("");

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldBeFullyLoadedAfterFullyLoadingThem) {
  Game game = CreateInitialisedGame("");

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_TRUE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest,
       GetActiveLoadOrderIndexShouldReturnNulloptForAPluginThatIsNotActive) {
  Game game(defaultGameSettings, "");
  game.Init();
  game.LoadAllInstalledPlugins(true);

  auto index = game.GetActiveLoadOrderIndex(game.GetPlugin(blankEsp),
                                            game.GetLoadOrder());
  EXPECT_FALSE(index.has_value());
}

TEST_P(
    GameTest,
    GetActiveLoadOrderIndexShouldReturnTheLoadOrderIndexOmittingInactivePlugins) {
  Game game(defaultGameSettings, "");
  game.Init();
  game.LoadAllInstalledPlugins(true);

  auto index = game.GetActiveLoadOrderIndex(game.GetPlugin(masterFile),
                                            game.GetLoadOrder());
  EXPECT_EQ(0, index);

  index = game.GetActiveLoadOrderIndex(game.GetPlugin(blankEsm),
                                       game.GetLoadOrder());
  EXPECT_EQ(1, index.value());

  index = game.GetActiveLoadOrderIndex(
      game.GetPlugin(blankDifferentMasterDependentEsp), game.GetLoadOrder());
  EXPECT_EQ(2, index.value());
}

TEST_P(
    GameTest,
    GetActiveLoadOrderIndexShouldCaseInsensitivelyCompareNonAsciiPluginNamesCorrectly) {
  Game game(defaultGameSettings, "");
  game.Init();
  game.LoadAllInstalledPlugins(true);

  auto index = game.GetActiveLoadOrderIndex(game.GetPlugin(nonAsciiEsp),
                                            {u8"non\u00E1scii.esp"});
  EXPECT_EQ(0, index.value());
}

TEST_P(GameTest, setLoadOrderWithoutLoadedPluginsShouldIgnoreCurrentState) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath);
  game.Init();

  auto lootGamePath = lootDataPath / u8path(game.FolderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile0));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile1));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile2));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile3));

  auto initialLoadOrder = getLoadOrder();
  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  EXPECT_TRUE(std::filesystem::exists(lootGamePath / loadOrderBackupFile0));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile1));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile2));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile3));

  auto loadOrder = readFileLines(lootGamePath / loadOrderBackupFile0);

  EXPECT_TRUE(loadOrder.empty());
}

TEST_P(GameTest, setLoadOrderShouldCreateABackupOfTheCurrentLoadOrder) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath);
  game.Init();
  game.LoadAllInstalledPlugins(true);

  auto lootGamePath = lootDataPath / u8path(game.FolderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile0));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile1));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile2));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile3));

  auto initialLoadOrder = getLoadOrder();
  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  EXPECT_TRUE(std::filesystem::exists(lootGamePath / loadOrderBackupFile0));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile1));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile2));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile3));

  auto loadOrder = readFileLines(lootGamePath / loadOrderBackupFile0);

  EXPECT_EQ(initialLoadOrder, loadOrder);
}

TEST_P(GameTest, setLoadOrderShouldRollOverExistingBackups) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath);
  game.Init();
  game.LoadAllInstalledPlugins(true);

  auto lootGamePath = lootDataPath / u8path(game.FolderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile0));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile1));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile2));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile3));

  auto initialLoadOrder = getLoadOrder();
  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  auto firstSetLoadOrder = loadOrderToSet_;

  ASSERT_NE(blankPluginDependentEsp, loadOrderToSet_[9]);
  ASSERT_NE(blankDifferentMasterDependentEsp, loadOrderToSet_[10]);
  loadOrderToSet_[9] = blankPluginDependentEsp;
  loadOrderToSet_[10] = blankDifferentMasterDependentEsp;

  ASSERT_NO_THROW(game.SetLoadOrder(loadOrderToSet_));

  EXPECT_TRUE(std::filesystem::exists(lootGamePath / loadOrderBackupFile0));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath / loadOrderBackupFile1));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile2));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile3));

  auto loadOrder = readFileLines(lootGamePath / loadOrderBackupFile0);
  EXPECT_EQ(firstSetLoadOrder, loadOrder);

  loadOrder = readFileLines(lootGamePath / loadOrderBackupFile1);
  EXPECT_EQ(initialLoadOrder, loadOrder);
}

TEST_P(GameTest, setLoadOrderShouldKeepUpToThreeBackups) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath);
  game.Init();

  auto lootGamePath = lootDataPath / u8path(game.FolderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile0));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile1));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile2));
  ASSERT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile3));

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

  EXPECT_TRUE(std::filesystem::exists(lootGamePath / loadOrderBackupFile0));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath / loadOrderBackupFile1));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath / loadOrderBackupFile2));
  EXPECT_FALSE(std::filesystem::exists(lootGamePath / loadOrderBackupFile3));

  auto loadOrder = readFileLines(lootGamePath / loadOrderBackupFile0);
  EXPECT_EQ(thirdSetLoadOrder, loadOrder);

  loadOrder = readFileLines(lootGamePath / loadOrderBackupFile1);
  EXPECT_EQ(secondSetLoadOrder, loadOrder);

  loadOrder = readFileLines(lootGamePath / loadOrderBackupFile2);
  EXPECT_EQ(firstSetLoadOrder, loadOrder);
}

TEST_P(GameTest, aMessageShouldBeCachedByDefault) {
  Game game = CreateInitialisedGame(lootDataPath);

  ASSERT_EQ(1, game.GetMessages().size());
}

TEST_P(GameTest,
       incrementLoadOrderSortCountShouldSupressTheDefaultCachedMessage) {
  Game game = CreateInitialisedGame(lootDataPath);
  game.IncrementLoadOrderSortCount();

  EXPECT_TRUE(game.GetMessages().empty());
}

TEST_P(GameTest,
       decrementingLoadOrderSortCountToZeroShouldShowTheDefaultCachedMessage) {
  Game game = CreateInitialisedGame(lootDataPath);
  auto expectedMessages = game.GetMessages();
  game.IncrementLoadOrderSortCount();
  game.DecrementLoadOrderSortCount();

  EXPECT_EQ(expectedMessages, game.GetMessages());
}

TEST_P(
    GameTest,
    decrementingLoadOrderSortCountThatIsAlreadyZeroShouldShowTheDefaultCachedMessage) {
  Game game = CreateInitialisedGame(lootDataPath);
  auto expectedMessages = game.GetMessages();
  game.DecrementLoadOrderSortCount();

  EXPECT_EQ(expectedMessages, game.GetMessages());
}

TEST_P(
    GameTest,
    decrementingLoadOrderSortCountToANonZeroValueShouldSupressTheDefaultCachedMessage) {
  Game game = CreateInitialisedGame(lootDataPath);
  auto expectedMessages = game.GetMessages();
  game.IncrementLoadOrderSortCount();
  game.IncrementLoadOrderSortCount();
  game.DecrementLoadOrderSortCount();

  EXPECT_TRUE(game.GetMessages().empty());
}

TEST_P(GameTest, appendingMessagesShouldStoreThemInTheGivenOrder) {
  Game game = CreateInitialisedGame(lootDataPath);
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
  Game game = CreateInitialisedGame(lootDataPath);
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
