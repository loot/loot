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
<http://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TEST_BACKEND_METADATA_PLUGIN_DIRTY_INFO
#define LOOT_TEST_BACKEND_METADATA_PLUGIN_DIRTY_INFO

#include "backend/metadata/plugin_dirty_info.h"
#include "tests/base_game_test.h"

namespace loot {
    namespace test {
        class PluginDirtyInfoTest : public BaseGameTest {};

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                PluginDirtyInfoTest,
                                ::testing::Values(
                                    GameSettings::tes4));

        TEST_P(PluginDirtyInfoTest, defaultConstructorShouldLeaveAllCountsAtZeroAndTheUtilityStringEmpty) {
            PluginDirtyInfo info;
            EXPECT_EQ(0, info.CRC());
            EXPECT_EQ(0, info.ITMs());
            EXPECT_EQ(0, info.DeletedRefs());
            EXPECT_EQ(0, info.DeletedNavmeshes());
            EXPECT_TRUE(info.CleaningUtility().empty());
        }

        TEST_P(PluginDirtyInfoTest, contentConstructorShouldStoreAllGivenData) {
            PluginDirtyInfo info(0x12345678, 2, 10, 30, "cleaner");
            EXPECT_EQ(0x12345678, info.CRC());
            EXPECT_EQ(2, info.ITMs());
            EXPECT_EQ(10, info.DeletedRefs());
            EXPECT_EQ(30, info.DeletedNavmeshes());
            EXPECT_EQ("cleaner", info.CleaningUtility());
        }

