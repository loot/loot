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
#include "tests/fixtures.h"

class PluginDirtyInfo : public SkyrimTest {};

TEST_F(PluginDirtyInfo, ConstructorsAndDataAccess) {
    loot::PluginDirtyInfo info;
    EXPECT_EQ(0, info.CRC());
    EXPECT_EQ(0, info.ITMs());
    EXPECT_EQ(0, info.DeletedRefs());
    EXPECT_EQ(0, info.DeletedNavmeshes());
    EXPECT_EQ("", info.CleaningUtility());

    info = loot::PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner");
    EXPECT_EQ(0x12345678, info.CRC());
    EXPECT_EQ(2, info.ITMs());
    EXPECT_EQ(10, info.DeletedRefs());
    EXPECT_EQ(30, info.DeletedNavmeshes());
    EXPECT_EQ("cleaner", info.CleaningUtility());
}

TEST_F(PluginDirtyInfo, MessageOutput) {
    loot::Message message;

    message = loot::PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner").AsMessage();
    EXPECT_EQ(loot::Message::warn, message.Type());
    EXPECT_EQ("Contains 2 ITM records, 10 deleted references and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::any).Str());

    message = loot::PluginDirtyInfo(0x12345678, 0, 0, 0, "cleaner").AsMessage();
    EXPECT_EQ(loot::Message::warn, message.Type());
    EXPECT_EQ("Clean with cleaner.", message.ChooseContent(Language::any).Str());

    message = loot::PluginDirtyInfo(0x12345678, 0, 10, 30, "cleaner").AsMessage();
    EXPECT_EQ(loot::Message::warn, message.Type());
    EXPECT_EQ("Contains 10 deleted references and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::any).Str());

    message = loot::PluginDirtyInfo(0x12345678, 0, 0, 30, "cleaner").AsMessage();
    EXPECT_EQ(loot::Message::warn, message.Type());
    EXPECT_EQ("Contains 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::any).Str());

    message = loot::PluginDirtyInfo(0x12345678, 0, 10, 0, "cleaner").AsMessage();
    EXPECT_EQ(loot::Message::warn, message.Type());
    EXPECT_EQ("Contains 10 deleted references. Clean with cleaner.", message.ChooseContent(Language::any).Str());

    message = loot::PluginDirtyInfo(0x12345678, 2, 0, 30, "cleaner").AsMessage();
    EXPECT_EQ(loot::Message::warn, message.Type());
    EXPECT_EQ("Contains 2 ITM records and 30 deleted navmeshes. Clean with cleaner.", message.ChooseContent(Language::any).Str());

    message = loot::PluginDirtyInfo(0x12345678, 2, 0, 0, "cleaner").AsMessage();
    EXPECT_EQ(loot::Message::warn, message.Type());
    EXPECT_EQ("Contains 2 ITM records. Clean with cleaner.", message.ChooseContent(Language::any).Str());

    message = loot::PluginDirtyInfo(0x12345678, 2, 10, 0, "cleaner").AsMessage();
    EXPECT_EQ(loot::Message::warn, message.Type());
    EXPECT_EQ("Contains 2 ITM records and 10 deleted references. Clean with cleaner.", message.ChooseContent(Language::any).Str());
}

TEST_F(PluginDirtyInfo, EqualityOperator) {
    loot::PluginDirtyInfo info1, info2;
    EXPECT_TRUE(info1 == info2);

    info1 = loot::PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner1");
    info2 = loot::PluginDirtyInfo(0x12345678, 4, 20, 60, "cleaner2");
    EXPECT_TRUE(info1 == info2);

    info1 = loot::PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner");
    info2 = loot::PluginDirtyInfo(0x87654321, 2, 10, 30, "cleaner");
    EXPECT_FALSE(info1 == info2);
}

TEST_F(PluginDirtyInfo, LessThanOperator) {
    loot::PluginDirtyInfo info1, info2;
    EXPECT_FALSE(info1 < info2);
    EXPECT_FALSE(info2 < info1);

    info1 = loot::PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner1");
    info2 = loot::PluginDirtyInfo(0x12345678, 4, 20, 60, "cleaner2");
    EXPECT_FALSE(info1 < info2);
    EXPECT_FALSE(info2 < info1);

    info1 = loot::PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner");
    info2 = loot::PluginDirtyInfo(0x87654321, 2, 10, 30, "cleaner");
    EXPECT_TRUE(info1 < info2);
    EXPECT_FALSE(info2 < info1);
}

TEST_F(PluginDirtyInfo, EvalConditionTrue) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::PluginDirtyInfo dirtyInfo = loot::PluginDirtyInfo(0x24F0E2A1, 2, 10, 30, "cleaner");
    EXPECT_TRUE(dirtyInfo.EvalCondition(game, "Blank.esp"));
}

TEST_F(PluginDirtyInfo, EvalConditionFalse) {
    loot::Game game(loot::Game::tes5);
    game.SetGamePath(dataPath.parent_path());
    ASSERT_NO_THROW(game.Init(false, localPath));

    loot::PluginDirtyInfo dirtyInfo;
    EXPECT_FALSE(dirtyInfo.EvalCondition(game, ""));

    dirtyInfo = loot::PluginDirtyInfo(0xDEADBEEF, 2, 10, 30, "cleaner");
    EXPECT_FALSE(dirtyInfo.EvalCondition(game, "Blank.esp"));
}

TEST_F(PluginDirtyInfo, YamlEmitter) {
    loot::PluginDirtyInfo info;

    info = loot::PluginDirtyInfo(0x12345678, 0, 0, 0, "cleaner");
    YAML::Emitter e1;
    e1 << info;
    EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'", e1.c_str());

    info = loot::PluginDirtyInfo(0x12345678, 2, 0, 0, "cleaner");
    YAML::Emitter e2;
    e2 << info;
    EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'\nitm: 2", e2.c_str());

    info = loot::PluginDirtyInfo(0x12345678, 2, 10, 0, "cleaner");
    YAML::Emitter e3;
    e3 << info;
    EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'\nitm: 2\nudr: 10", e3.c_str());

    info = loot::PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner");
    YAML::Emitter e4;
    e4 << info;
    EXPECT_STREQ("crc: 0x12345678\nutil: 'cleaner'\nitm: 2\nudr: 10\nnav: 30", e4.c_str());
}

TEST_F(PluginDirtyInfo, YamlEncode) {
    YAML::Node node;
    loot::PluginDirtyInfo info;

    info = loot::PluginDirtyInfo(0x12345678, 0, 0, 0, "cleaner");
    node = info;
    EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
    EXPECT_EQ("cleaner", node["util"].as<std::string>());
    EXPECT_FALSE(node["itm"]);
    EXPECT_FALSE(node["udr"]);
    EXPECT_FALSE(node["nav"]);

    info = loot::PluginDirtyInfo(0x12345678, 2, 0, 0, "cleaner");
    node = info;
    EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
    EXPECT_EQ("cleaner", node["util"].as<std::string>());
    EXPECT_EQ(2, node["itm"].as<unsigned int>());
    EXPECT_FALSE(node["udr"]);
    EXPECT_FALSE(node["nav"]);

    info = loot::PluginDirtyInfo(0x12345678, 2, 10, 0, "cleaner");
    node = info;
    EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
    EXPECT_EQ("cleaner", node["util"].as<std::string>());
    EXPECT_EQ(2, node["itm"].as<unsigned int>());
    EXPECT_EQ(10, node["udr"].as<unsigned int>());
    EXPECT_FALSE(node["nav"]);

    info = loot::PluginDirtyInfo(0x12345678, 2, 10, 30, "cleaner");
    node = info;
    EXPECT_EQ(0x12345678, node["crc"].as<uint32_t>());
    EXPECT_EQ("cleaner", node["util"].as<std::string>());
    EXPECT_EQ(2, node["itm"].as<unsigned int>());
    EXPECT_EQ(10, node["udr"].as<unsigned int>());
    EXPECT_EQ(30, node["nav"].as<unsigned int>());
}

TEST_F(PluginDirtyInfo, YamlDecode) {
    YAML::Node node;
    loot::PluginDirtyInfo info;

    node = YAML::Load("{crc: 0x12345678, util: cleaner}");
    info = node.as<loot::PluginDirtyInfo>();
    EXPECT_EQ(0x12345678, info.CRC());
    EXPECT_EQ(0, info.ITMs());
    EXPECT_EQ(0, info.DeletedRefs());
    EXPECT_EQ(0, info.DeletedNavmeshes());
    EXPECT_EQ("cleaner", info.CleaningUtility());

    node = YAML::Load("{crc: 0x12345678, util: cleaner, itm: 2}");
    info = node.as<loot::PluginDirtyInfo>();
    EXPECT_EQ(0x12345678, info.CRC());
    EXPECT_EQ(2, info.ITMs());
    EXPECT_EQ(0, info.DeletedRefs());
    EXPECT_EQ(0, info.DeletedNavmeshes());
    EXPECT_EQ("cleaner", info.CleaningUtility());

    node = YAML::Load("{crc: 0x12345678, util: cleaner, itm: 2, udr: 10}");
    info = node.as<loot::PluginDirtyInfo>();
    EXPECT_EQ(0x12345678, info.CRC());
    EXPECT_EQ(2, info.ITMs());
    EXPECT_EQ(10, info.DeletedRefs());
    EXPECT_EQ(0, info.DeletedNavmeshes());
    EXPECT_EQ("cleaner", info.CleaningUtility());

    node = YAML::Load("{crc: 0x12345678, util: cleaner, itm: 2, udr: 10, nav: 30}");
    info = node.as<loot::PluginDirtyInfo>();
    EXPECT_EQ(0x12345678, info.CRC());
    EXPECT_EQ(2, info.ITMs());
    EXPECT_EQ(10, info.DeletedRefs());
    EXPECT_EQ(30, info.DeletedNavmeshes());
    EXPECT_EQ("cleaner", info.CleaningUtility());

    node = YAML::Load("scalar");
    EXPECT_ANY_THROW(node.as<loot::PluginDirtyInfo>());

    node = YAML::Load("[0, 1, 2]");
    EXPECT_ANY_THROW(node.as<loot::PluginDirtyInfo>());
}

#endif
