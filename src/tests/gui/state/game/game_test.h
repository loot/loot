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
#include "gui/state/game/helpers.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/test_helpers.h"

namespace loot {
namespace gui {
namespace test {
class GameTest : public loot::test::CommonGameTestFixture,
                 public testing::WithParamInterface<GameId> {
protected:
  GameTest() :
      CommonGameTestFixture(GetParam()),
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
      detail_(std::vector<MessageContent>({
          MessageContent("detail"),
      })),
      loadOrderBackupFile0("loadorder.bak.0"),
      loadOrderBackupFile1("loadorder.bak.1"),
      loadOrderBackupFile2("loadorder.bak.2"),
      loadOrderBackupFile3("loadorder.bak.3"),
      defaultGameSettings(GameSettings(GetParam(), u8"non\u00C1sciiFolder")
                              .SetMinimumHeaderVersion(0.0f)
                              .SetGamePath(dataPath.parent_path())
                              .SetGameLocalPath(localPath)) {
    // Do some preliminary locale / UTF-8 support setup, as GetMessages()
    // indirectly calls boost::locale::to_lower().
    boost::locale::generator gen;
    std::locale::global(gen("en.UTF-8"));
  }

  Game CreateInitialisedGame() {
    Game game(defaultGameSettings, lootDataPath, "");
    game.Init();
    return game;
  }

  std::optional<std::filesystem::path> GetCCCPath() {
    switch (GetParam()) {
      case GameId::tes5se:
        return dataPath.parent_path() / "Skyrim.ccc";
      case GameId::fo4:
        return dataPath.parent_path() / "Fallout4.ccc";
      case GameId::starfield:
        return dataPath.parent_path() / "Starfield.ccc";
      default:
        return std::nullopt;
    }
  }

  std::vector<std::string> loadOrderToSet_;
  const std::string loadOrderBackupFile0;
  const std::string loadOrderBackupFile1;
  const std::string loadOrderBackupFile2;
  const std::string loadOrderBackupFile3;

  const std::vector<MessageContent> detail_;

  const GameSettings defaultGameSettings;
};

// Pass an empty first argument, as it's a prefix for the test instantiation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_SUITE_P(,
                         GameTest,
                         ::testing::Values(GameId::tes3,
                                           GameId::tes4,
                                           GameId::tes5,
                                           GameId::fo3,
                                           GameId::fonv,
                                           GameId::fo4,
                                           GameId::fo4vr,
                                           GameId::tes5se,
                                           GameId::tes5vr));

TEST_P(GameTest, constructingFromGameSettingsShouldUseTheirValues) {
  using std::filesystem::u8path;
  GameSettings settings = defaultGameSettings;
  settings.SetName("foo");
  settings.SetMaster(blankEsm);
  settings.SetMasterlistSource("foo");
  Game game(settings, lootDataPath, "");

  EXPECT_EQ(settings.Id(), game.GetSettings().Id());
  EXPECT_EQ(settings.Type(), game.GetSettings().Type());
  EXPECT_EQ(settings.Name(), game.GetSettings().Name());
  EXPECT_EQ(settings.FolderName(), game.GetSettings().FolderName());
  EXPECT_EQ(settings.Master(), game.GetSettings().Master());
  EXPECT_EQ(settings.MinimumHeaderVersion(),
            game.GetSettings().MinimumHeaderVersion());
  EXPECT_EQ(settings.MasterlistSource(), game.GetSettings().MasterlistSource());
  EXPECT_EQ(settings.GamePath(), game.GetSettings().GamePath());
  EXPECT_EQ(settings.GameLocalPath(), game.GetSettings().GameLocalPath());

  auto lootGamePath =
      lootDataPath / "games" / u8path(defaultGameSettings.FolderName());
  EXPECT_EQ(lootGamePath / "masterlist.yaml", game.MasterlistPath());
  EXPECT_EQ(lootGamePath / "userlist.yaml", game.UserlistPath());
}

TEST_P(GameTest, initShouldThrowIfGamePathWasNotGiven) {
  auto settings = GameSettings(GetParam(), "").SetGameLocalPath(localPath);
  Game game(settings, "", "");
  EXPECT_THROW(game.Init(), std::invalid_argument);
}

#ifndef _WIN32
TEST_P(GameTest,
       initShouldNotThrowOnLinuxIfLocalPathWasNotGivenAndGameIsMorrowind) {
  auto settings =
      GameSettings(GetParam(), "folder").SetGamePath(dataPath.parent_path());
  Game game(settings, lootDataPath, "");

  if (GetParam() == GameId::tes3) {
    EXPECT_NO_THROW(game.Init());
  } else {
    EXPECT_THROW(game.Init(), std::system_error);
  }
}
#else
TEST_P(GameTest, initShouldNotThrowOnWindowsIfLocalPathWasNotGiven) {
  auto settings =
      GameSettings(GetParam(), "folder").SetGamePath(dataPath.parent_path());
  Game game(settings, lootDataPath, "");
  EXPECT_NO_THROW(game.Init());
}
#endif

TEST_P(GameTest, initShouldThrowIfTheLootDataPathIsEmpty) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, "", "");

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_THROW(game.Init(), std::runtime_error);

