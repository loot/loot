/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2015    WrinklyNinja

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

#ifndef LOOT_TEST_BACKEND_METADATA_TAG
#define LOOT_TEST_BACKEND_METADATA_TAG

#include "backend/metadata/tag.h"
#include "tests/fixtures.h"

using loot::Tag;

TEST(Tag, ConstructorsAndDataAccess) {
    Tag tag;
    EXPECT_EQ("", tag.Name());
    EXPECT_TRUE(tag.IsAddition());
    EXPECT_EQ("", tag.Condition());

    tag = Tag("name");
    EXPECT_EQ("name", tag.Name());
    EXPECT_TRUE(tag.IsAddition());
    EXPECT_EQ("", tag.Condition());

    tag = Tag("name", false);
    EXPECT_EQ("name", tag.Name());
    EXPECT_FALSE(tag.IsAddition());
    EXPECT_EQ("", tag.Condition());

    // Not a valid condition, but not evaluating it in this test.
    tag = Tag("name", false, "condition");
    EXPECT_EQ("name", tag.Name());
    EXPECT_FALSE(tag.IsAddition());
    EXPECT_EQ("condition", tag.Condition());
}

TEST(Tag, EqualityOperator) {
    Tag tag1, tag2;
    EXPECT_TRUE(tag1 == tag2);

    // Not valid conditions, but not evaluating them in this test.
    tag1 = Tag("name", true, "condition1");
    tag2 = Tag("name", true, "condition2");
    EXPECT_TRUE(tag1 == tag2);

    tag1 = Tag("name");
    tag2 = Tag("Name");
    EXPECT_TRUE(tag1 == tag2);

    tag1 = Tag("name", true);
    tag2 = Tag("name", false);
    EXPECT_FALSE(tag1 == tag2);

    tag1 = Tag("name1");
    tag2 = Tag("name2");
    EXPECT_FALSE(tag1 == tag2);
}

TEST(Tag, LessThanOperator) {
    Tag tag1, tag2;
    EXPECT_FALSE(tag1 < tag2);
    EXPECT_FALSE(tag2 < tag1);

    tag1 = Tag("name", true, "condition1");
    tag2 = Tag("name", true, "condition2");
    EXPECT_FALSE(tag1 < tag2);
    EXPECT_FALSE(tag2 < tag1);

    tag1 = Tag("name");
    tag2 = Tag("Name");
    EXPECT_FALSE(tag1 < tag2);
    EXPECT_FALSE(tag2 < tag1);

    tag1 = Tag("name1");
    tag2 = Tag("name2");
    EXPECT_TRUE(tag1 < tag2);
    EXPECT_FALSE(tag2 < tag1);

    tag1 = Tag("name", true);
    tag2 = Tag("name", false);
    EXPECT_TRUE(tag1 < tag2);
    EXPECT_FALSE(tag2 < tag1);
}

TEST(Tag, YamlEmitter) {
    Tag tag("name1");
    YAML::Emitter e1;
    e1 << tag;
    EXPECT_STREQ("name1", e1.c_str());

    tag = Tag("name1", false);
    YAML::Emitter e2;
    e2 << tag;
    EXPECT_STREQ("-name1", e2.c_str());

    tag = Tag("name1", true, "condition1");
    YAML::Emitter e3;
    e3 << tag;
    EXPECT_STREQ("name: name1\ncondition: 'condition1'", e3.c_str());

    tag = Tag("name1", false, "condition1");
    YAML::Emitter e4;
    e4 << tag;
    EXPECT_STREQ("name: -name1\ncondition: 'condition1'", e4.c_str());
}

TEST(Tag, YamlEncode) {
    YAML::Node node;

    Tag tag("name1");
    node = tag;
    EXPECT_EQ("name1", node["name"].as<std::string>());
    EXPECT_FALSE(node["condition"]);

    tag = Tag("name1", false);
    node = tag;
    EXPECT_EQ("-name1", node["name"].as<std::string>());
    EXPECT_FALSE(node["condition"]);

    tag = Tag("name1", false, "condition1");
    node = tag;
    EXPECT_EQ("-name1", node["name"].as<std::string>());
    EXPECT_EQ("condition1", node["condition"].as<std::string>());
}

TEST(Tag, YamlDecode) {
    YAML::Node node;
    Tag tag;

    node = YAML::Load("name1");
    tag = node.as<Tag>();
    EXPECT_EQ("name1", tag.Name());
    EXPECT_TRUE(tag.IsAddition());
    EXPECT_EQ("", tag.Condition());

    node = YAML::Load("-name1");
    tag = node.as<Tag>();
    EXPECT_EQ("name1", tag.Name());
    EXPECT_FALSE(tag.IsAddition());
    EXPECT_EQ("", tag.Condition());

    node = YAML::Load("{name: -name1}");
    tag = node.as<Tag>();
    EXPECT_EQ("name1", tag.Name());
    EXPECT_FALSE(tag.IsAddition());
    EXPECT_EQ("", tag.Condition());

    node = YAML::Load("{name: name1, condition: 'file(\"Foo.esp\")'}");
    tag = node.as<Tag>();
    EXPECT_EQ("name1", tag.Name());
    EXPECT_TRUE(tag.IsAddition());
    EXPECT_EQ("file(\"Foo.esp\")", tag.Condition());

    node = YAML::Load("{name: name1, condition: invalid}");
    EXPECT_THROW(node.as<Tag>(), YAML::RepresentationException);

    node = YAML::Load("[0, 1, 2]");
    EXPECT_THROW(node.as<Tag>(), YAML::RepresentationException);
}

#endif
