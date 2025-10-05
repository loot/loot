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

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <fstream>

#include "gui/state/game/game.h"
#include "gui/state/game/helpers.h"
#include "tests/common_game_test_fixture.h"
#include "tests/gui/test_helpers.h"

namespace loot::test {
class CreationClubPluginsTest : public CommonGameTestFixture {
protected:
  CreationClubPluginsTest() : CommonGameTestFixture(GameId::tes5se) {}

  std::string ccPluginName{"ccPlugin.esp"};
  std::filesystem::path cccPath{gamePath / "Skyrim.ccc"};
};

TEST_F(CreationClubPluginsTest,
       loadShouldClearCCPluginSetIfCCCFileDoesNotExist) {
  std::ofstream out(cccPath);
  out << ccPluginName;
  out.close();

  CreationClubPlugins ccPlugins;

  ccPlugins.load(GameId::tes5se, gamePath);

  EXPECT_TRUE(ccPlugins.isCreationClubPlugin(ccPluginName));

  ccPlugins.load(GameId::tes5se, gamePath);

  std::filesystem::remove(cccPath);

  ccPlugins.load(GameId::tes5se, gamePath);

  EXPECT_FALSE(ccPlugins.isCreationClubPlugin(ccPluginName));
}

TEST_F(CreationClubPluginsTest, loadShouldTrimCRLFLineEndingsFromCCCFileLines) {
  CreationClubPlugins ccPlugins;

  std::ofstream out(cccPath, std::ios::out | std::ios::binary);
  out << ccPluginName << "\r\n";
  out.close();

  ccPlugins.load(GameId::tes5se, gamePath);

  EXPECT_TRUE(ccPlugins.isCreationClubPlugin(ccPluginName));
}
}

namespace loot::gui::test {
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
      defaultGameSettings(GameSettings(GetParam(), u8"non\u00C1sciiFolder")
                              .setMinimumHeaderVersion(0.0f)
                              .setGamePath(gamePath)
                              .setGameLocalPath(localPath)) {
    // Do some preliminary locale / UTF-8 support setup, as GetMessages()
    // indirectly calls boost::locale::to_lower().
    boost::locale::generator gen;
    std::locale::global(gen("en.UTF-8"));
  }

  Game createInitialisedGame() {
    Game game(defaultGameSettings, lootDataPath, "");
    game.init();
    return game;
  }

  std::optional<std::filesystem::path> getCCCPath() {
    switch (GetParam()) {
      case GameId::tes5se:
        return gamePath / "Skyrim.ccc";
      case GameId::fo4:
        return gamePath / "Fallout4.ccc";
      case GameId::starfield:
        return gamePath / "Starfield.ccc";
      default:
        return std::nullopt;
    }
  }

  std::vector<std::filesystem::path> findLoadOrderBackups(const Game& game) {
    using std::filesystem::u8path;

    const auto parentPath = lootDataPath / u8path("games") /
                            u8path(game.getSettings().folderName()) / "backups";

    if (!std::filesystem::exists(parentPath)) {
      return {};
    }

    std::vector<std::filesystem::path> paths;

    for (const auto& entry : std::filesystem::directory_iterator(parentPath)) {
      const auto filename = entry.path().filename().u8string();
      if (boost::starts_with(filename, "loadorder.") &&
          boost::ends_with(filename, ".json")) {
        paths.push_back(entry.path());
      }
    }

    std::sort(paths.begin(), paths.end());

    return paths;
  }

  std::vector<std::string> readBackedUpLoadOrder(
      const std::filesystem::path& backupPath) {
    auto file = QFile(QString::fromStdString(backupPath.u8string()));

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    const auto content = file.readAll();
    file.close();

    const auto plugins =
        QJsonDocument::fromJson(content).object().value("loadOrder").toArray();

    std::vector<std::string> loadOrder;
    for (const auto plugin : plugins) {
      const auto pluginName = plugin.toString();
      if (!pluginName.isEmpty()) {
        loadOrder.push_back(pluginName.toStdString());
      }
    }

    return loadOrder;
  }

  std::vector<std::string> loadOrderToSet_;

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
  settings.setName("foo");
  settings.setMaster(blankEsm);
  settings.setMasterlistSource("foo");
  Game game(settings, lootDataPath, "");

