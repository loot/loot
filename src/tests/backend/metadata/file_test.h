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

#ifndef LOOT_TESTS_BACKEND_METADATA_FILE_TEST
#define LOOT_TESTS_BACKEND_METADATA_FILE_TEST

#include "backend/metadata/file.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(File, defaultConstructorShouldInitialiseEmptyStrings) {
  File file;

  EXPECT_EQ("", file.Name());
  EXPECT_EQ("", file.DisplayName());
  EXPECT_EQ("", file.Condition());
}

TEST(File, stringsConstructorShouldStoreGivenStrings) {
  File file("name", "display", "condition");

  EXPECT_EQ("name", file.Name());
  EXPECT_EQ("display", file.DisplayName());
  EXPECT_EQ("condition", file.Condition());
}

TEST(File, filesWithCaseInsensitiveEqualNameStringsShouldBeEqual) {
  File file1("name", "display1", "condition1");
  File file2("Name", "display2", "condition2");

  EXPECT_TRUE(file1 == file2);
}

TEST(File, filesWithDifferentNamesShouldBeUnequal) {
  File file1("name1");
  File file2("name2");

  EXPECT_FALSE(file1 == file2);
}

TEST(File, lessThanOperatorShouldUseCaseInsensitiveLexicographicalNameComparison) {
  File file1("name", "display1", "condition1");
  File file2("Name", "display2", "condition2");

  EXPECT_FALSE(file1 < file2);
  EXPECT_FALSE(file2 < file1);

  file1 = File("name1");
  file2 = File("name2");

  EXPECT_TRUE(file1 < file2);
  EXPECT_FALSE(file2 < file1);
}

TEST(File, emittingAsYamlShouldSingleQuoteValues) {
  File file("name1", "display1", "condition1");
  YAML::Emitter emitter;
  emitter << file;
  std::string expected = "name: '" + file.Name() +
    "'\ncondition: '" + file.Condition() +
    "'\ndisplay: '" + file.DisplayName() + "'";

  EXPECT_EQ(expected, emitter.c_str());
}

TEST(File, emittingAsYamlShouldOutputAsAScalarIfOnlyTheNameStringIsNotEmpty) {
  File file("name1");
  YAML::Emitter emitter;
  emitter << file;

  EXPECT_EQ("'" + file.Name() + "'", emitter.c_str());
}

TEST(File, emittingAsYamlShouldOmitDisplayFieldIfItMatchesTheNameField) {
  File file("name1", "name1");
  YAML::Emitter emitter;
  emitter << file;

  EXPECT_EQ("'" + file.Name() + "'", emitter.c_str());
}

TEST(File, emittingAsYamlShouldOmitAnEmptyConditionString) {
  File file("name1", "display1");
  YAML::Emitter emitter;
  emitter << file;
  std::string expected = "name: '" + file.Name() +
    "'\ndisplay: '" + file.DisplayName() + "'";

  EXPECT_EQ(expected, emitter.c_str());
}

TEST(File, encodingAsYamlShouldStoreDataCorrectly) {
  File file("name1", "display1", "condition1");
  YAML::Node node;
  node = file;

  EXPECT_EQ(file.Name(), node["name"].as<std::string>());
  EXPECT_EQ(file.DisplayName(), node["display"].as<std::string>());
  EXPECT_EQ(file.Condition(), node["condition"].as<std::string>());
}

TEST(File, encodingAsYamlShouldOmitEmptyFields) {
  File file("name1");
  YAML::Node node;
  node = file;

  EXPECT_EQ(file.Name(), node["name"].as<std::string>());
  EXPECT_FALSE(node["display"]);
  EXPECT_FALSE(node["condition"]);
}

TEST(File, encodingAsYamlShouldOmitDisplayFieldIfItMatchesTheNameField) {
  File file("name1", "name1");
  YAML::Node node;
  node = file;

  EXPECT_EQ(file.Name(), node["name"].as<std::string>());
  EXPECT_FALSE(node["display"]);
  EXPECT_FALSE(node["condition"]);
}

TEST(File, decodingFromYamlShouldSetDataCorrectly) {
  YAML::Node node = YAML::Load("{name: name1, display: display1, condition: 'file(\"Foo.esp\")'}");
  File file = node.as<File>();

  EXPECT_EQ(node["name"].as<std::string>(), file.Name());
  EXPECT_EQ(node["display"].as<std::string>(), file.DisplayName());
  EXPECT_EQ(node["condition"].as<std::string>(), file.Condition());
}

TEST(File, decodingFromYamlWithMissingConditionFieldShouldLeaveConditionStringEmpty) {
  YAML::Node node = YAML::Load("{name: name1, display: display1}");
  File file = node.as<File>();

  EXPECT_EQ(node["name"].as<std::string>(), file.Name());
  EXPECT_EQ(node["display"].as<std::string>(), file.DisplayName());
  EXPECT_TRUE(file.Condition().empty());
}

TEST(File, decodingFromYamlScalarShouldUseNameValueForDisplayNameAndLeaveConditionEmpty) {
  YAML::Node node = YAML::Load("name1");
  File file = node.as<File>();

  EXPECT_EQ(node.as<std::string>(), file.Name());
  EXPECT_EQ(node.as<std::string>(), file.DisplayName());
  EXPECT_TRUE(file.Condition().empty());
}

TEST(File, decodingFromYamlShouldThrowIfAnInvalidMapIsGiven) {
  YAML::Node node = YAML::Load("{name: name1, condition: invalid}");

  EXPECT_THROW(node.as<File>(), YAML::RepresentationException);
}

TEST(File, decodingFromYamlShouldThrowIfAListIsGiven) {
  YAML::Node node = YAML::Load("[0, 1, 2]");

  EXPECT_ANY_THROW(node.as<File>());
}
}
}

#endif
