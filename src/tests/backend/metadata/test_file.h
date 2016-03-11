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

#ifndef LOOT_TEST_BACKEND_METADATA_FILE
#define LOOT_TEST_BACKEND_METADATA_FILE

#include "backend/metadata/file.h"
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        TEST(File, ConstructorsAndDataAccess) {
            File file;
            EXPECT_EQ("", file.Name());
            EXPECT_EQ("", file.DisplayName());
            EXPECT_EQ("", file.Condition());

            file = File("name");
            EXPECT_EQ("name", file.Name());
            EXPECT_EQ("name", file.DisplayName());
            EXPECT_EQ("", file.Condition());

            file = File("name", "display");
            EXPECT_EQ("name", file.Name());
            EXPECT_EQ("display", file.DisplayName());
            EXPECT_EQ("", file.Condition());

            // Not a valid condition, but not evaluating it in this test.
            file = File("name", "display", "condition");
            EXPECT_EQ("name", file.Name());
            EXPECT_EQ("display", file.DisplayName());
            EXPECT_EQ("condition", file.Condition());
        }

        TEST(File, EqualityOperator) {
            File file1, file2;
            EXPECT_TRUE(file1 == file2);

            // Not valid conditions, but not evaluating them in this test.
            file1 = File("name", "display1", "condition1");
            file2 = File("name", "display2", "condition2");
            EXPECT_TRUE(file1 == file2);

            file1 = File("name1");
            file2 = File("name2");
            EXPECT_FALSE(file1 == file2);
        }

        TEST(File, LessThanOperator) {
            File file1, file2;
            EXPECT_FALSE(file1 < file2);
            EXPECT_FALSE(file2 < file1);

            file1 = File("name", "display1", "condition1");
            file2 = File("name", "display2", "condition2");
            EXPECT_FALSE(file1 < file2);
            EXPECT_FALSE(file2 < file1);

            file1 = File("name1");
            file2 = File("name2");
            EXPECT_TRUE(file1 < file2);
            EXPECT_FALSE(file2 < file1);
        }

        TEST(File, YamlEmitter) {
            File file("name1", "display1", "condition1");

            YAML::Emitter e1;
            e1 << file;
            EXPECT_STREQ("name: 'name1'\ncondition: 'condition1'\ndisplay: 'display1'", e1.c_str());

            file = File("name1");
            YAML::Emitter e2;
            e2 << file;
            EXPECT_STREQ("'name1'", e2.c_str());

            file = File("name1", "", "condition1");
            YAML::Emitter e3;
            e3 << file;
            EXPECT_STREQ("name: 'name1'\ncondition: 'condition1'", e3.c_str());

            file = File("name1", "display1");
            YAML::Emitter e4;
            e4 << file;
            EXPECT_STREQ("name: 'name1'\ndisplay: 'display1'", e4.c_str());

            file = File("name1", "name1");
            YAML::Emitter e5;
            e5 << file;
            EXPECT_STREQ("'name1'", e5.c_str());

            file = File("name1", "name1", "condition1");
            YAML::Emitter e6;
            e6 << file;
            EXPECT_STREQ("name: 'name1'\ncondition: 'condition1'", e6.c_str());
        }

        TEST(File, YamlEncode) {
            File file("name1", "display1", "condition1");
            YAML::Node node;
            node = file;
            EXPECT_EQ("name1", node["name"].as<std::string>());
            EXPECT_EQ("display1", node["display"].as<std::string>());
            EXPECT_EQ("condition1", node["condition"].as<std::string>());

            file = File("name1");
            node = file;
            EXPECT_EQ("name1", node["name"].as<std::string>());
            EXPECT_FALSE(node["display"]);
            EXPECT_FALSE(node["condition"]);

            file = File("name1", "name1");
            node = file;
            EXPECT_EQ("name1", node["name"].as<std::string>());
            EXPECT_FALSE(node["display"]);
            EXPECT_FALSE(node["condition"]);

            file = File("name1", "display1");
            node = file;
            EXPECT_EQ("name1", node["name"].as<std::string>());
            EXPECT_EQ("display1", node["display"].as<std::string>());
            EXPECT_FALSE(node["condition"]);
        }

        TEST(File, YamlDecode) {
            YAML::Node node = YAML::Load("{name: name1, display: display1, condition: 'file(\"Foo.esp\")'}");
            File file = node.as<File>();
            EXPECT_EQ("name1", file.Name());
            EXPECT_EQ("display1", file.DisplayName());
            EXPECT_EQ("file(\"Foo.esp\")", file.Condition());

            node = YAML::Load("name1");
            file = node.as<File>();
            EXPECT_EQ("name1", file.Name());
            EXPECT_EQ("name1", file.DisplayName());
            EXPECT_EQ("", file.Condition());

            node = YAML::Load("{name: name1, display: display1}");
            file = node.as<File>();
            EXPECT_EQ("name1", file.Name());
            EXPECT_EQ("display1", file.DisplayName());
            EXPECT_EQ("", file.Condition());

            node = YAML::Load("{name: name1, condition: 'file(\"Foo.esp\")'}");
            file = node.as<File>();
            EXPECT_EQ("name1", file.Name());
            EXPECT_EQ("name1", file.DisplayName());
            EXPECT_EQ("file(\"Foo.esp\")", file.Condition());

            node = YAML::Load("{name: name1, condition: invalid}");
            EXPECT_THROW(node.as<File>(), YAML::RepresentationException);

            node = YAML::Load("[0, 1, 2]");
            EXPECT_ANY_THROW(node.as<File>());
        }
    }
}

#endif