  EXPECT_EQ(settings.id(), game.getSettings().id());
  EXPECT_EQ(settings.name(), game.getSettings().name());
  EXPECT_EQ(settings.folderName(), game.getSettings().folderName());
  EXPECT_EQ(settings.master(), game.getSettings().master());
  EXPECT_EQ(settings.minimumHeaderVersion(),
            game.getSettings().minimumHeaderVersion());
  EXPECT_EQ(settings.masterlistSource(), game.getSettings().masterlistSource());
  EXPECT_EQ(settings.gamePath(), game.getSettings().gamePath());
  EXPECT_EQ(settings.gameLocalPath(), game.getSettings().gameLocalPath());

  auto lootGamePath =
      lootDataPath / "games" / u8path(defaultGameSettings.folderName());
  EXPECT_EQ(lootGamePath / "masterlist.yaml", game.masterlistPath());
  EXPECT_EQ(lootGamePath / "userlist.yaml", game.userlistPath());
  EXPECT_EQ(lootGamePath / "group_node_positions.bin",
            game.groupNodePositionsPath());
  EXPECT_EQ(lootGamePath / "old_messages.json", game.oldMessagesPath());
}

TEST_P(GameTest, initShouldThrowIfGamePathWasNotGiven) {
  GameSettings settings =
      GameSettings(GetParam(), "").setGameLocalPath(localPath);
  Game game(settings, "", "");
  EXPECT_THROW(game.init(), std::invalid_argument);
}

#ifndef _WIN32
TEST_P(GameTest,
       initShouldNotThrowOnLinuxIfLocalPathWasNotGivenAndGameIsMorrowind) {
  GameSettings settings =
      GameSettings(GetParam(), "folder").setGamePath(gamePath);
  Game game(settings, lootDataPath, "");

  if (GetParam() == GameId::tes3) {
    EXPECT_NO_THROW(game.init());
  } else {
    EXPECT_THROW(game.init(), std::runtime_error);
  }
}
#else
TEST_P(GameTest, initShouldNotThrowOnWindowsIfLocalPathWasNotGiven) {
  GameSettings settings =
      GameSettings(GetParam(), "folder").setGamePath(gamePath);
  Game game(settings, lootDataPath, "");
  EXPECT_NO_THROW(game.init());
}
#endif

TEST_P(GameTest, initShouldThrowIfTheLootDataPathIsEmpty) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, "", "");

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.getSettings().folderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_THROW(game.init(), std::runtime_error);

  EXPECT_FALSE(std::filesystem::exists(lootGamePath));
}

TEST_P(GameTest, initShouldCreateAGameFolderIfTheLootDataPathIsNotEmpty) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.getSettings().folderName());
  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_NO_THROW(game.init());

  EXPECT_TRUE(std::filesystem::exists(lootGamePath));
}

TEST_P(GameTest, initShouldThrowIfTheLootGamePathExistsAndIsNotADirectory) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  std::filesystem::create_directory(lootDataPath / "games");
  auto lootGamePath =
      lootDataPath / "games" / u8path(game.getSettings().folderName());
  std::ofstream out(lootGamePath);
  out << "";
  out.close();

  ASSERT_TRUE(std::filesystem::exists(lootGamePath));
  ASSERT_FALSE(std::filesystem::is_directory(lootGamePath));
  EXPECT_ANY_THROW(game.init());
}

TEST_P(GameTest, initShouldMoveTheLegacyGameFolderIfItExists) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  auto legacyGamePath = lootDataPath / u8path(game.getSettings().folderName());
  std::filesystem::create_directory(legacyGamePath);
  std::ofstream out(legacyGamePath / "file.txt");
  out << "";
  out.close();

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.getSettings().folderName());

  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_NO_THROW(game.init());

  EXPECT_FALSE(std::filesystem::exists(legacyGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath / "file.txt"));
}

TEST_P(GameTest, initShouldNotMoveAFileWithTheSamePathAsTheLegacyGameFolder) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  auto legacyGamePath = lootDataPath / u8path(game.getSettings().folderName());
  std::ofstream out(legacyGamePath);
  out << "";
  out.close();

  auto lootGamePath =
      lootDataPath / "games" / u8path(game.getSettings().folderName());

  ASSERT_FALSE(std::filesystem::exists(lootGamePath));
  EXPECT_NO_THROW(game.init());

  EXPECT_TRUE(std::filesystem::exists(legacyGamePath));
  EXPECT_TRUE(std::filesystem::is_directory(lootGamePath));
}