  EXPECT_FALSE(std::filesystem::exists(lootGamePath));
}

TEST_P(GameTest, initShouldCreateAGameFolderIfTheLootDataPathIsNotEmpty) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_NO_THROW(game.Init());

  EXPECT_TRUE(std::filesystem::exists(lootGamePath));
}

TEST_P(GameTest, initShouldThrowIfTheLootGamePathExistsAndIsNotADirectory) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  std::filesystem::create_directory(lootDataPath / "games");
  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());
  std::ofstream out(lootGamePath);
  out << "";
  out.close();

  ASSERT_TRUE(std::filesystem::exists(lootGamePath));
  ASSERT_FALSE(std::filesystem::is_directory(lootGamePath));
  EXPECT_ANY_THROW(game.Init());
}

TEST_P(GameTest, initShouldMoveTheLegacyGameFolderIfItExists) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  auto legacyGamePath = lootDataPath / u8path(game.GetSettings().FolderName());
  std::filesystem::create_directory(legacyGamePath);
  std::ofstream out(legacyGamePath / "file.txt");
  out << "";
  out.close();

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());

  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_NO_THROW(game.Init());

  EXPECT_FALSE(std::filesystem::exists(legacyGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath / "file.txt"));
}

TEST_P(GameTest, initShouldNotMoveAFileWithTheSamePathAsTheLegacyGameFolder) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  auto legacyGamePath = lootDataPath / u8path(game.GetSettings().FolderName());
  std::ofstream out(legacyGamePath);
  out << "";
  out.close();

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());

  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_NO_THROW(game.Init());

  EXPECT_TRUE(std::filesystem::exists(legacyGamePath));
  EXPECT_TRUE(std::filesystem::is_directory(lootGamePath));
}

TEST_P(
    GameTest,
    initShouldCreateTheGamesFolderWhenItDoesNotExistAndMigratingALegacyGameFolder) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  auto legacyGamePath = lootDataPath / u8path(game.GetSettings().FolderName());
  std::filesystem::create_directory(legacyGamePath);
  std::ofstream out(legacyGamePath / "file.txt");
  out << "";
  out.close();

  auto lootGamesPath = lootDataPath / "games";
  std::filesystem::remove_all(lootGamesPath);

  ASSERT_FALSE(std::filesystem::exists(lootGamesPath));
  EXPECT_NO_THROW(game.Init());

  auto lootGamePath = lootGamesPath / u8path(game.GetSettings().FolderName());

  EXPECT_FALSE(std::filesystem::exists(legacyGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath / "file.txt"));
}

TEST_P(GameTest, initShouldMigrateTheSkyrimSEGameFolder) {
  using std::filesystem::u8path;

  if (GetParam() != GameId::tes5se) {
    return;
  }

  Game game(defaultGameSettings, lootDataPath, "");

  const auto legacyGamePath = lootDataPath / "SkyrimSE";
  std::filesystem::create_directory(legacyGamePath);
  std::ofstream out(legacyGamePath / "file.txt");
  out << "";
  out.close();

  const auto lootGamesPath = lootDataPath / "games";
  std::filesystem::remove_all(lootGamesPath);

  ASSERT_FALSE(std::filesystem::exists(lootGamesPath));
  EXPECT_NO_THROW(game.Init());

  const auto lootGamePath =
      lootGamesPath / u8path(game.GetSettings().FolderName());

  EXPECT_FALSE(std::filesystem::exists(legacyGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath / "file.txt"));
}