        TEST_P(PluginDirtyInfoTest, asMessageShouldOutputAllNonZeroCounts) {
            Message message = PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner").AsMessage();
            EXPECT_EQ(Message::Type::warn, message.GetType());
            EXPECT_EQ("Contains 2 ITM records, 10 deleted references and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::english).Text());

            message = PluginDirtyInfo(0x12345678, 0, 0, 0, "cleaner").AsMessage();
            EXPECT_EQ(Message::Type::warn, message.GetType());
            EXPECT_EQ("Clean with cleaner.", message.ChooseContent(Language::english).Text());

            message = PluginDirtyInfo(0x12345678, 0, 10, 30, "cleaner").AsMessage();
            EXPECT_EQ(Message::Type::warn, message.GetType());
            EXPECT_EQ("Contains 10 deleted references and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::english).Text());

            message = PluginDirtyInfo(0x12345678, 0, 0, 30, "cleaner").AsMessage();
            EXPECT_EQ(Message::Type::warn, message.GetType());
            EXPECT_EQ("Contains 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::english).Text());

            message = PluginDirtyInfo(0x12345678, 0, 10, 0, "cleaner").AsMessage();
            EXPECT_EQ(Message::Type::warn, message.GetType());
            EXPECT_EQ("Contains 10 deleted references. Clean with cleaner.", message.ChooseContent(Language::english).Text());

            message = PluginDirtyInfo(0x12345678, 2, 0, 30, "cleaner").AsMessage();
            EXPECT_EQ(Message::Type::warn, message.GetType());
            EXPECT_EQ("Contains 2 ITM records and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::english).Text());

            message = PluginDirtyInfo(0x12345678, 2, 0, 0, "cleaner").AsMessage();
            EXPECT_EQ(Message::Type::warn, message.GetType());
            EXPECT_EQ("Contains 2 ITM records. Clean with cleaner.", message.ChooseContent(Language::english).Text());

            message = PluginDirtyInfo(0x12345678, 2, 10, 0, "cleaner").AsMessage();
            EXPECT_EQ(Message::Type::warn, message.GetType());
            EXPECT_EQ("Contains 2 ITM records and 10 deleted references. Clean with cleaner.", message.ChooseContent(Language::english).Text());
        }

        TEST_P(PluginDirtyInfoTest, dirtyInfoShouldBeEqualIfCrcValuesAreEqual) {
            PluginDirtyInfo info1(0x12345678, 2, 10, 30, "cleaner1");
            PluginDirtyInfo info2(0x12345678, 4, 20, 60, "cleaner2");
            EXPECT_TRUE(info1 == info2);

            info1 = PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner");
            info2 = PluginDirtyInfo(0x87654321, 2, 10, 30, "cleaner");
            EXPECT_FALSE(info1 == info2);
        }

        TEST_P(PluginDirtyInfoTest, LessThanOperatorShouldCompareCrcValues) {
            PluginDirtyInfo info1(0x12345678, 2, 10, 30, "cleaner1");
            PluginDirtyInfo info2(0x12345678, 4, 20, 60, "cleaner2");
            EXPECT_FALSE(info1 < info2);
            EXPECT_FALSE(info2 < info1);

            info1 = PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner");
            info2 = PluginDirtyInfo(0x87654321, 2, 10, 30, "cleaner");
            EXPECT_TRUE(info1 < info2);
            EXPECT_FALSE(info2 < info1);
        }

        TEST_P(PluginDirtyInfoTest, evalConditionShouldBeTrueIfTheCrcGivenMatchesTheRealPluginCrc) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            PluginDirtyInfo dirtyInfo(blankEsmCrc, 2, 10, 30, "cleaner");
            EXPECT_TRUE(dirtyInfo.EvalCondition(game, blankEsm));
        }

        TEST_P(PluginDirtyInfoTest, evalConditionShouldBeFalseIfTheCrcGivenDoesNotMatchTheRealPluginCrc) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            PluginDirtyInfo dirtyInfo(0xDEADBEEF, 2, 10, 30, "cleaner");
            EXPECT_FALSE(dirtyInfo.EvalCondition(game, blankEsm));
        }

        TEST_P(PluginDirtyInfoTest, evalConditionShouldBeFalseIfAnEmptyPluginFilenameIsGiven) {
            Game game(GetParam());
            game.SetGamePath(dataPath.parent_path());

            PluginDirtyInfo dirtyInfo;
            EXPECT_FALSE(dirtyInfo.EvalCondition(game, ""));
        }

        TEST_P(PluginDirtyInfoTest, emittingAsYamlShouldOutputAllNonZeroCounts) {
            PluginDirtyInfo info(0x12345678, 2, 10, 30, "cleaner");
            YAML::Emitter emitter;
            emitter << info;

            EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'\nitm: 2\nudr: 10\nnav: 30", emitter.c_str());
        }

        TEST_P(PluginDirtyInfoTest, emittingAsYamlShouldOmitAllZeroCounts) {
            PluginDirtyInfo info(0x12345678, 0, 0, 0, "cleaner");
            YAML::Emitter emitter;
            emitter << info;

            EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'", emitter.c_str());
        }

        TEST_P(PluginDirtyInfoTest, encodingAsYamlShouldOmitAllZeroCountFields) {
            PluginDirtyInfo info(0x12345678, 0, 0, 0, "cleaner");
            YAML::Node node;
            node = info;

            EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
            EXPECT_EQ("cleaner", node["util"].as<std::string>());
            EXPECT_FALSE(node["itm"]);
            EXPECT_FALSE(node["udr"]);
            EXPECT_FALSE(node["nav"]);
        }

        TEST_P(PluginDirtyInfoTest, encodingAsYamlShouldOutputAllNonZeroCountFields) {
            PluginDirtyInfo info(0x12345678, 2, 10, 30, "cleaner");
            YAML::Node node;
            node = info;

            EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
            EXPECT_EQ("cleaner", node["util"].as<std::string>());
            EXPECT_EQ(2, node["itm"].as<unsigned int>());
            EXPECT_EQ(10, node["udr"].as<unsigned int>());
            EXPECT_EQ(30, node["nav"].as<unsigned int>());
        }

        TEST_P(PluginDirtyInfoTest, decodingFromYamlShouldLeaveMissingFieldsWithZeroValues) {
            YAML::Node node = YAML::Load("{crc: 0x12345678, util: cleaner}");
            PluginDirtyInfo info = node.as<PluginDirtyInfo>();

            EXPECT_EQ(0x12345678, info.CRC());
            EXPECT_EQ(0, info.ITMs());
            EXPECT_EQ(0, info.DeletedRefs());
            EXPECT_EQ(0, info.DeletedNavmeshes());
            EXPECT_EQ("cleaner", info.CleaningUtility());
        }

        TEST_P(PluginDirtyInfoTest, decodingFromYamlShouldStoreAllNonZeroCounts) {
            YAML::Node node = YAML::Load("{crc: 0x12345678, util: cleaner, itm: 2, udr: 10, nav: 30}");
            PluginDirtyInfo info = node.as<PluginDirtyInfo>();

            EXPECT_EQ(0x12345678, info.CRC());
            EXPECT_EQ(2, info.ITMs());
            EXPECT_EQ(10, info.DeletedRefs());
            EXPECT_EQ(30, info.DeletedNavmeshes());
            EXPECT_EQ("cleaner", info.CleaningUtility());
        }

        TEST_P(PluginDirtyInfoTest, decodingFromYamlScalarShouldThrow) {
            YAML::Node node = YAML::Load("scalar");

            EXPECT_ANY_THROW(node.as<PluginDirtyInfo>());
        }

        TEST_P(PluginDirtyInfoTest, decodingFromYamlListShouldThrow) {
            YAML::Node node = YAML::Load("[0, 1, 2]");

            EXPECT_ANY_THROW(node.as<PluginDirtyInfo>());
        }
    }
}

#endif