TEST_P(
    GameTest,
    initShouldCreateTheGamesFolderWhenItDoesNotExistAndMigratingALegacyGameFolder) {
  using std::filesystem::u8path;
  Game game(defaultGameSettings, lootDataPath, "");

  auto legacyGamePath = lootDataPath / u8path(game.getSettings().folderName());
  std::filesystem::create_directory(legacyGamePath);
  std::ofstream out(legacyGamePath / "file.txt");
  out << "";
  out.close();

  auto lootGamesPath = lootDataPath / "games";
  std::filesystem::remove_all(lootGamesPath);

  ASSERT_FALSE(std::filesystem::exists(lootGamesPath));
  EXPECT_NO_THROW(game.init());

  auto lootGamePath = lootGamesPath / u8path(game.getSettings().folderName());

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
  EXPECT_NO_THROW(game.init());

  const auto lootGamePath =
      lootGamesPath / u8path(game.getSettings().folderName());

  EXPECT_FALSE(std::filesystem::exists(legacyGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath));
  EXPECT_TRUE(std::filesystem::exists(lootGamePath / "file.txt"));
}

TEST_P(GameTest, initShouldNotThrowIfGameAndLocalPathsAreNotEmpty) {
  Game game(defaultGameSettings, lootDataPath, "");

  EXPECT_NO_THROW(game.init());
}

TEST_P(GameTest, checkInstallValidityShouldCheckThatRequirementsArePresent) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(missingEsp),
      File(blankEsp),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                SourcedMessage{MessageType::error,
                               MessageSource::requirementMetadata,
                               "This plugin requires \"" +
                                   escapeMarkdownASCIIPunctuation(missingEsp) +
                                   "\" to be installed, but it is missing."},
            }),
            messages);
}

TEST_P(GameTest,
       checkInstallValidityShouldHandleNonAsciiFileMetadataCorrectly) {
  using std::filesystem::u8path;
  ASSERT_NO_THROW(std::filesystem::rename(
      dataPath / blankEsp, dataPath / u8path(u8"nonAsc\u00EDi.esp.ghost")));

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(nonAsciiEsp),
      File(u8"nonAsc\u00EDi.esp"),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_TRUE(messages.empty());
}

TEST_P(
    GameTest,
    checkInstallValidityShouldUseDisplayNamesInRequirementMessagesIfPresent) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(missingEsp, "foo"),
      File(blankEsp),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::requirementMetadata,
                         "This plugin requires foo to be installed, but it "
                         "is missing."},
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotDisplayMoreThanOneRequirementMessageForAnyOneDisplayName) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(missingEsp, "foo"),
      File(missingEsp, "foo", "file(\"master.esm\")"),
      File(blankEsp),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::requirementMetadata,
                         "This plugin requires foo to be installed, but it "
                         "is missing."},
      }),
      messages);
}

TEST_P(GameTest,
       checkInstallValidityShouldAddAMessageForActiveIncompatiblePlugins) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(missingEsp),
      File(masterFile),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                SourcedMessage{MessageType::error,
                               MessageSource::incompatibilityMetadata,
                               "This plugin is incompatible with \"" +
                                   escapeMarkdownASCIIPunctuation(masterFile) +
                                   "\", but both are present."},
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldShowAMessageForIncompatibleNonPluginFilesThatArePresent) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  std::string incompatibleFilename = "incompatible.txt";
  std::ofstream out(dataPath / incompatibleFilename);
  out.close();

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(incompatibleFilename),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                SourcedMessage{
                    MessageType::error,
                    MessageSource::incompatibilityMetadata,
                    "This plugin is incompatible with \"" +
                        escapeMarkdownASCIIPunctuation(incompatibleFilename) +
                        "\", but both are present."},
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldUseDisplayNamesInIncompatibilityMessagesIfPresent) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(missingEsp),
      File(masterFile, "foo"),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                SourcedMessage{MessageType::error,
                               MessageSource::incompatibilityMetadata,
                               "This plugin is incompatible with foo, but both "
                               "are present."},
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotDisplayMoreThanOneIncompatibilityMessageForAnyOneDisplayName) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  std::string incompatibleFilename = "incompatible.txt";
  std::ofstream out(dataPath / incompatibleFilename);
  out.close();

  PluginMetadata metadata(blankEsm);
  metadata.SetIncompatibilities({
      File(incompatibleFilename, "test file"),
      File(incompatibleFilename, "test file", "file(\"master.esm\")"),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::incompatibilityMetadata,
                         "This plugin is incompatible with test file, but "
                         "both are present."},
      }),
      messages);
}

