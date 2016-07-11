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

#ifndef LOOT_TESTS_BACKEND_HELPERS_YAML_SET_HELPERS_TEST
#define LOOT_TESTS_BACKEND_HELPERS_YAML_SET_HELPERS_TEST

#include "backend/helpers/yaml_set_helpers.h"

#include <gtest/gtest.h>

namespace loot {
namespace test {
TEST(set, encodingAsYamlShouldStoreAllValuesInSetOrder) {
  std::set<std::string> stringSet({"a", "b", "c"});
  YAML::Node node;
  node = stringSet;

  EXPECT_TRUE(node.IsSequence());
  ASSERT_EQ(3, node.size());
  EXPECT_EQ("a", node[0].as<std::string>());
  EXPECT_EQ("b", node[1].as<std::string>());
  EXPECT_EQ("c", node[2].as<std::string>());
}

TEST(set, decodingFromAYamlListShouldStoreValuesCorrectly) {
  YAML::Node node = YAML::Load("[a, b, c]");
  std::set<std::string> stringSet = node.as<std::set<std::string>>();
  std::set<std::string> expectedStringSet({"a", "b", "c"});

  EXPECT_EQ(expectedStringSet, stringSet);
}

TEST(set, decodingFromAYamlListThatContainsDuplicateElementsShouldThrow) {
  YAML::Node node = YAML::Load("[a, b, c, c]");
  EXPECT_ANY_THROW(node.as<std::set<std::string>>());
}

TEST(set, emittingAsYamlShouldOutputAYamlListContainingAllValues) {
  std::set<std::string> stringSet({"a", "b", "c"});
  YAML::Emitter e1;
  e1 << stringSet;

  YAML::Node node = YAML::Load(e1.c_str());
  EXPECT_TRUE(node.IsSequence());
  ASSERT_EQ(3, node.size());
  EXPECT_EQ("a", node[0].as<std::string>());
  EXPECT_EQ("b", node[1].as<std::string>());
  EXPECT_EQ("c", node[2].as<std::string>());
}

class unordered_set : public ::testing::Test {
protected:
  static bool nodeContains(const YAML::Node& node, const std::string& value) {
    for (const auto& element : node) {
      if (element.as<std::string>() == value)
        return true;
    }
    return false;
  }

  static bool isSequenceOf(const std::string& sequence, const std::vector<std::string>& values) {
    std::set<std::string> sortedValues(std::begin(values), std::end(values));

    std::set<std::string> found;
    size_t i = 2;
    while (i < sequence.length()) {
      size_t separatorPos = sequence.find("\n- ", i);
      found.insert(sequence.substr(i, separatorPos));
    }

    return sortedValues == found;
  }
};

TEST_F(unordered_set, encodingAsYamlShouldStoreAllValuesInUndefinedOrder) {
  std::unordered_set<std::string> stringSet({"a", "b", "c"});
  YAML::Node node;
  node = stringSet;

  EXPECT_TRUE(node.IsSequence());
  ASSERT_EQ(3, node.size());
  EXPECT_PRED2(&nodeContains, node, "a");
  EXPECT_PRED2(&nodeContains, node, "b");
  EXPECT_PRED2(&nodeContains, node, "c");
}

TEST_F(unordered_set, decodingFromAYamlListShouldStoreValuesCorrectly) {
  YAML::Node node = YAML::Load("[a, b, c]");
  std::unordered_set<std::string> stringSet = node.as<std::unordered_set<std::string>>();

  EXPECT_EQ(1, stringSet.count("a"));
  EXPECT_EQ(1, stringSet.count("b"));
  EXPECT_EQ(1, stringSet.count("c"));
}

TEST_F(unordered_set, decodingFromAYamlListThatContainsDuplicateElementsShouldThrow) {
  YAML::Node node = YAML::Load("[a, b, c, c]");

  EXPECT_ANY_THROW(node.as<std::unordered_set<std::string>>());
}

TEST_F(unordered_set, emittingAsYamlShouldOutputAYamlListContainingAllValues) {
  std::unordered_set<std::string> stringSet({"a", "b", "c"});
  YAML::Emitter e1;
  e1 << stringSet;

  YAML::Node node = YAML::Load(e1.c_str());
  EXPECT_TRUE(node.IsSequence());
  ASSERT_EQ(3, node.size());
  EXPECT_PRED2(&nodeContains, node, "a");
  EXPECT_PRED2(&nodeContains, node, "b");
  EXPECT_PRED2(&nodeContains, node, "c");
}
}
}

#endif