TEST_P(GameTest, initShouldNotThrowIfGameAndLocalPathsAreNotEmpty) {
  Game game(defaultGameSettings, lootDataPath, "");

  EXPECT_NO_THROW(game.Init());
}

TEST_P(GameTest, checkInstallValidityShouldCheckThatRequirementsArePresent) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(missingEsp),
      File(blankEsp),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                SourcedMessage{MessageType::error,
                               MessageSource::requirementMetadata,
                               "This plugin requires \"" +
                                   EscapeMarkdownASCIIPunctuation(missingEsp) +
                                   "\" to be installed, but it is missing."},
            }),
            messages);
}

TEST_P(GameTest,
       checkInstallValidityShouldHandleNonAsciiFileMetadataCorrectly) {
  using std::filesystem::u8path;
  ASSERT_NO_THROW(std::filesystem::rename(
      dataPath / blankEsp, dataPath / u8path(u8"nonAsc\u00EDi.esp.ghost")));

  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(nonAsciiEsp),
      File(u8"nonAsc\u00EDi.esp"),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_TRUE(messages.empty());
}

TEST_P(
    GameTest,
    checkInstallValidityShouldUseDisplayNamesInRequirementMessagesIfPresent) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(missingEsp, "foo"),
      File(blankEsp),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::requirementMetadata,
                         "This plugin requires \"foo\" to be installed, but it "
                         "is missing."},
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotDisplayMoreThanOneRequirementMessageForAnyOneDisplayName) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(missingEsp, "foo"),
      File(missingEsp, "foo", "file(\"master.esm\")"),
      File(blankEsp),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::requirementMetadata,
                         "This plugin requires \"foo\" to be installed, but it "
                         "is missing."},
      }),
      messages);
}

TEST_P(GameTest,
       checkInstallValidityShouldAddAMessageForActiveIncompatiblePlugins) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(missingEsp),
      File(masterFile),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                SourcedMessage{MessageType::error,
                               MessageSource::incompatibilityMetadata,
                               "This plugin is incompatible with \"" +
                                   EscapeMarkdownASCIIPunctuation(masterFile) +
                                   "\", but both are present."},
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldShowAMessageForIncompatibleNonPluginFilesThatArePresent) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  std::string incompatibleFilename = "incompatible.txt";
  std::ofstream out(dataPath / incompatibleFilename);
  out.close();

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(incompatibleFilename),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                SourcedMessage{
                    MessageType::error,
                    MessageSource::incompatibilityMetadata,
                    "This plugin is incompatible with \"" +
                        EscapeMarkdownASCIIPunctuation(incompatibleFilename) +
                        "\", but both are present."},
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldUseDisplayNamesInIncompatibilityMessagesIfPresent) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(missingEsp),
      File(masterFile, "foo"),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::incompatibilityMetadata,
                         "This plugin is incompatible with \"foo\", but both "
                         "are present."},
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotDisplayMoreThanOneIncompatibilityMessageForAnyOneDisplayName) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  std::string incompatibleFilename = "incompatible.txt";
  std::ofstream out(dataPath / incompatibleFilename);
  out.close();

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(incompatibleFilename, "test file"),
      File(incompatibleFilename, "test file", "file(\"master.esm\")"),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::incompatibilityMetadata,
                         "This plugin is incompatible with \"test file\", but "
                         "both are present."},
      }),
      messages);
}

TEST_P(GameTest, checkInstallValidityShouldGenerateMessagesFromDirtyInfo) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  const std::vector<MessageContent> detail = std::vector<MessageContent>({
      MessageContent("detail", MessageContent::DEFAULT_LANGUAGE),
  });

  metadata.SetDirtyInfo({
      PluginCleaningData(blankEsmCrc, "utility1", detail, 0, 1, 2),
      PluginCleaningData(0xDEADBEEF, "utility2", detail, 0, 5, 10),
  });

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                ToSourcedMessage(PluginCleaningData(
                                     blankEsmCrc, "utility1", detail, 0, 1, 2),
                                 MessageContent::DEFAULT_LANGUAGE),
                ToSourcedMessage(PluginCleaningData(
                                     0xDEADBEEF, "utility2", detail, 0, 5, 10),
                                 MessageContent::DEFAULT_LANGUAGE),
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldCheckIfAPluginsMastersAreAllPresentAndActiveIfNoFilterTagIsPresent) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);

  auto messages = game.CheckInstallValidity(
      *game.GetPlugin(blankDifferentMasterDependentEsp), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::inactiveMaster,
                         "This plugin requires \\\"" +
                             EscapeMarkdownASCIIPunctuation(blankDifferentEsm) +
                             "\\\" to be active\\, but it is inactive\\."},
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotCheckIfAPluginsMastersAreAllActiveIfAFilterTagIsPresent) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);
  metadata.SetTags({Tag("Filter")});

  auto messages = game.CheckInstallValidity(
      *game.GetPlugin(blankDifferentMasterDependentEsp), metadata, "en");
  EXPECT_TRUE(messages.empty());
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotCompareConditionsWhenCheckingIfAFilterTagIsPresent) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);
  metadata.SetTags({Tag("Filter", true, "file(\"master.esm\")")});

  auto messages = game.CheckInstallValidity(
      *game.GetPlugin(blankDifferentMasterDependentEsp), metadata, "en");
  EXPECT_TRUE(messages.empty());
}

