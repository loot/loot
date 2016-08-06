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

#include "backend/metadata/plugin_cleaning_data.h"

#include "backend/game/game.h"
#include "tests/backend/base_game_test.h"

namespace loot {
namespace test {
class PluginCleaningDataTest : public BaseGameTest {};

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
}

TEST_P(PluginCleaningDataTest, contentConstructorShouldStoreAllGivenData) {
  PluginCleaningData info(0x12345678, 2, 10, 30, "cleaner");
  EXPECT_EQ(0x12345678, info.CRC());
  EXPECT_EQ(2, info.ITMs());
  EXPECT_EQ(10, info.DeletedRefs());
  EXPECT_EQ(30, info.DeletedNavmeshes());
  EXPECT_EQ("cleaner", info.CleaningUtility());
}

TEST_P(PluginCleaningDataTest, asMessageShouldOutputAllNonZeroCounts) {
  Message message = PluginCleaningData(0x12345678, 2, 10, 30, "cleaner").AsMessage();
  EXPECT_EQ(Message::Type::warn, message.GetType());
  EXPECT_EQ("Contains 2 ITM records, 10 deleted references and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::Code::english).GetText());

  message = PluginCleaningData(0x12345678, 0, 0, 0, "cleaner").AsMessage();
  EXPECT_EQ(Message::Type::warn, message.GetType());
  EXPECT_EQ("Clean with cleaner.", message.ChooseContent(Language::Code::english).GetText());

  message = PluginCleaningData(0x12345678, 0, 10, 30, "cleaner").AsMessage();
  EXPECT_EQ(Message::Type::warn, message.GetType());
  EXPECT_EQ("Contains 10 deleted references and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::Code::english).GetText());

  message = PluginCleaningData(0x12345678, 0, 0, 30, "cleaner").AsMessage();
  EXPECT_EQ(Message::Type::warn, message.GetType());
  EXPECT_EQ("Contains 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::Code::english).GetText());

  message = PluginCleaningData(0x12345678, 0, 10, 0, "cleaner").AsMessage();
  EXPECT_EQ(Message::Type::warn, message.GetType());
  EXPECT_EQ("Contains 10 deleted references. Clean with cleaner.", message.ChooseContent(Language::Code::english).GetText());

  message = PluginCleaningData(0x12345678, 2, 0, 30, "cleaner").AsMessage();
  EXPECT_EQ(Message::Type::warn, message.GetType());
  EXPECT_EQ("Contains 2 ITM records and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::Code::english).GetText());

  message = PluginCleaningData(0x12345678, 2, 0, 0, "cleaner").AsMessage();
  EXPECT_EQ(Message::Type::warn, message.GetType());
  EXPECT_EQ("Contains 2 ITM records. Clean with cleaner.", message.ChooseContent(Language::Code::english).GetText());

  message = PluginCleaningData(0x12345678, 2, 10, 0, "cleaner").AsMessage();
  EXPECT_EQ(Message::Type::warn, message.GetType());
  EXPECT_EQ("Contains 2 ITM records and 10 deleted references. Clean with cleaner.", message.ChooseContent(Language::Code::english).GetText());
}

TEST_P(PluginCleaningDataTest, dirtyInfoShouldBeEqualIfCrcValuesAreEqual) {
  PluginCleaningData info1(0x12345678, 2, 10, 30, "cleaner1");
  PluginCleaningData info2(0x12345678, 4, 20, 60, "cleaner2");
  EXPECT_TRUE(info1 == info2);

  info1 = PluginCleaningData(0x12345678, 2, 10, 30, "cleaner");
  info2 = PluginCleaningData(0x87654321, 2, 10, 30, "cleaner");
  EXPECT_FALSE(info1 == info2);
}

TEST_P(PluginCleaningDataTest, LessThanOperatorShouldCompareCrcValues) {
  PluginCleaningData info1(0x12345678, 2, 10, 30, "cleaner1");
  PluginCleaningData info2(0x12345678, 4, 20, 60, "cleaner2");
  EXPECT_FALSE(info1 < info2);
  EXPECT_FALSE(info2 < info1);

  info1 = PluginCleaningData(0x12345678, 2, 10, 30, "cleaner");
  info2 = PluginCleaningData(0x87654321, 2, 10, 30, "cleaner");
  EXPECT_TRUE(info1 < info2);
  EXPECT_FALSE(info2 < info1);
}

TEST_P(PluginCleaningDataTest, evalConditionShouldBeTrueIfTheCrcGivenMatchesTheRealPluginCrc) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  PluginCleaningData dirtyInfo(blankEsmCrc, 2, 10, 30, "cleaner");
  EXPECT_TRUE(dirtyInfo.EvalCondition(game, blankEsm));
}

TEST_P(PluginCleaningDataTest, evalConditionShouldBeFalseIfTheCrcGivenDoesNotMatchTheRealPluginCrc) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  PluginCleaningData dirtyInfo(0xDEADBEEF, 2, 10, 30, "cleaner");
  EXPECT_FALSE(dirtyInfo.EvalCondition(game, blankEsm));
}

TEST_P(PluginCleaningDataTest, evalConditionShouldBeFalseIfAnEmptyPluginFilenameIsGiven) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  PluginCleaningData dirtyInfo;
  EXPECT_FALSE(dirtyInfo.EvalCondition(game, ""));
}

TEST_P(PluginCleaningDataTest, emittingAsYamlShouldOutputAllNonZeroCounts) {
  PluginCleaningData info(0x12345678, 2, 10, 30, "cleaner");
  YAML::Emitter emitter;
  emitter << info;

  EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'\nitm: 2\nudr: 10\nnav: 30", emitter.c_str());
}

TEST_P(PluginCleaningDataTest, emittingAsYamlShouldOmitAllZeroCounts) {
  PluginCleaningData info(0x12345678, 0, 0, 0, "cleaner");
  YAML::Emitter emitter;
  emitter << info;

  EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'", emitter.c_str());
}

TEST_P(PluginCleaningDataTest, encodingAsYamlShouldOmitAllZeroCountFields) {
  PluginCleaningData info(0x12345678, 0, 0, 0, "cleaner");
  YAML::Node node;
  node = info;

  EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
  EXPECT_EQ("cleaner", node["util"].as<std::string>());
  EXPECT_FALSE(node["itm"]);
  EXPECT_FALSE(node["udr"]);
  EXPECT_FALSE(node["nav"]);
}

TEST_P(PluginCleaningDataTest, encodingAsYamlShouldOutputAllNonZeroCountFields) {
  PluginCleaningData info(0x12345678, 2, 10, 30, "cleaner");
  YAML::Node node;
  node = info;

  EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
  EXPECT_EQ("cleaner", node["util"].as<std::string>());
  EXPECT_EQ(2, node["itm"].as<unsigned int>());
  EXPECT_EQ(10, node["udr"].as<unsigned int>());
  EXPECT_EQ(30, node["nav"].as<unsigned int>());
}

TEST_P(PluginCleaningDataTest, decodingFromYamlShouldLeaveMissingFieldsWithZeroValues) {
  YAML::Node node = YAML::Load("{crc: 0x12345678, util: cleaner}");
  PluginCleaningData info = node.as<PluginCleaningData>();

  EXPECT_EQ(0x12345678, info.CRC());
  EXPECT_EQ(0, info.ITMs());
  EXPECT_EQ(0, info.DeletedRefs());
  EXPECT_EQ(0, info.DeletedNavmeshes());
  EXPECT_EQ("cleaner", info.CleaningUtility());
}

TEST_P(PluginCleaningDataTest, decodingFromYamlShouldStoreAllNonZeroCounts) {
  YAML::Node node = YAML::Load("{crc: 0x12345678, util: cleaner, itm: 2, udr: 10, nav: 30}");
  PluginCleaningData info = node.as<PluginCleaningData>();

  EXPECT_EQ(0x12345678, info.CRC());
  EXPECT_EQ(2, info.ITMs());
  EXPECT_EQ(10, info.DeletedRefs());
  EXPECT_EQ(30, info.DeletedNavmeshes());
  EXPECT_EQ("cleaner", info.CleaningUtility());
}

TEST_P(PluginCleaningDataTest, decodingFromYamlScalarShouldThrow) {
  YAML::Node node = YAML::Load("scalar");

  EXPECT_ANY_THROW(node.as<PluginCleaningData>());
}

TEST_P(PluginCleaningDataTest, decodingFromYamlListShouldThrow) {
  YAML::Node node = YAML::Load("[0, 1, 2]");

  EXPECT_ANY_THROW(node.as<PluginCleaningData>());
}
}
}

#endif
