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

#ifndef LOOT_TEST_BACKEND_METADATA_MESSAGE_CONTENT
#define LOOT_TEST_BACKEND_METADATA_MESSAGE_CONTENT

#include "backend/metadata/message_content.h"
#include "tests/fixtures.h"

using loot::Language;
using loot::MessageContent;

TEST(MessageContent, ConstructorsAndDataAccess) {
    MessageContent mc;
    EXPECT_EQ("", mc.Str());
    EXPECT_EQ(Language::english, mc.Language());

    mc = MessageContent("content", Language::french);
    EXPECT_EQ("content", mc.Str());
    EXPECT_EQ(Language::french, mc.Language());
}

TEST(MessageContent, EqualityOperator) {
    MessageContent mc1, mc2;
    EXPECT_TRUE(mc1 == mc2);

    mc1 = MessageContent("content", Language::french);
    mc2 = MessageContent("Content", Language::french);
    EXPECT_TRUE(mc1 == mc2);

    mc1 = MessageContent("content1", Language::french);
    mc2 = MessageContent("content2", Language::french);
    EXPECT_FALSE(mc1 == mc2);

    mc1 = MessageContent("content", Language::english);
    mc2 = MessageContent("Content", Language::french);
    EXPECT_TRUE(mc1 == mc2);
}

TEST(MessageContent, LessThanOperator) {
    MessageContent mc1, mc2;
    EXPECT_FALSE(mc1 < mc2);
    EXPECT_FALSE(mc2 < mc1);

    mc1 = MessageContent("content", Language::french);
    mc2 = MessageContent("Content", Language::french);
    EXPECT_FALSE(mc1 < mc2);
    EXPECT_FALSE(mc2 < mc1);

    mc1 = MessageContent("content", Language::english);
    mc2 = MessageContent("content", Language::french);
    EXPECT_FALSE(mc1 < mc2);
    EXPECT_FALSE(mc2 < mc1);

    mc1 = MessageContent("content1", Language::english);
    mc2 = MessageContent("content2", Language::english);
    EXPECT_TRUE(mc1 < mc2);
    EXPECT_FALSE(mc2 < mc1);
}

TEST(MessageContent, YamlEmitter) {
    MessageContent mc("content", Language::french);
    YAML::Emitter e1;
    e1 << mc;
    EXPECT_STREQ("lang: fr\nstr: 'content'", e1.c_str());
}

TEST(MessageContent, YamlEncode) {
    YAML::Node node;

    MessageContent mc("content", Language::french);
    node = mc;
    EXPECT_EQ("content", node["str"].as<std::string>());
    EXPECT_EQ("fr", node["lang"].as<std::string>());
}

TEST(MessageContent, YamlDecode) {
    YAML::Node node = YAML::Load("{str: content, lang: de}");
    MessageContent mc = node.as<MessageContent>();
    EXPECT_EQ("content", mc.Str());
    EXPECT_EQ(Language::german, mc.Language());

    node = YAML::Load("scalar");
    EXPECT_ANY_THROW(node.as<MessageContent>());

    node = YAML::Load("[0, 1, 2]");
    EXPECT_ANY_THROW(node.as<MessageContent>());
}

#endif
