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

#ifndef LOOT_TESTS_GUI_STATE_GAME_GAME_SETTINGS_TEST
#define LOOT_TESTS_GUI_STATE_GAME_GAME_SETTINGS_TEST

#include "gui/helpers.h"
#include "gui/state/game/game_settings.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class ShouldAllowRedatingTest : public ::testing::TestWithParam<GameId> {};

INSTANTIATE_TEST_SUITE_P(,
                         ShouldAllowRedatingTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(ShouldAllowRedatingTest, shouldReturnTrueForOnlySkyrimAndSkyrimSE) {
  const auto result = shouldAllowRedating(GetParam());
  if (GetParam() == GameId::tes5 || GetParam() == GameId::enderal ||
      GetParam() == GameId::tes5se || GetParam() == GameId::enderalse ||
      GetParam() == GameId::oblivionRemastered) {
    EXPECT_TRUE(result);
  } else {
    EXPECT_FALSE(result);
  }
}

class GetDefaultMasterlistUrlTest : public ::testing::TestWithParam<GameId> {};

INSTANTIATE_TEST_SUITE_P(,
                         GetDefaultMasterlistUrlTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(GetDefaultMasterlistUrlTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(getDefaultMasterlistUrl(GetParam()));
}

class GameSettingsTest : public CommonGameTestFixture,
                         public testing::WithParamInterface<GameId> {
protected:
  GameSettingsTest() : CommonGameTestFixture(GetParam()) {}

  GameSettings settings_;
};

// Pass an empty first argument, as it's a prefix for the test instantiation,
// but we only have the one so no prefix is necessary.
// Just test with one game because if it works for one it will work for them
// all.
INSTANTIATE_TEST_SUITE_P(,
                         GameSettingsTest,
                         ::testing::Values(GameId::tes3,
                                           GameId::tes4,
                                           GameId::tes5,
                                           GameId::fo3,
                                           GameId::fonv,
                                           GameId::fo4,
                                           GameId::tes5se));

TEST_P(
    GameSettingsTest,
    defaultConstructorShouldInitialiseIdToTes4AndAllOtherSettingsToEmptyStrings) {
  EXPECT_EQ(GameId::tes4, settings_.id());
  EXPECT_EQ("", settings_.name());
  EXPECT_EQ("", settings_.folderName());
  EXPECT_EQ("", settings_.master());
  EXPECT_EQ(0.0, settings_.minimumHeaderVersion());
  EXPECT_EQ("", settings_.masterlistSource());

  EXPECT_EQ("", settings_.gamePath());
}

TEST_P(GameSettingsTest,
       idConstructorShouldInitialiseSettingsToDefaultsForThatGame) {
  const auto folder = "folder";
  settings_ = GameSettings(GetParam(), folder);

  EXPECT_EQ(GetParam(), settings_.id());
  EXPECT_EQ(folder, settings_.folderName());

  // Repo branch changes between LOOT versions, so don't check an exact value.
  EXPECT_NE("", settings_.masterlistSource());

  switch (GetParam()) {
    case GameId::fo3:
      EXPECT_EQ("Fallout 3", settings_.name());
      EXPECT_EQ("Fallout3.esm", settings_.master());
      EXPECT_EQ(0.94f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/fallout3/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::fonv:
      EXPECT_EQ("Fallout: New Vegas", settings_.name());
      EXPECT_EQ("FalloutNV.esm", settings_.master());
      EXPECT_EQ(1.32f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/falloutnv/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::fo4:
      EXPECT_EQ("Fallout 4", settings_.name());
      EXPECT_EQ("Fallout4.esm", settings_.master());
      EXPECT_EQ(0.95f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/fallout4/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::fo4vr:
      EXPECT_EQ("Fallout 4 VR", settings_.name());
      EXPECT_EQ("Fallout4.esm", settings_.master());
      // TODO: Get the real value off someone who owns Fallout 4 VR.
      EXPECT_EQ(0.95f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/fallout4/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::tes3:
      EXPECT_EQ("TES III: Morrowind", settings_.name());
      EXPECT_EQ("Morrowind.esm", settings_.master());
      EXPECT_EQ(1.2f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/morrowind/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::tes4:
      EXPECT_EQ("TES IV: Oblivion", settings_.name());
      EXPECT_EQ("Oblivion.esm", settings_.master());
      EXPECT_EQ(0.8f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/oblivion/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::tes5:
      EXPECT_EQ("TES V: Skyrim", settings_.name());
      EXPECT_EQ("Skyrim.esm", settings_.master());
      EXPECT_EQ(0.94f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/skyrim/v0.26/masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::tes5se:
      EXPECT_EQ("TES V: Skyrim Special Edition", settings_.name());
      EXPECT_EQ("Skyrim.esm", settings_.master());
      EXPECT_EQ(1.7f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/skyrimse/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::tes5vr:
      EXPECT_EQ("TES V: Skyrim VR", settings_.name());
      EXPECT_EQ("Skyrim.esm", settings_.master());
      // TODO: Get the real value off someone who owns Skyrim VR.
      EXPECT_EQ(1.7f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/skyrimse/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    case GameId::oblivionRemastered:
      EXPECT_EQ("TES IV: Oblivion Remastered", settings_.name());
      EXPECT_EQ("Oblivion.esm", settings_.master());
      EXPECT_EQ(0.8f, settings_.minimumHeaderVersion());
      EXPECT_EQ(
          "https://raw.githubusercontent.com/loot/oblivion/v0.26/"
          "masterlist.yaml",
          settings_.masterlistSource());
      break;
    default:
      FAIL();
  }
}

TEST_P(GameSettingsTest, idConstructorShouldSetGameFolderIfGiven) {
  settings_ = GameSettings(GameId::tes5, "folder");

  EXPECT_EQ("folder", settings_.folderName());
}

TEST_P(GameSettingsTest, setNameShouldStoreGivenValue) {
  settings_.setName("name");
  EXPECT_EQ("name", settings_.name());
}

TEST_P(GameSettingsTest, setMasterShouldStoreGivenValue) {
  settings_.setMaster("master");
  EXPECT_EQ("master", settings_.master());
}

TEST_P(GameSettingsTest, setMinimumHeaderVersionShouldStoreGivenValue) {
  settings_.setMinimumHeaderVersion(1.34f);
  EXPECT_EQ(1.34f, settings_.minimumHeaderVersion());
}

TEST_P(GameSettingsTest, setMasterlistSourceShouldStoreGivenValue) {
  settings_.setMasterlistSource("url");
  EXPECT_EQ("url", settings_.masterlistSource());
}

TEST_P(GameSettingsTest, setGamePathShouldStoreGivenValue) {
  std::string pathValue = u8"p\u00E1th";

  settings_.setGamePath(std::filesystem::u8path(pathValue));
  EXPECT_EQ(pathValue, settings_.gamePath().u8string());
}

TEST_P(GameSettingsTest,
       setGameLocalFolderShouldSetLocalPathToTheNamedFolderInLocalAppData) {
  std::string folderName = u8"p\u00E1th";

  settings_.setGameLocalFolder(folderName);

  auto expectedPath =
      getLocalAppDataPath() / std::filesystem::u8path(folderName);
  EXPECT_EQ(expectedPath, settings_.gameLocalPath());
}

TEST_P(GameSettingsTest, setHiddenMessagesShouldStoreGivenValue) {
  std::vector<HiddenMessage> messages{
      HiddenMessage{std::nullopt, "general message"},
      HiddenMessage{"plugin name", "plugin message"}};

  settings_.setHiddenMessages(messages);

  EXPECT_EQ(messages, settings_.hiddenMessages());
}

TEST_P(GameSettingsTest, hideMessageShouldAppendNewHiddenMessage) {
  std::vector<HiddenMessage> messages{
      HiddenMessage{std::nullopt, "general message"},
      HiddenMessage{"plugin name", "plugin message"}};

  settings_.hideMessage("", messages[0].text);
  settings_.hideMessage(messages[1].pluginName.value(), messages[1].text);

  EXPECT_EQ(messages, settings_.hiddenMessages());
}

TEST_P(GameSettingsTest,
       hideMessageShouldMakeNoChangesIfMessageIsAlreadyHidden) {
  std::vector<HiddenMessage> messages{
      HiddenMessage{std::nullopt, "general message"},
      HiddenMessage{"plugin name", "plugin message"}};

  settings_.setHiddenMessages(messages);

  settings_.hideMessage("", messages[0].text);
  settings_.hideMessage(messages[1].pluginName.value(), messages[1].text);

  EXPECT_EQ(messages, settings_.hiddenMessages());
}

TEST_P(
    GameSettingsTest,
    hasHiddenGeneralMessageShouldBeFalseIfThereAreNoMessagesWithoutAPluginName) {
  settings_.hideMessage("plugin name", "plugin message");

  EXPECT_FALSE(settings_.hasHiddenGeneralMessages());
}

TEST_P(
    GameSettingsTest,
    hasHiddenGeneralMessageShouldBeTrueIfThereIsAMessageWithoutAPluginName) {
  settings_.hideMessage("", "plugin message");

  EXPECT_TRUE(settings_.hasHiddenGeneralMessages());
}

TEST_P(
    GameSettingsTest,
    pluginHasHiddenMessagesShouldBeFalseIfThereAreNoMessagesWithTheGivenPluginName) {
  std::vector<HiddenMessage> messages{
      HiddenMessage{std::nullopt, "general message"},
      HiddenMessage{"other plugin name", "plugin message"}};

  settings_.setHiddenMessages(messages);

  EXPECT_FALSE(settings_.pluginHasHiddenMessages("plugin name"));
}

TEST_P(GameSettingsTest,
    pluginHasHiddenMessagesShouldBeTrueIfThereIsAMessageWithTheGivenPluginName) {
  std::vector<HiddenMessage> messages{
      HiddenMessage{std::nullopt, "general message"},
      HiddenMessage{"plugin name", "plugin message"}};

  settings_.setHiddenMessages(messages);

  EXPECT_TRUE(settings_.pluginHasHiddenMessages("plugin name"));
}
}
}
#endif