TEST_P(GameTest, checkInstallValidityShouldGenerateMessagesFromDirtyInfo) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  const std::vector<MessageContent> detail = std::vector<MessageContent>({
      MessageContent("detail", MessageContent::DEFAULT_LANGUAGE),
  });

  metadata.SetDirtyInfo({
      PluginCleaningData(blankEsmCrc, "utility1", detail, 0, 1, 2),
      PluginCleaningData(0xDEADBEEF, "utility2", detail, 0, 5, 10),
  });

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
  EXPECT_EQ(std::vector<SourcedMessage>({
                toSourcedMessage(PluginCleaningData(
                                     blankEsmCrc, "utility1", detail, 0, 1, 2),
                                 MessageContent::DEFAULT_LANGUAGE),
                toSourcedMessage(PluginCleaningData(
                                     0xDEADBEEF, "utility2", detail, 0, 5, 10),
                                 MessageContent::DEFAULT_LANGUAGE),
            }),
            messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldCheckIfAPluginsMastersAreAllPresentAndActiveIfNoFilterTagIsPresent) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);

  auto messages = game.checkInstallValidity(
      *game.getPlugin(blankDifferentMasterDependentEsp), metadata, "en");
  EXPECT_EQ(
      std::vector<SourcedMessage>({
          SourcedMessage{MessageType::error,
                         MessageSource::inactiveMaster,
                         "This plugin requires \\\"" +
                             escapeMarkdownASCIIPunctuation(blankDifferentEsm) +
                             "\\\" to be active\\, but it is inactive\\."},
      }),
      messages);
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotCheckIfAPluginsMastersAreAllActiveIfAFilterTagIsPresent) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);
  metadata.SetTags({Tag("Filter")});

  auto messages = game.checkInstallValidity(
      *game.getPlugin(blankDifferentMasterDependentEsp), metadata, "en");
  EXPECT_TRUE(messages.empty());
}

TEST_P(
    GameTest,
    checkInstallValidityShouldNotCompareConditionsWhenCheckingIfAFilterTagIsPresent) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankDifferentMasterDependentEsp);
  metadata.SetTags({Tag("Filter", true, "file(\"master.esm\")")});

  auto messages = game.checkInstallValidity(
      *game.getPlugin(blankDifferentMasterDependentEsp), metadata, "en");
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

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(false);

  auto messages = game.checkInstallValidity(
      *game.getPlugin(blankEsl), PluginMetadata(blankEsl), "en");
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

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(false);

  auto messages = game.checkInstallValidity(
      *game.getPlugin(blankEsm), PluginMetadata(blankEsm), "en");
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
  Game game = createInitialisedGame();
  game.getSettings().setMinimumHeaderVersion(5.1f);
  game.loadAllInstalledPlugins(false);

  auto messages = game.checkInstallValidity(
      *game.getPlugin(blankEsm), PluginMetadata(blankEsm), "en");

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
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetGroup("missing group");

  auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
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

  loot::test::touch(gamePath / "appxmanifest.xml");
  const auto dlcPluginName = "DLCCoast.esm";
  const auto dlcDataPath = gamePath.parent_path().parent_path() /
                           "Fallout 4- Far Harbor (PC)" / "Content" / "Data";
  std::filesystem::create_directories(dlcDataPath);
  loot::test::touch(dlcDataPath / dlcPluginName);

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  PluginMetadata metadata(blankEsm);
  metadata.SetRequirements({
      File(dlcPluginName),
      File(blankEsp),
  });

  const auto messages =
      game.checkInstallValidity(*game.getPlugin(blankEsm), metadata, "en");
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

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  const auto eslMessages = game.checkInstallValidity(
      *game.getPlugin(blankEsl), PluginMetadata(blankEsl), "en");
  const auto esmMessages = game.checkInstallValidity(
      *game.getPlugin(blankEsm), PluginMetadata(blankEsm), "en");
  const auto espMessages = game.checkInstallValidity(
      *game.getPlugin(blankEsp), PluginMetadata(blankEsp), "en");

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

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

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

  EXPECT_NO_THROW(game.redatePlugins());

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