TEST_P(GameTest, checkInstallValidityShouldCheckThatAnEslIsValid) {
  if (GetParam() != GameId::tes5se) {
    return;
  }

  std::string blankEsl = "blank.esl";
  std::filesystem::copy(dataPath / blankEsm, dataPath / blankEsl);
  std::fstream out(
      dataPath / blankEsl,
      std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  out.seekp(0x10619, std::ios_base::beg);
  out.put('\xFF');
  out.close();

  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(false);

  auto messages = game.CheckInstallValidity(
      *game.GetPlugin(blankEsl), PluginMetadata(blankEsl), "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{
              MessageType::error,
              MessageSource::invalidLightPlugin,
              "This plugin contains records that have FormIDs outside the "
              "valid range for an ESL plugin\\. Using this plugin will cause "
              "irreversible damage to your game saves\\."},
      }),
      messages);
}

TEST_P(GameTest, checkInstallValidityShouldCheckThatAMediumPluginIsValid) {
  if (GetParam() != GameId::starfield) {
    return;
  }

  std::fstream out(
      dataPath / blankEsm,
      std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  out.seekp(0x09, std::ios_base::beg);
  out.put('\x04');
  out.seekp(0x10619, std::ios_base::beg);
  out.put('\xFF');
  out.close();

  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(false);

  auto messages = game.CheckInstallValidity(
      *game.GetPlugin(blankEsm), PluginMetadata(blankEsm), "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{
              MessageType::error,
              MessageSource::invalidMediumPlugin,
              "This plugin contains records that have FormIDs outside the "
              "valid range for a medium plugin\\. Using this plugin will cause "
              "irreversible damage to your game saves\\."},
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldCheckThatAPluginHeaderVersionIsNotLessThanTheMinimum) {
  Game game = CreateInitialisedGame();
  game.GetSettings().SetMinimumHeaderVersion(5.1f);
  game.LoadAllInstalledPlugins(false);

  auto messages = game.CheckInstallValidity(
      *game.GetPlugin(blankEsm), PluginMetadata(blankEsm), "en");

  std::string messageText;
  if (GetParam() == GameId::tes3) {
    messageText =
        "This plugin has a header version of 1\\.2\\, which is less than the "
        "game\\'s minimum supported header version of 5\\.1\\.";
  } else if (GetParam() == GameId::tes4) {
    messageText =
        "This plugin has a header version of 0\\.8\\, which is less than the "
        "game\\'s minimum supported header version of 5\\.1\\.";
  } else {
    messageText =
        "This plugin has a header version of 0\\.94\\, which is less than the "
        "game\\'s minimum supported header version of 5\\.1\\.";
  }

  EXPECT_EQ(std::vector<SourcedMessage>(
                {SourcedMessage{MessageType::warn,
                                MessageSource::invalidHeaderVersion,
                                messageText}}),
            messages);
}

TEST_P(GameTest, checkInstallValidityShouldCheckThatAPluginGroupExists) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetGroup("missing group");

  auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                SourcedMessage{
                    MessageType::error,
                    MessageSource::missingGroup,
                    "This plugin belongs to the group \\\"missing group\\\"\\, "
                    "which does not exist\\."},
            }),
            messages);
}

