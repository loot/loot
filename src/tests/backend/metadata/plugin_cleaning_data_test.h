/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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

#ifndef LOOT_TESTS_BACKEND_METADATA_PLUGIN_CLEANING_DATA
#define LOOT_TESTS_BACKEND_METADATA_PLUGIN_CLEANING_DATA

#include "loot/metadata/plugin_cleaning_data.h"

#include "backend/game/game.h"
#include "loot/yaml/plugin_cleaning_data.h"
#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class PluginCleaningDataTest : public CommonGameTestFixture {
protected:
  PluginCleaningDataTest() : info_(std::vector<MessageContent>({
    MessageContent("info", LanguageCode::english),
  })) {}

  const std::vector<MessageContent> info_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        PluginCleaningDataTest,
                        ::testing::Values(
                          GameType::tes4));

TEST_P(PluginCleaningDataTest, defaultConstructorShouldLeaveAllCountsAtZeroAndTheUtilityStringEmpty) {
  PluginCleaningData info;
  EXPECT_EQ(0, info.CRC());
  EXPECT_EQ(0, info.ITMs());
  EXPECT_EQ(0, info.DeletedRefs());
  EXPECT_EQ(0, info.DeletedNavmeshes());
  EXPECT_TRUE(info.CleaningUtility().empty());
  EXPECT_TRUE(info.Info().empty());
}

TEST_P(PluginCleaningDataTest, contentConstructorShouldStoreAllGivenData) {
  PluginCleaningData info(0x12345678, "cleaner", info_, 2, 10, 30);
  EXPECT_EQ(0x12345678, info.CRC());
  EXPECT_EQ(2, info.ITMs());
  EXPECT_EQ(10, info.DeletedRefs());
  EXPECT_EQ(30, info.DeletedNavmeshes());
  EXPECT_EQ("cleaner", info.CleaningUtility());
  EXPECT_EQ(info_, info.Info());
}

TEST_P(PluginCleaningDataTest, asMessageShouldOutputAllNonZeroCounts) {
  Message message = PluginCleaningData(0x12345678, "cleaner", info_, 2, 10, 30).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records, 10 deleted references and 30 deleted navmeshes. info", message.GetContent(LanguageCode::english).GetText());

  message = PluginCleaningData(0x12345678, "cleaner", info_, 0, 0, 0).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found dirty edits. info", message.GetContent(LanguageCode::english).GetText());

  message = PluginCleaningData(0x12345678, "cleaner", info_, 0, 10, 30).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 10 deleted references and 30 deleted navmeshes. info", message.GetContent(LanguageCode::english).GetText());

  message = PluginCleaningData(0x12345678, "cleaner", info_, 0, 0, 30).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 30 deleted navmeshes. info", message.GetContent(LanguageCode::english).GetText());

  message = PluginCleaningData(0x12345678, "cleaner", info_, 0, 10, 0).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 10 deleted references. info", message.GetContent(LanguageCode::english).GetText());

  message = PluginCleaningData(0x12345678, "cleaner", info_, 2, 0, 30).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records and 30 deleted navmeshes. info", message.GetContent(LanguageCode::english).GetText());

  message = PluginCleaningData(0x12345678, "cleaner", info_, 2, 0, 0).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records. info", message.GetContent(LanguageCode::english).GetText());

  message = PluginCleaningData(0x12345678, "cleaner", info_, 2, 10, 0).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records and 10 deleted references. info", message.GetContent(LanguageCode::english).GetText());
}

TEST_P(PluginCleaningDataTest, asMessageShouldDistinguishBetweenSingularAndPluralCounts) {
  Message message = PluginCleaningData(0x12345678, "cleaner", info_, 1, 2, 3).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 1 ITM record, 2 deleted references and 3 deleted navmeshes. info", message.GetContent(LanguageCode::english).GetText());
  
  message = PluginCleaningData(0x12345678, "cleaner", info_, 2, 1, 3).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 2 ITM records, 1 deleted reference and 3 deleted navmeshes. info", message.GetContent(LanguageCode::english).GetText());
  
  message = PluginCleaningData(0x12345678, "cleaner", info_, 3, 2, 1).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 3 ITM records, 2 deleted references and 1 deleted navmesh. info", message.GetContent(LanguageCode::english).GetText());
}