TEST_P(
    GameTest,
    loadAllInstalledPluginsWithHeadersOnlyTrueShouldLoadTheHeadersOfAllInstalledPlugins) {
  Game game = createInitialisedGame();

  EXPECT_NO_THROW(game.loadAllInstalledPlugins(true));
  EXPECT_EQ(12, game.getPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(game.getPlugin(masterFile));
  auto plugin = game.getPlugin(masterFile);
  EXPECT_EQ("5.0", plugin->GetVersion().value());

  // Check that only the header has been read.
  EXPECT_FALSE(plugin->GetCRC().has_value());
}

TEST_P(
    GameTest,
    loadAllInstalledPluginsWithHeadersOnlyFalseShouldFullyLoadAllInstalledPlugins) {
  Game game = createInitialisedGame();

  EXPECT_NO_THROW(game.loadAllInstalledPlugins(false));
  EXPECT_EQ(12, game.getPlugins().size());

  // Check that one plugin's header has been read.
  ASSERT_NO_THROW(game.getPlugin(blankEsm));
  auto plugin = game.getPlugin(blankEsm);
  EXPECT_EQ("5.0", plugin->GetVersion().value());

  // Check that not only the header has been read.
  EXPECT_EQ(blankEsmCrc, plugin->GetCRC().value());
}

TEST_P(GameTest,
       loadAllInstalledPluginsShouldNotGenerateWarningsForGhostedPlugins) {
  Game game = createInitialisedGame();

  EXPECT_NO_THROW(game.loadAllInstalledPlugins(false));

  const auto messages =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false);

  EXPECT_EQ(1, messages.size());
  EXPECT_EQ("You have not sorted your load order this session\\.",
            game.getMessages(MessageContent::DEFAULT_LANGUAGE, false)[0].text);
}

TEST_P(GameTest, loadAllInstalledPluginsShouldLoadPluginsAtExternalPaths) {
  if (GetParam() != GameId::fo4) {
    // Only FO4 has external plugins.
    return;
  }

  loot::test::touch(gamePath / "appxmanifest.xml");
  const auto dlcPluginName = "DLCCoast.esm";
  const auto dlcDataPath = gamePath.parent_path().parent_path() /
                           "Fallout 4- Far Harbor (PC)" / "Content" / "Data";
  std::filesystem::create_directories(dlcDataPath);
  std::filesystem::copy(dataPath / blankEsm, dlcDataPath / dlcPluginName);

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  const auto plugin = game.getPlugin(dlcPluginName);

  EXPECT_NE(nullptr, plugin);
}

#ifdef _WIN32
TEST_P(
    GameTest,
    loadAllInstalledPluginsShouldLoadPluginsThatAreSymlinksForOnlyOpenMWAndOblivionRemastered) {
  const auto symlinkPluginName = "Blank.symlink.esm";
  std::filesystem::create_symlink(getSourcePluginsPath() / blankEsm,
                                  dataPath / symlinkPluginName);

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  const auto plugin = game.getPlugin(symlinkPluginName);

  if (GetParam() == GameId::openmw ||
      GetParam() == GameId::oblivionRemastered) {
    EXPECT_NE(nullptr, plugin);
  } else {
    EXPECT_EQ(nullptr, plugin);
  }
}
#else
TEST_P(GameTest, loadAllInstalledPluginsShouldLoadPluginsThatAreSymlinks) {
  const auto symlinkPluginName = "Blank.symlink.esm";
  std::filesystem::create_symlink(dataPath / blankEsm,
                                  dataPath / symlinkPluginName);

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  const auto plugin = game.getPlugin(symlinkPluginName);

  EXPECT_NE(nullptr, plugin);
}
#endif

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedByDefault) {
  Game game = createInitialisedGame();

  EXPECT_FALSE(game.arePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldNotBeFullyLoadedAfterLoadingHeadersOnly) {
  Game game = createInitialisedGame();

  ASSERT_NO_THROW(game.loadAllInstalledPlugins(true));

  EXPECT_FALSE(game.arePluginsFullyLoaded());
}

TEST_P(GameTest, pluginsShouldBeFullyLoadedAfterFullyLoadingThem) {
  Game game = createInitialisedGame();

  ASSERT_NO_THROW(game.loadAllInstalledPlugins(false));

  EXPECT_TRUE(game.arePluginsFullyLoaded());
}

TEST_P(GameTest,
       supportsLightPluginsShouldReturnTrueForSkyrimVRIfSKSEPluginIsInstalled) {
  Game game = createInitialisedGame();
  const auto gameId = game.getSettings().id();

  if (gameId == GameId::tes5se || gameId == GameId::fo4 ||
      gameId == GameId::starfield) {
    EXPECT_TRUE(game.supportsLightPlugins());
  } else {
    EXPECT_FALSE(game.supportsLightPlugins());
  }

  loot::test::touch(dataPath / "SKSE" / "Plugins" / "skyrimvresl.dll");
  game.loadAllInstalledPlugins(true);

  if (gameId == GameId::tes5se || gameId == GameId::tes5vr ||
      gameId == GameId::fo4 || gameId == GameId::starfield) {
    EXPECT_TRUE(game.supportsLightPlugins());
  } else {
    EXPECT_FALSE(game.supportsLightPlugins());
  }
}

TEST_P(GameTest,
       GetActiveLoadOrderIndexShouldReturnNulloptForAPluginThatIsNotActive) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  auto index = game.getActiveLoadOrderIndex(*game.getPlugin(blankEsp),
                                            game.getLoadOrder());
  EXPECT_FALSE(index.has_value());
}

