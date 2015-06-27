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

#ifndef LOOT_TEST_BACKEND_HELPERS_YAML_SET_HELPERS
#define LOOT_TEST_BACKEND_HELPERS_YAML_SET_HELPERS

#include "backend/helpers/yaml_set_helpers.h"
#include "tests/fixtures.h"

TEST(set, YamlEncode) {
    YAML::Node node;
    std::set<std::string> stringSet({"a", "b", "c"});
    node = stringSet;
    EXPECT_EQ("a", node[0].as<std::string>());
    EXPECT_EQ("b", node[1].as<std::string>());
    EXPECT_EQ("c", node[2].as<std::string>());
}

TEST(set, YamlDecode) {
    YAML::Node node;
    std::set<std::string> stringSet;

    node = YAML::Load("[a, b, c]");
    stringSet = node.as<std::set<std::string>>();
    EXPECT_EQ(stringSet.begin(), stringSet.find("a"));
    EXPECT_EQ(++stringSet.begin(), stringSet.find("b"));
    EXPECT_EQ(--stringSet.end(), stringSet.find("c"));
}

TEST(set, YamlEmitter) {
    std::set<std::string> stringSet({"a", "b", "c"});
    YAML::Emitter e1;
    e1 << stringSet;
    EXPECT_STREQ("- a\n- b\n- c", e1.c_str());
}

bool nodeContains(const std::string& value, const YAML::Node& node) {
    for (const auto& element : node) {
        if (element.as<std::string>() == value)
            return true;
    }
    return false;
}

TEST(unordered_set, YamlEncode) {
    YAML::Node node;
    std::unordered_set<std::string> stringSet({"a", "b", "c"});
    node = stringSet;
    // Check that the values are all present in the node.
    EXPECT_PRED2(nodeContains, "a", node);
    EXPECT_PRED2(nodeContains, "b", node);
    EXPECT_PRED2(nodeContains, "c", node);
}

TEST(unordered_set, YAMLDecode) {
    YAML::Node node;
    std::unordered_set<std::string> stringSet;

    node = YAML::Load("[a, b, c]");
    stringSet = node.as<std::unordered_set<std::string>>();
    EXPECT_EQ(1, stringSet.count("a"));
    EXPECT_EQ(1, stringSet.count("b"));
    EXPECT_EQ(1, stringSet.count("c"));
}

bool sequenceOf(const char val1,
                const char val2,
                const char val3,
                const std::string& seq) {
    if (seq.substr(0, 2) != "- "
        || seq.substr(3, 3) != "\n- "
        || seq.substr(7, 3) != "\n- ")
        return false;

    std::list<char> values({val1, val2, val3});
    std::list<char> found({seq[2], seq[6], seq[10]});

    values.sort();
    found.sort();

    return values == found;
}

TEST(unordered_set, YamlEmitter) {
    std::unordered_set<std::string> stringSet({"a", "b", "c"});
    YAML::Emitter e1;
    e1 << stringSet;
    ASSERT_EQ(11, e1.size());
    EXPECT_PRED4(sequenceOf, 'a', 'b', 'c', e1.c_str());
}

#endif
