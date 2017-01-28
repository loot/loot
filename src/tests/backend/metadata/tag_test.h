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

#ifndef LOOT_TESTS_BACKEND_METADATA_TAG_TEST
#define LOOT_TESTS_BACKEND_METADATA_TAG_TEST

#include "loot/metadata/tag.h"

#include <gtest/gtest.h>

#include "loot/yaml/tag.h"

namespace loot {
namespace test {
TEST(Tag, defaultConstructorShouldSetEmptyNameAndConditionStringsForATagAddition) {
  Tag tag;

  EXPECT_TRUE(tag.Name().empty());
  EXPECT_TRUE(tag.IsAddition());
  EXPECT_TRUE(tag.Condition().empty());
}

TEST(Tag, dataConstructorShouldSetFieldsToGivenValues) {
  Tag tag("name", false, "condition");

  EXPECT_EQ("name", tag.Name());
  EXPECT_FALSE(tag.IsAddition());
  EXPECT_EQ("condition", tag.Condition());
}

TEST(Tag, tagsWithCaseInsensitiveEqualNamesAndEqualAdditionStatesShouldBeEqual) {
  Tag tag1("Name", true, "condition1");
  Tag tag2("name", true, "condition2");

  EXPECT_TRUE(tag1 == tag2);
}

TEST(Tags, tagsWithUnequalNamesShouldNotBeEqual) {
  Tag tag1("name1");
  Tag tag2("name2");

  EXPECT_FALSE(tag1 == tag2);
}

TEST(Tag, tagsWithUnequalAdditionStatesShouldNotBeEqual) {
  Tag tag1("Name", true);
  Tag tag2("name", false);

  EXPECT_FALSE(tag1 == tag2);
}

TEST(Tag, lessThanOperatorShouldCaseInsensitivelyLexicographicallyCompareNameStrings) {
  Tag tag1("Name");
  Tag tag2("name");

  EXPECT_FALSE(tag1 < tag2);
  EXPECT_FALSE(tag2 < tag1);

  tag1 = Tag("name1");
  tag2 = Tag("name2");

  EXPECT_TRUE(tag1 < tag2);
  EXPECT_FALSE(tag2 < tag1);
}

TEST(Tag, lessThanOperatorShouldTreatTagAdditionsAsBeingLessThanRemovals) {
  Tag tag1("name", true);
  Tag tag2("name", false);

  EXPECT_TRUE(tag1 < tag2);
  EXPECT_FALSE(tag2 < tag1);
}

TEST(Tag, emittingAsYamlShouldOutputOnlyTheNameStringIfTheTagIsAnAdditionWithNoCondition) {
  Tag tag("name1");
  YAML::Emitter emitter;
  emitter << tag;

  EXPECT_EQ(tag.Name(), emitter.c_str());
}

TEST(Tag, emittingAsYamlShouldOutputOnlyTheNameStringPrefixedWithAHyphenIfTheTagIsARemovalWithNoCondition) {
  Tag tag("name1", false);
  YAML::Emitter emitter;
  emitter << tag;

  EXPECT_EQ("-" + tag.Name(), emitter.c_str());
}

TEST(Tag, emittingAsYamlShouldOutputAMapIfTheTagHasACondition) {
  Tag tag("name1", false, "condition1");
  YAML::Emitter emitter;
  emitter << tag;

  EXPECT_STREQ("name: -name1\ncondition: 'condition1'", emitter.c_str());
}

TEST(Tag, encodingAsYamlShouldOmitTheConditionFieldIfTheConditionStringIsEmpty) {
  Tag tag;
  YAML::Node node;
  node = tag;

  EXPECT_FALSE(node["condition"]);
}

TEST(Tag, encodingAsYamlShouldOutputTheNameFieldCorrectly) {
  Tag tag("name1");
  YAML::Node node;
  node = tag;

  EXPECT_EQ(tag.Name(), node["name"].as<std::string>());
}

TEST(Tag, encodingAsYamlShouldOutputTheNameFieldWithAHyphenPrefixIfTheTagIsARemoval) {
  Tag tag("name1", false);
  YAML::Node node;
  node = tag;

  EXPECT_EQ("-" + tag.Name(), node["name"].as<std::string>());
}

TEST(Tag, encodingAsYamlShouldOutputTheConditionFieldIfTheConditionStringIsNotEmpty) {
  Tag tag("name1", true, "condition1");
  YAML::Node node;
  node = tag;

  EXPECT_EQ(tag.Name(), node["name"].as<std::string>());
  EXPECT_EQ(tag.Condition(), node["condition"].as<std::string>());
}

TEST(Tag, decodingFromYamlScalarShouldSetNameCorrectly) {
  YAML::Node node = YAML::Load("name1");
  Tag tag = node.as<Tag>();

  EXPECT_EQ("name1", tag.Name());
  EXPECT_TRUE(tag.IsAddition());
  EXPECT_EQ("", tag.Condition());
}

TEST(Tag, decodingFromYamlScalarShouldSetAdditionStateCorrectly) {
  YAML::Node node = YAML::Load("-name1");
  Tag tag = node.as<Tag>();

  EXPECT_EQ("name1", tag.Name());
  EXPECT_FALSE(tag.IsAddition());
  EXPECT_EQ("", tag.Condition());
}

TEST(Tag, decodingFromYamlMapShouldSetDataCorrectly) {
  YAML::Node node = YAML::Load("{name: name1, condition: 'file(\"Foo.esp\")'}");
  Tag tag = node.as<Tag>();

  EXPECT_EQ("name1", tag.Name());
  EXPECT_TRUE(tag.IsAddition());
  EXPECT_EQ("file(\"Foo.esp\")", tag.Condition());
}

TEST(Tag, decodingFromYamlShouldThrowIfAnInvalidConditionIsGiven) {
  YAML::Node node = YAML::Load("{name: name1, condition: invalid}");

  EXPECT_THROW(node.as<Tag>(), YAML::RepresentationException);
}

TEST(Tag, decodingFromYamlListShouldThrow) {
  YAML::Node node = YAML::Load("[0, 1, 2]");

  EXPECT_THROW(node.as<Tag>(), YAML::RepresentationException);
}
}
}

#endif