TEST_P(
    GameTest,
    GetActiveLoadOrderIndexShouldReturnTheLoadOrderIndexOmittingInactivePlugins) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  auto index = game.getActiveLoadOrderIndex(*game.getPlugin(masterFile),
                                            game.getLoadOrder());
  EXPECT_EQ(0, index);

  index = game.getActiveLoadOrderIndex(*game.getPlugin(blankEsm),
                                       game.getLoadOrder());
  EXPECT_EQ(1, index.value());

  index = game.getActiveLoadOrderIndex(
      *game.getPlugin(blankDifferentMasterDependentEsp), game.getLoadOrder());
  EXPECT_EQ(2, index.value());
}

TEST_P(
    GameTest,
    GetActiveLoadOrderIndexShouldCaseInsensitivelyCompareNonAsciiPluginNamesCorrectly) {
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  auto index = game.getActiveLoadOrderIndex(*game.getPlugin(nonAsciiEsp),
                                            {u8"non\u00E1scii.esp"});
  EXPECT_EQ(0, index.value());
}

TEST_P(GameTest, setLoadOrderWithoutLoadedPluginsShouldIgnoreCurrentState) {
  using std::filesystem::u8path;
  Game game = createInitialisedGame();

  ASSERT_TRUE(findLoadOrderBackups(game).empty());

  ASSERT_NO_THROW(game.setLoadOrder(loadOrderToSet_));

  const auto paths = findLoadOrderBackups(game);
  ASSERT_EQ(1, paths.size());

  const auto loadOrder = readBackedUpLoadOrder(paths[0]);

  EXPECT_TRUE(loadOrder.empty());
}

TEST_P(GameTest, setLoadOrderShouldCreateABackupOfTheCurrentLoadOrder) {
  using std::filesystem::u8path;
  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  ASSERT_TRUE(findLoadOrderBackups(game).empty());

  const auto initialLoadOrder = game.getLoadOrder();
  ASSERT_NO_THROW(game.setLoadOrder(loadOrderToSet_));

  const auto paths = findLoadOrderBackups(game);
  ASSERT_EQ(1, paths.size());

  const auto loadOrder = readBackedUpLoadOrder(paths[0]);

  EXPECT_EQ(initialLoadOrder, loadOrder);
}

TEST_P(GameTest, setLoadOrderShouldKeepTheTenNewestBackups) {
  using std::filesystem::u8path;

  constexpr auto SLEEP_DURATION = std::chrono::milliseconds(1);

  Game game = createInitialisedGame();

  auto backupPaths = findLoadOrderBackups(game);
  ASSERT_TRUE(backupPaths.empty());

  for (size_t i = 0; i < 10; i += 1) {
    ASSERT_NO_THROW(game.setLoadOrder(loadOrderToSet_));

    const auto newBackupPaths = findLoadOrderBackups(game);
    EXPECT_EQ(backupPaths,
              std::vector<std::filesystem::path>(newBackupPaths.begin(),
                                                 newBackupPaths.end() - 1));

    backupPaths = newBackupPaths;

    // Backup files are distinguished using millisecond-precision timestamps, so
    // make sure there's at least 1 ms between the creation of each.
    std::this_thread::sleep_for(SLEEP_DURATION);
  }

  ASSERT_NO_THROW(game.setLoadOrder(loadOrderToSet_));
  const auto newBackupPaths = findLoadOrderBackups(game);
  EXPECT_EQ(std::vector<std::filesystem::path>(backupPaths.begin() + 1,
                                               backupPaths.end()),
            std::vector<std::filesystem::path>(newBackupPaths.begin(),
                                               newBackupPaths.end() - 1));
}