TEST_P(GameTest, checkInstallValidityShouldResolveExternalPluginPaths) {
  if (GetParam() != GameId::fo4) {
    // Only FO4 has external plugins.
    return;
  }

  loot::test::touch(dataPath.parent_path() / "appxmanifest.xml");
  const auto dlcPluginName = "DLCCoast.esm";
  const auto dlcDataPath = dataPath.parent_path().parent_path().parent_path() /
                           "Fallout 4- Far Harbor (PC)" / "Content" / "Data";
  std::filesystem::create_directories(dlcDataPath);
  loot::test::touch(dlcDataPath / dlcPluginName);

  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(dlcPluginName),
      File(blankEsp),
  });

  const auto messages =
      game.CheckInstallValidity(*game.GetPlugin(blankEsm), metadata, "en");
  EXPECT_TRUE(messages.empty());
}

TEST_P(
    GameTest,
    checkInstallValidityShouldWarnIfLightPluginIsPresentAndGameDoesNotSupportLightPlugins) {
  if (GetParam() != GameId::tes5se && GetParam() != GameId::tes5vr &&
      GetParam() != GameId::fo4 && GetParam() != GameId::fo4vr &&
      GetParam() != GameId::starfield) {
    return;
  }

  std::string blankEsl = "Blank.esl";
  std::filesystem::copy(dataPath / blankEsm, dataPath / blankEsl);

  // Light-flag esm
  std::fstream out(
      dataPath / blankEsm,
      std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  out.seekp(0x09, std::ios_base::beg);
  out.put('\x02');
  out.close();
  // Light-flag esp
  out.open(dataPath / blankEsp,
           std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  out.seekp(0x09, std::ios_base::beg);
  out.put('\x02');
  out.close();

  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  const auto eslMessages = game.CheckInstallValidity(
      *game.GetPlugin(blankEsl), PluginMetadata(blankEsl), "en");
  const auto esmMessages = game.CheckInstallValidity(
      *game.GetPlugin(blankEsm), PluginMetadata(blankEsm), "en");
  const auto espMessages = game.CheckInstallValidity(
      *game.GetPlugin(blankEsp), PluginMetadata(blankEsp), "en");

  if (GetParam() == GameId::tes5vr) {
    ASSERT_EQ(1, eslMessages.size());
    EXPECT_EQ(
        std::vector<SourcedMessage>({
            SourcedMessage{
                MessageType::error,
                MessageSource::lightPluginNotSupported,
                "\"Blank\\.esl\" is a light master, but [Skyrim VR ESL "
                "Support](https://www.nexusmods.com/skyrimspecialedition/"
                "mods/106712/) seems to be missing. Please ensure you have "
                "correctly installed [Skyrim VR ESL "
                "Support](https://www.nexusmods.com/skyrimspecialedition/"
                "mods/106712/) and all its requirements."},
        }),
        eslMessages);
    EXPECT_EQ(
        std::vector<SourcedMessage>({
            SourcedMessage{
                MessageType::error,
                MessageSource::lightPluginNotSupported,
                "\"Blank\\.esm\" is a light master, but [Skyrim VR ESL "
                "Support](https://www.nexusmods.com/skyrimspecialedition/"
                "mods/106712/) seems to be missing. Please ensure you have "
                "correctly installed [Skyrim VR ESL "
                "Support](https://www.nexusmods.com/skyrimspecialedition/"
                "mods/106712/) and all its requirements."},
        }),
        esmMessages);
    EXPECT_EQ(
        std::vector<SourcedMessage>({
            SourcedMessage{
                MessageType::error,
                MessageSource::lightPluginNotSupported,
                "\"Blank\\.esp\" is a light plugin, but [Skyrim VR ESL "
                "Support](https://www.nexusmods.com/skyrimspecialedition/"
                "mods/106712/) seems to be missing. Please ensure you have "
                "correctly installed [Skyrim VR ESL "
                "Support](https://www.nexusmods.com/skyrimspecialedition/"
                "mods/106712/) and all its requirements."},
        }),
        espMessages);
  } else if (GetParam() == GameId::fo4vr) {
    EXPECT_EQ(
        std::vector<SourcedMessage>({
            SourcedMessage{
                MessageType::error,
                MessageSource::lightPluginNotSupported,
                "\\\"Blank\\.esl\\\" is a \\.esl plugin\\, but the game does "
                "not support such plugins\\, and will not load it\\."},
        }),
        eslMessages);
    EXPECT_EQ(std::vector<SourcedMessage>({
                  SourcedMessage{
                      MessageType::warn,
                      MessageSource::lightPluginNotSupported,
                      "\\\"Blank\\.esm\\\" is flagged as a light master\\, but "
                      "the game does not support such plugins\\, and will load "
                      "it as a full master\\."},
              }),
              esmMessages);
    EXPECT_EQ(std::vector<SourcedMessage>({
                  SourcedMessage{
                      MessageType::warn,
                      MessageSource::lightPluginNotSupported,
                      "\\\"Blank\\.esp\\\" is flagged as a light plugin\\, but "
                      "the game does not support such plugins\\, and will load "
                      "it as a full plugin\\."},
              }),
              espMessages);
  } else {
    EXPECT_TRUE(eslMessages.empty());
    EXPECT_TRUE(esmMessages.empty());
    EXPECT_TRUE(espMessages.empty());
  }
}

TEST_P(
    GameTest,
    redatePluginsShouldRedatePluginsForSkyrimAndSkyrimSEAndDoNothingForOtherGames) {
  using std::filesystem::u8path;

  Game game = CreateInitialisedGame();
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
  if (GetParam() != GameId::tes5 && GetParam() != GameId::tes5se)
    interval *= -1;

  for (size_t i = 0; i < loadOrder.size(); ++i) {
    auto pluginPath = dataPath / u8path(loadOrder[i].first);
    if (!std::filesystem::exists(pluginPath))
      pluginPath += ".ghost";

    EXPECT_EQ(time + i * interval,
              std::filesystem::last_write_time(pluginPath));
  }
}

TEST_P(GameTest,
       loadCreationClubPluginNamesShouldClearCCPluginSetIfCCCFileDoesNotExist) {
  if (GetParam() != GameId::tes5se && GetParam() != GameId::fo4 &&
      GetParam() != GameId::starfield) {
    return;
  }

  Game game = CreateInitialisedGame();

  const auto pluginName = "ccPlugin.esp";

  const auto cccPath = GetCCCPath().value();
  std::ofstream out(cccPath);
  out << pluginName;
  out.close();

  game.LoadCreationClubPluginNames();

  EXPECT_TRUE(game.IsCreationClubPlugin(pluginName));

  game.LoadCreationClubPluginNames();

  std::filesystem::remove(cccPath);

  game.LoadCreationClubPluginNames();

  EXPECT_FALSE(game.IsCreationClubPlugin(pluginName));
}

TEST_P(GameTest,
       loadCreationClubPluginNamesShouldTrimCRLFLineEndingsFromCCCFileLines) {
  if (GetParam() != GameId::tes5se && GetParam() != GameId::fo4 &&
      GetParam() != GameId::starfield) {
    return;
  }

  Game game = CreateInitialisedGame();

  const auto pluginName = "ccPlugin.esp";

  const auto cccPath = GetCCCPath().value();
  std::ofstream out(cccPath, std::ios::out | std::ios::binary);
  out << pluginName << "\r\n";
  out.close();

  game.LoadCreationClubPluginNames();

  EXPECT_TRUE(game.IsCreationClubPlugin(pluginName));
}

TEST_P(
    GameTest,
    loadAllInstalledPluginsWithHeadersOnlyTrueShouldLoadTheHeadersOfAllInstalledPlugins) {
  Game game = CreateInitialisedGame();

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
  Game game = CreateInitialisedGame();

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
  Game game = CreateInitialisedGame();

  EXPECT_NO_THROW(game.LoadAllInstalledPlugins(false));

  const auto messages =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false);

  EXPECT_EQ(1, messages.size());
  EXPECT_EQ("You have not sorted your load order this session\\.",
            game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false)[0].text);
}