TEST_P(PluginCleaningDataTest, asMessageShouldReturnAMessageWithCountsButNoInfoStringIfInfoIsAnEmptyString) {
  Message message = PluginCleaningData(0x12345678, "cleaner", std::vector<MessageContent>(), 1, 2, 3).AsMessage();
  EXPECT_EQ(MessageType::warn, message.GetType());
  EXPECT_EQ("cleaner found 1 ITM record, 2 deleted references and 3 deleted navmeshes.", message.GetContent(LanguageCode::english).GetText());
}

TEST_P(PluginCleaningDataTest, dirtyInfoShouldBeEqualIfCrcValuesAreEqual) {
  PluginCleaningData info1(0x12345678, "cleaner1", info_, 2, 10, 30);
  PluginCleaningData info2(0x12345678, "cleaner2", info_, 4, 20, 60);
  EXPECT_TRUE(info1 == info2);

  info1 = PluginCleaningData(0x12345678, "cleaner", info_, 2, 10, 30);
  info2 = PluginCleaningData(0x87654321, "cleaner", info_, 2, 10, 30);
  EXPECT_FALSE(info1 == info2);
}

TEST_P(PluginCleaningDataTest, LessThanOperatorShouldCompareCrcValues) {
  PluginCleaningData info1(0x12345678, "cleaner1", info_, 2, 10, 30);
  PluginCleaningData info2(0x12345678, "cleaner2", info_, 4, 20, 60);
  EXPECT_FALSE(info1 < info2);
  EXPECT_FALSE(info2 < info1);

  info1 = PluginCleaningData(0x12345678, "cleaner", info_, 2, 10, 30);
  info2 = PluginCleaningData(0x87654321, "cleaner", info_, 2, 10, 30);
  EXPECT_TRUE(info1 < info2);
  EXPECT_FALSE(info2 < info1);
}

TEST_P(PluginCleaningDataTest, evalConditionShouldBeTrueIfTheCrcGivenMatchesTheRealPluginCrc) {
  Game game(GetParam(), dataPath.parent_path(), localPath);

  PluginCleaningData dirtyInfo(blankEsmCrc, "cleaner", info_, 2, 10, 30);
  EXPECT_TRUE(dirtyInfo.EvalCondition(game, blankEsm));
}

TEST_P(PluginCleaningDataTest, evalConditionShouldBeFalseIfTheCrcGivenDoesNotMatchTheRealPluginCrc) {
  Game game(GetParam(), dataPath.parent_path(), localPath);

  PluginCleaningData dirtyInfo(0xDEADBEEF, "cleaner", info_, 2, 10, 30);
  EXPECT_FALSE(dirtyInfo.EvalCondition(game, blankEsm));
}

TEST_P(PluginCleaningDataTest, evalConditionShouldBeFalseIfAnEmptyPluginFilenameIsGiven) {
  Game game(GetParam(), dataPath.parent_path(), localPath);

  PluginCleaningData dirtyInfo;
  EXPECT_FALSE(dirtyInfo.EvalCondition(game, ""));
}

TEST_P(PluginCleaningDataTest, chooseInfoShouldCreateADefaultContentObjectIfNoneExists) {
  PluginCleaningData dirtyInfo(0xDEADBEEF, "cleaner", std::vector<MessageContent>(), 2, 10, 30);
  EXPECT_EQ(MessageContent(), dirtyInfo.ChooseInfo(LanguageCode::english));
}

TEST_P(PluginCleaningDataTest, chooseInfoShouldLeaveTheContentUnchangedIfOnlyOneStringExists) {
  PluginCleaningData dirtyInfo(0xDEADBEEF, "cleaner", info_, 2, 10, 30);

  EXPECT_EQ(info_[0], dirtyInfo.ChooseInfo(LanguageCode::french));
  EXPECT_EQ(info_[0], dirtyInfo.ChooseInfo(LanguageCode::english));
}

TEST_P(PluginCleaningDataTest, chooseInfoShouldSelectTheEnglishStringIfNoStringExistsForTheGivenLanguage) {
  MessageContent content("content1", LanguageCode::english);
  std::vector<MessageContent> info({
    content,
    MessageContent("content1", LanguageCode::german),
  });
  PluginCleaningData dirtyInfo(0xDEADBEEF, "cleaner", info, 2, 10, 30);

  EXPECT_EQ(content, dirtyInfo.ChooseInfo(LanguageCode::french));
}