TEST_P(GameTest, aMessageShouldBeCachedByDefault) {
  Game game = createInitialisedGame();

  const auto messageCount =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false).size();

  ASSERT_EQ(1, messageCount);
}

TEST_P(GameTest, sortPluginsShouldSupportPluginsAtExternalPaths) {
  if (GetParam() != GameId::fo4) {
    // Only FO4 has external plugins.
    return;
  }

  loot::test::touch(gamePath / "appxmanifest.xml");
  const auto dlcPluginName = "DLCCoast.esm";
  const auto dlcDataPath = gamePath.parent_path().parent_path() /
                           "Fallout 4- Far Harbor (PC)" / "Content" / "Data";
  std::filesystem::create_directories(dlcDataPath);
  std::filesystem::copy(dataPath / blankEsm, dlcDataPath / dlcPluginName);

  Game game = createInitialisedGame();
  game.loadAllInstalledPlugins(true);

  const auto loadOrder = game.sortPlugins();

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
  Game game = createInitialisedGame();
  game.getSortCount().increment();

  const auto messages =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false);

  EXPECT_TRUE(messages.empty());
}

TEST_P(GameTest,
       decrementingLoadOrderSortCountToZeroShouldShowTheDefaultCachedMessage) {
  Game game = createInitialisedGame();
  auto expectedMessages =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false);
  game.getSortCount().increment();
  game.getSortCount().decrement();

  EXPECT_EQ(expectedMessages,
            game.getMessages(MessageContent::DEFAULT_LANGUAGE, false));
}

TEST_P(
    GameTest,
    decrementingLoadOrderSortCountThatIsAlreadyZeroShouldShowTheDefaultCachedMessage) {
  Game game = createInitialisedGame();
  auto expectedMessages =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false);
  game.getSortCount().decrement();

  EXPECT_EQ(expectedMessages,
            game.getMessages(MessageContent::DEFAULT_LANGUAGE, false));
}

TEST_P(
    GameTest,
    decrementingLoadOrderSortCountToANonZeroValueShouldSupressTheDefaultCachedMessage) {
  Game game = createInitialisedGame();
  game.getSortCount().increment();
  game.getSortCount().increment();
  game.getSortCount().decrement();

  const auto messages =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false);

  EXPECT_TRUE(messages.empty());
}

#ifndef _WIN32
TEST_P(GameTest,
       getMessagesShouldIncludeCaseSensitivityWarningIfParameterIsTrue) {
  Game game = createInitialisedGame();
  const auto messages =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, true);

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
  Game game = createInitialisedGame();
  const auto messages =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false);

  EXPECT_EQ(1, messages.size());
  EXPECT_EQ("You have not sorted your load order this session\\.",
            messages[0].text);
}
#endif

TEST_P(GameTest, appendingMessagesShouldStoreThemInTheGivenOrder) {
  Game game = createInitialisedGame();
  std::vector<SourcedMessage> messages({
      SourcedMessage{MessageType::say, MessageSource::messageMetadata, "1"},
      SourcedMessage{MessageType::error, MessageSource::messageMetadata, "2"},
  });
  for (const auto& message : messages) {
    game.appendMessage(message);
  }

  const auto gameMessages =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false);

  ASSERT_EQ(3, gameMessages.size());
  EXPECT_EQ(messages[0], gameMessages[0]);
  EXPECT_EQ(messages[1], gameMessages[1]);
}

TEST_P(GameTest, clearingMessagesShouldRemoveAllAppendedMessages) {
  Game game = createInitialisedGame();
  std::vector<SourcedMessage> messages({
      SourcedMessage{MessageType::say, MessageSource::messageMetadata, "1"},
      SourcedMessage{MessageType::error, MessageSource::messageMetadata, "2"},
  });
  for (const auto& message : messages) {
    game.appendMessage(message);
  }

  const auto previousSize =
      game.getMessages(MessageContent::DEFAULT_LANGUAGE, false).size();

  game.clearMessages();

  EXPECT_EQ(previousSize - messages.size(),
            game.getMessages(MessageContent::DEFAULT_LANGUAGE, false).size());
}
}

#endif