TEST_P(GameTest, loadAllInstalledPluginsShouldLoadPluginsAtExternalPaths) {
  if (GetParam() != GameId::fo4) {
    // Only FO4 has external plugins.
    return;
  }

  loot::test::touch(dataPath.parent_path() / "appxmanifest.xml");
  const auto dlcPluginName = "DLCCoast.esm";
  const auto dlcDataPath = dataPath.parent_path().parent_path().parent_path() /
                           "Fallout 4- Far Harbor (PC)" / "Content" / "Data";
  std::filesystem::create_directories(dlcDataPath);
  std::filesystem::copy(dataPath / blankEsm, dlcDataPath / dlcPluginName);

  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  const auto plugin = game.GetPlugin(dlcPluginName);

  EXPECT_NE(nullptr, plugin);
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedByDefault) {
  Game game = CreateInitialisedGame();

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedAfterLoadingHeadersOnly) {
  Game game = CreateInitialisedGame();

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(true));

  EXPECT_FALSE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldBeFullyLoadedAfterFullyLoadingThem) {
  Game game = CreateInitialisedGame();

  ASSERT_NO_THROW(game.LoadAllInstalledPlugins(false));

  EXPECT_TRUE(game.ArePluginsFullyLoaded());
}

TEST_P(GameTest,
       supportsLightPluginsShouldReturnTrueForSkyrimVRIfSKSEPluginIsInstalled) {
  Game game = CreateInitialisedGame();
  const auto gameType = game.GetSettings().Type();

  if (gameType == GameType::tes5se || gameType == GameType::fo4 ||
      gameType == GameType::starfield) {
    EXPECT_TRUE(game.SupportsLightPlugins());
  } else {
    EXPECT_FALSE(game.SupportsLightPlugins());
  }

  loot::test::touch(dataPath / "SKSE" / "Plugins" / "skyrimvresl.dll");
  game.LoadAllInstalledPlugins(true);

  if (gameType == GameType::tes5se || gameType == GameType::tes5vr ||
      gameType == GameType::fo4 || gameType == GameType::starfield) {
    EXPECT_TRUE(game.SupportsLightPlugins());
  } else {
    EXPECT_FALSE(game.SupportsLightPlugins());
  }
}