TEST_P(PluginCleaningDataTest, chooseInfoShouldSelectTheStringForTheGivenLanguageIfOneExists) {
  MessageContent french("content3", LanguageCode::french);
  std::vector<MessageContent> info({
    MessageContent("content1", LanguageCode::german),
    MessageContent("content2", LanguageCode::english),
    french,
  });
  PluginCleaningData dirtyInfo(0xDEADBEEF, "cleaner", info, 2, 10, 30);

  EXPECT_EQ(french, dirtyInfo.ChooseInfo(LanguageCode::french));
}

TEST_P(PluginCleaningDataTest, emittingAsYamlShouldOutputAllNonZeroCounts) {
  PluginCleaningData info(0x12345678, "cleaner", info_, 2, 10, 30);
  YAML::Emitter emitter;
  emitter << info;

  EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'\ninfo: 'info'\nitm: 2\nudr: 10\nnav: 30", emitter.c_str());
}

TEST_P(PluginCleaningDataTest, emittingAsYamlShouldOmitAllZeroCounts) {
  PluginCleaningData info(0x12345678, "cleaner", info_, 0, 0, 0);
  YAML::Emitter emitter;
  emitter << info;

  EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'\ninfo: 'info'", emitter.c_str());
}

TEST_P(PluginCleaningDataTest, encodingAsYamlShouldOmitAllZeroCountFields) {
  PluginCleaningData info(0x12345678, "cleaner", info_, 0, 0, 0);
  YAML::Node node;
  node = info;

  EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
  EXPECT_EQ("cleaner", node["util"].as<std::string>());
  EXPECT_EQ(info_, node["info"].as<std::vector<MessageContent>>());
  EXPECT_FALSE(node["itm"]);
  EXPECT_FALSE(node["udr"]);
  EXPECT_FALSE(node["nav"]);
}

TEST_P(PluginCleaningDataTest, encodingAsYamlShouldOutputAllNonZeroCountFields) {
  PluginCleaningData info(0x12345678, "cleaner", info_, 2, 10, 30);
  YAML::Node node;
  node = info;

  EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
  EXPECT_EQ("cleaner", node["util"].as<std::string>());
  EXPECT_EQ(info_, node["info"].as<std::vector<MessageContent>>());
  EXPECT_EQ(2, node["itm"].as<unsigned int>());
  EXPECT_EQ(10, node["udr"].as<unsigned int>());
  EXPECT_EQ(30, node["nav"].as<unsigned int>());
}

TEST_P(PluginCleaningDataTest, decodingFromYamlShouldLeaveMissingFieldsWithZeroValues) {
  YAML::Node node = YAML::Load("{crc: 0x12345678, util: cleaner}");
  PluginCleaningData info = node.as<PluginCleaningData>();

  EXPECT_EQ(0x12345678, info.CRC());
  EXPECT_TRUE(info.Info().empty());
  EXPECT_EQ(0, info.ITMs());
  EXPECT_EQ(0, info.DeletedRefs());
  EXPECT_EQ(0, info.DeletedNavmeshes());
  EXPECT_EQ("cleaner", info.CleaningUtility());
}

TEST_P(PluginCleaningDataTest, decodingFromYamlShouldStoreAllNonZeroCounts) {
  YAML::Node node = YAML::Load("{crc: 0x12345678, util: cleaner, info: info, itm: 2, udr: 10, nav: 30}");
  PluginCleaningData info = node.as<PluginCleaningData>();

  EXPECT_EQ(0x12345678, info.CRC());
  EXPECT_EQ(info_, info.Info());
  EXPECT_EQ(2, info.ITMs());
  EXPECT_EQ(10, info.DeletedRefs());
  EXPECT_EQ(30, info.DeletedNavmeshes());
  EXPECT_EQ("cleaner", info.CleaningUtility());
}

TEST_P(PluginCleaningDataTest, decodingFromYamlScalarShouldThrow) {
  YAML::Node node = YAML::Load("scalar");

  EXPECT_THROW(node.as<PluginCleaningData>(), YAML::RepresentationException);
}

TEST_P(PluginCleaningDataTest, decodingFromYamlListShouldThrow) {
  YAML::Node node = YAML::Load("[0, 1, 2]");

  EXPECT_THROW(node.as<PluginCleaningData>(), YAML::RepresentationException);
}
}
}

#endif