TEST_P(GameTest,
       GetActiveLoadOrderIndexShouldReturnNulloptForAPluginThatIsNotActive) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  auto index = game.GetActiveLoadOrderIndex(*game.GetPlugin(blankEsp),
                                            game.GetLoadOrder());
  EXPECT_FALSE(index.has_value());
}

TEST_P(
    GameTest,
    GetActiveLoadOrderIndexShouldReturnTheLoadOrderIndexOmittingInactivePlugins) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  auto index = game.GetActiveLoadOrderIndex(*game.GetPlugin(masterFile),
                                            game.GetLoadOrder());
  EXPECT_EQ(0, index);

  index = game.GetActiveLoadOrderIndex(*game.GetPlugin(blankEsm),
                                       game.GetLoadOrder());
  EXPECT_EQ(1, index.value());

  index = game.GetActiveLoadOrderIndex(
      *game.GetPlugin(blankDifferentMasterDependentEsp), game.GetLoadOrder());
  EXPECT_EQ(2, index.value());
}

TEST_P(
    GameTest,
    GetActiveLoadOrderIndexShouldCaseInsensitivelyCompareNonAsciiPluginNamesCorrectly) {
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  auto index = game.GetActiveLoadOrderIndex(*game.GetPlugin(nonAsciiEsp),
                                            {u8"non\u00E1scii.esp"});
  EXPECT_EQ(0, index.value());
}

TEST_P(GameTest, setLoadOrderWithoutLoadedPluginsShouldIgnoreCurrentState) {
  using std::filesystem::u8path;
  Game game = CreateInitialisedGame();

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());
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
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());
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
  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());
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
  Game game = CreateInitialisedGame();

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.GetSettings().FolderName());
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

  ASSERT_NE(blankDifferentEsm, loadOrderToSet_[2]);
  ASSERT_NE(blankMasterDependentEsm, loadOrderToSet_[3]);
  loadOrderToSet_[2] = blankDifferentEsm;
  loadOrderToSet_[3] = blankMasterDependentEsm;

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
  Game game = CreateInitialisedGame();

  const auto messageCount =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false).size();

  ASSERT_EQ(1, messageCount);
}

TEST_P(GameTest, sortPluginsShouldSupportPluginsAtExternalPaths) {
  if (GetParam() != GameId::fo4) {
    // Only FO4 has external plugins.
    return;
  }

  loot::test::touch(dataPath.parent_path() / "appxmanifest.xml");
  const auto dlcPluginName = "DLCCoast.esm";
  const auto dlcDataPath = dataPath.parent_path().parent_path().parent_path() /
                           "Fallout 4- Far Harbor (PC)" / "Content" / "Data";
  std::filesystem::create_directories(dlcDataPath);
  std::filesystem::copy(dataPath / blankEsm, dlcDataPath / dlcPluginName);

  Game game = CreateInitialisedGame();
  game.LoadAllInstalledPlugins(true);

  const auto loadOrder = game.SortPlugins();

  EXPECT_EQ(std::vector<std::string>({masterFile,
                                      dlcPluginName,
                                      blankEsm,
                                      blankDifferentEsm,
                                      blankMasterDependentEsm,
                                      blankDifferentMasterDependentEsm,
                                      blankEsp,
                                      blankDifferentEsp,
                                      blankMasterDependentEsp,
                                      blankDifferentMasterDependentEsp,
                                      blankPluginDependentEsp,
                                      blankDifferentPluginDependentEsp,
                                      nonAsciiEsp}),
            loadOrder);
}

TEST_P(GameTest,
       incrementLoadOrderSortCountShouldSupressTheDefaultCachedMessage) {
  Game game = CreateInitialisedGame();
  game.IncrementLoadOrderSortCount();

  const auto messages =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false);

  EXPECT_TRUE(messages.empty());
}

TEST_P(GameTest,
       decrementingLoadOrderSortCountToZeroShouldShowTheDefaultCachedMessage) {
  Game game = CreateInitialisedGame();
  auto expectedMessages =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false);
  game.IncrementLoadOrderSortCount();
  game.DecrementLoadOrderSortCount();

  EXPECT_EQ(expectedMessages,
            game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false));
}

TEST_P(
    GameTest,
    decrementingLoadOrderSortCountThatIsAlreadyZeroShouldShowTheDefaultCachedMessage) {
  Game game = CreateInitialisedGame();
  auto expectedMessages =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false);
  game.DecrementLoadOrderSortCount();

  EXPECT_EQ(expectedMessages,
            game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false));
}

TEST_P(
    GameTest,
    decrementingLoadOrderSortCountToANonZeroValueShouldSupressTheDefaultCachedMessage) {
  Game game = CreateInitialisedGame();
  game.IncrementLoadOrderSortCount();
  game.IncrementLoadOrderSortCount();
  game.DecrementLoadOrderSortCount();

  const auto messages =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false);

  EXPECT_TRUE(messages.empty());
}

#ifndef _WIN32
TEST_P(GameTest,
       getMessagesShouldIncludeCaseSensitivityWarningIfParameterIsTrue) {
  Game game = CreateInitialisedGame();
  const auto messages =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, true);

  EXPECT_EQ(3, messages.size());
  EXPECT_EQ("You have not sorted your load order this session\\.",
            messages[0].text);
  EXPECT_TRUE(boost::contains(
      messages[1].text, "is installed in a case\\-sensitive location\\."));
  EXPECT_TRUE(boost::contains(
      messages[2].text,
      "local application data is stored in a case\\-sensitive location\\."));
}

TEST_P(GameTest,
       getMessagesShouldIncludeCaseSensitivityWarningIfParameterIsFalse) {
  Game game = CreateInitialisedGame();
  const auto messages =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false);

  EXPECT_EQ(1, messages.size());
  EXPECT_EQ("You have not sorted your load order this session\\.",
            messages[0].text);
}
#endif

TEST_P(GameTest, appendingMessagesShouldStoreThemInTheGivenOrder) {
  Game game = CreateInitialisedGame();
  std::vector<SourcedMessage> messages({
      SourcedMessage{MessageType::say, MessageSource::messageMetadata, "1"},
      SourcedMessage{MessageType::error, MessageSource::messageMetadata, "2"},
  });
  for (const auto& message : messages) {
    game.AppendMessage(message);
  }

  const auto gameMessages =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false);

  ASSERT_EQ(3, gameMessages.size());
  EXPECT_EQ(messages[0], gameMessages[0]);
  EXPECT_EQ(messages[1], gameMessages[1]);
}

TEST_P(GameTest, clearingMessagesShouldRemoveAllAppendedMessages) {
  Game game = CreateInitialisedGame();
  std::vector<SourcedMessage> messages({
      SourcedMessage{MessageType::say, MessageSource::messageMetadata, "1"},
      SourcedMessage{MessageType::error, MessageSource::messageMetadata, "2"},
  });
  for (const auto& message : messages) {
    game.AppendMessage(message);
  }

  const auto previousSize =
      game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false).size();

  game.ClearMessages();

  EXPECT_EQ(previousSize - messages.size(),
            game.GetMessages(MessageContent::DEFAULT_LANGUAGE, false).size());
}
}
}
}

#endif
