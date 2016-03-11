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

#ifndef LOOT_TEST_F_BACKEND_METADATA_PLUGIN_METADATA
#define LOOT_TEST_F_BACKEND_METADATA_PLUGIN_METADATA

#include "backend/metadata/plugin_metadata.h"
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class PluginMetadataTest : public SkyrimTest {};

        TEST_F(PluginMetadataTest, Constructors) {
            PluginMetadata pm;
            EXPECT_TRUE(pm.Enabled());
            EXPECT_FALSE(pm.IsPriorityExplicit());
            EXPECT_FALSE(pm.IsPriorityGlobal());
            EXPECT_EQ(0, pm.Priority());

            pm = PluginMetadata("Blank.esm");
            EXPECT_EQ("Blank.esm", pm.Name());
            EXPECT_TRUE(pm.Enabled());
            EXPECT_FALSE(pm.IsPriorityExplicit());
            EXPECT_FALSE(pm.IsPriorityGlobal());
            EXPECT_EQ(0, pm.Priority());
        }

        TEST_F(PluginMetadataTest, EqualityOperator) {
            PluginMetadata pm1, pm2;
            EXPECT_TRUE(pm1 == pm2);

            pm1 = PluginMetadata("Blank.esm");
            pm2 = PluginMetadata("blank.esm");
            EXPECT_TRUE(pm1 == pm2);

            pm1 = PluginMetadata("Blank.esm");
            pm2 = PluginMetadata("Blan.\\.esm");
            EXPECT_TRUE(pm1 == pm2);
            EXPECT_TRUE(pm2 == pm1);

            pm1 = PluginMetadata("Blan.esm");
            pm2 = PluginMetadata("Blan.\\.esm");
            EXPECT_FALSE(pm1 == pm2);
            EXPECT_FALSE(pm2 == pm1);

            pm1 = PluginMetadata("Blan.\\.esm");
            pm2 = PluginMetadata("Blan.\\.esm");
            EXPECT_TRUE(pm1 == pm2);
            EXPECT_TRUE(pm2 == pm1);

            pm1 = PluginMetadata("Blan(k|p).esm");
            pm2 = PluginMetadata("Blan.\\.esm");
            EXPECT_FALSE(pm1 == pm2);
            EXPECT_FALSE(pm2 == pm1);
        }

        TEST_F(PluginMetadataTest, MergeMetadata) {
            PluginMetadata pm1(YAML::Load(
                "name: 'Blank.esp'\n"
                "enabled: false\n"
                "priority: 5\n"
                "after:\n"
                "  - 'Blank.esm'\n"
                "req:\n"
                "  - 'Blank.esm'\n"
                "inc:\n"
                "  - 'Blank.esm'\n"
                "msg:\n"
                "  - type: say\n"
                "    content: 'content'\n"
                "tag:\n"
                "  - Relev\n"
                "dirty:\n"
                "  - crc: 0x5\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "url:\n"
                "  - 'http://www.example.com'"
                ).as<PluginMetadata>());

            PluginMetadata pm2(YAML::Load(
                "name: 'Blank - Different.esp'\n"
                "after:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "req:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "inc:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "msg:\n"
                "  - type: say\n"
                "    content: 'content'\n"
                "tag:\n"
                "  - Relev\n"
                "  - -Relev\n"
                "  - Delev\n"
                "dirty:\n"
                "  - crc: 0x5\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "  - crc: 0x9\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "url:\n"
                "  - 'http://www.example.com'\n"
                "  - 'http://www.example2.com'"
                ).as<PluginMetadata>());

            EXPECT_NO_THROW(pm1.MergeMetadata(pm2));
            EXPECT_EQ("Blank.esp", pm1.Name());
            EXPECT_TRUE(pm1.Enabled());
            EXPECT_EQ(5, pm1.Priority());
            EXPECT_TRUE(pm1.IsPriorityExplicit());
            EXPECT_FALSE(pm1.IsPriorityGlobal());
            EXPECT_EQ(std::set<File>({
                File("Blank.esm"),
                File("Blank.esp"),
            }), pm1.LoadAfter());
            EXPECT_EQ(std::set<File>({
                File("Blank.esm"),
                File("Blank.esp"),
            }), pm1.Reqs());
            EXPECT_EQ(std::set<File>({
                File("Blank.esm"),
                File("Blank.esp"),
            }), pm1.Incs());
            EXPECT_EQ(std::list<Message>({
                Message(Message::say, "content"),
                Message(Message::say, "content"),
            }), pm1.Messages());
            EXPECT_EQ(std::set<Tag>({
                Tag("Relev"),
                Tag("Relev", false),
                Tag("Delev"),
            }), pm1.Tags());
            EXPECT_EQ(std::set<PluginDirtyInfo>({
                PluginDirtyInfo(5, 0, 1, 2, "utility"),
                PluginDirtyInfo(9, 0, 1, 2, "utility"),
            }), pm1.DirtyInfo());
            EXPECT_EQ(std::set<Location>({
                Location("http://www.example.com"),
                Location("http://www.example2.com"),
            }), pm1.Locations());

            pm2.SetPriorityExplicit(true);
            EXPECT_NO_THROW(pm1.MergeMetadata(pm2));
            EXPECT_EQ(0, pm1.Priority());
            EXPECT_TRUE(pm1.IsPriorityExplicit());
            EXPECT_FALSE(pm1.IsPriorityGlobal());
        }

        TEST_F(PluginMetadataTest, DiffMetadata) {
            PluginMetadata pm1(YAML::Load(
                "name: 'Blank.esp'\n"
                "enabled: false\n"
                "after:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank - Different.esm'\n"
                "req:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank - Different.esm'\n"
                "inc:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank - Different.esm'\n"
                "msg:\n"
                "  - type: say\n"
                "    content: 'content'\n"
                "  - type: say\n"
                "    content: 'content2'\n"
                "tag:\n"
                "  - Relev\n"
                "  - NoMerge\n"
                "dirty:\n"
                "  - crc: 0x5\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "  - crc: 0x1\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "url:\n"
                "  - 'http://www.example.com'\n"
                "  - 'http://www.other-example.com'"
                ).as<PluginMetadata>());

            PluginMetadata pm2(YAML::Load(
                "name: 'Blank - Different.esp'\n"
                "after:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "req:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "inc:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "msg:\n"
                "  - type: say\n"
                "    content: 'content'\n"
                "  - type: say\n"
                "    content: 'content3'\n"
                "tag:\n"
                "  - Relev\n"
                "  - -Relev\n"
                "  - Delev\n"
                "dirty:\n"
                "  - crc: 0x5\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "  - crc: 0x9\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "url:\n"
                "  - 'http://www.example.com'\n"
                "  - 'http://www.example2.com'"
                ).as<PluginMetadata>());

            PluginMetadata result = pm1.DiffMetadata(pm2);
            EXPECT_EQ("Blank.esp", result.Name());
            EXPECT_FALSE(result.Enabled());
            EXPECT_EQ(0, result.Priority());
            EXPECT_FALSE(result.IsPriorityExplicit());
            EXPECT_FALSE(result.IsPriorityGlobal());
            EXPECT_EQ(std::set<File>({
                File("Blank.esp"),
                File("Blank - Different.esm"),
            }), result.LoadAfter());
            EXPECT_EQ(std::set<File>({
                File("Blank.esp"),
                File("Blank - Different.esm"),
            }), result.Reqs());
            EXPECT_EQ(std::set<File>({
                File("Blank.esp"),
                File("Blank - Different.esm"),
            }), result.Incs());
            EXPECT_EQ(std::list<Message>({
                Message(Message::say, "content2"),
                Message(Message::say, "content3"),
            }), result.Messages());
            EXPECT_EQ(std::set<Tag>({
                Tag("Relev", false),
                Tag("Delev"),
                Tag("NoMerge"),
            }), result.Tags());
            EXPECT_EQ(std::set<PluginDirtyInfo>({
                PluginDirtyInfo(9, 0, 1, 2, "utility"),
                PluginDirtyInfo(1, 0, 1, 2, "utility"),
            }), result.DirtyInfo());
            EXPECT_EQ(std::set<Location>({
                Location("http://www.example2.com"),
                Location("http://www.other-example.com"),
            }), result.Locations());

            pm1.Priority(5);
            result = pm1.DiffMetadata(pm2);

            EXPECT_EQ(5, result.Priority());
            EXPECT_TRUE(result.IsPriorityExplicit());
            EXPECT_FALSE(result.IsPriorityGlobal());
        }

        TEST_F(PluginMetadataTest, NewMetadata) {
            PluginMetadata pm1(YAML::Load(
                "name: 'Blank.esp'\n"
                "enabled: false\n"
                "after:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank - Different.esm'\n"
                "req:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank - Different.esm'\n"
                "inc:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank - Different.esm'\n"
                "msg:\n"
                "  - type: say\n"
                "    content: 'content'\n"
                "  - type: say\n"
                "    content: 'content2'\n"
                "tag:\n"
                "  - Relev\n"
                "  - NoMerge\n"
                "dirty:\n"
                "  - crc: 0x5\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "  - crc: 0x1\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "url:\n"
                "  - 'http://www.example.com'\n"
                "  - 'http://www.other-example.com'"
                ).as<PluginMetadata>());

            PluginMetadata pm2(YAML::Load(
                "name: 'Blank - Different.esp'\n"
                "after:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "req:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "inc:\n"
                "  - 'Blank.esm'\n"
                "  - 'Blank.esp'\n"
                "msg:\n"
                "  - type: say\n"
                "    content: 'content'\n"
                "  - type: say\n"
                "    content: 'content3'\n"
                "tag:\n"
                "  - Relev\n"
                "  - -Relev\n"
                "  - Delev\n"
                "dirty:\n"
                "  - crc: 0x5\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "  - crc: 0x9\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "url:\n"
                "  - 'http://www.example.com'\n"
                "  - 'http://www.example2.com'"
                ).as<PluginMetadata>());

            PluginMetadata result = pm2.NewMetadata(pm1);
            EXPECT_EQ("Blank - Different.esp", result.Name());
            EXPECT_TRUE(result.Enabled());
            EXPECT_EQ(0, result.Priority());
            EXPECT_FALSE(result.IsPriorityExplicit());
            EXPECT_FALSE(result.IsPriorityGlobal());
            EXPECT_EQ(std::set<File>({
                File("Blank.esp")
            }), result.LoadAfter());
            EXPECT_EQ(std::set<File>({
                File("Blank.esp")
            }), result.Reqs());
            EXPECT_EQ(std::set<File>({
                File("Blank.esp")
            }), result.Incs());
            EXPECT_EQ(std::list<Message>({
                Message(Message::say, "content3"),
            }), result.Messages());
            EXPECT_EQ(std::set<Tag>({
                Tag("Relev", false),
                Tag("Delev"),
            }), result.Tags());
            EXPECT_EQ(std::set<PluginDirtyInfo>({
                PluginDirtyInfo(9, 0, 1, 2, "utility")
            }), result.DirtyInfo());
            EXPECT_EQ(std::set<Location>({
                Location("http://www.example2.com")
            }), result.Locations());

            pm1.Priority(5);
            result = pm2.NewMetadata(pm1);

            EXPECT_EQ(0, result.Priority());
            EXPECT_FALSE(result.IsPriorityExplicit());
            EXPECT_FALSE(result.IsPriorityGlobal());
        }

        TEST_F(PluginMetadataTest, Enabled) {
            PluginMetadata pm;
            ASSERT_TRUE(pm.Enabled());
            pm.Enabled(false);
            EXPECT_FALSE(pm.Enabled());
        }

        TEST_F(PluginMetadataTest, SetPriorityExplicit) {
            PluginMetadata pm;
            ASSERT_FALSE(pm.IsPriorityExplicit());
            pm.SetPriorityExplicit(true);
            EXPECT_TRUE(pm.IsPriorityExplicit());
        }

        TEST_F(PluginMetadataTest, Priority) {
            PluginMetadata pm;
            ASSERT_NE(10, pm.Priority());
            pm.Priority(10);
            EXPECT_EQ(10, pm.Priority());
        }

        TEST_F(PluginMetadataTest, settingPriorityWithAbsoluteValueGreaterOrEqualToGlobalPriorityDivisorShouldThrow) {
            PluginMetadata pm;
            EXPECT_ANY_THROW(pm.Priority(yamlGlobalPriorityDivisor));
            EXPECT_ANY_THROW(pm.Priority(yamlGlobalPriorityDivisor + 1));
            EXPECT_ANY_THROW(pm.Priority(-yamlGlobalPriorityDivisor));
            EXPECT_ANY_THROW(pm.Priority(-yamlGlobalPriorityDivisor - 1));
        }

        TEST_F(PluginMetadataTest, settingPriorityAsGlobalShouldSucceed) {
            PluginMetadata pm;
            ASSERT_FALSE(pm.IsPriorityGlobal());
            pm.SetPriorityGlobal(true);
            EXPECT_TRUE(pm.IsPriorityGlobal());
        }

        TEST_F(PluginMetadataTest, settingPriorityAsNotGlobalShouldSucceed) {
            PluginMetadata pm;
            pm.SetPriorityGlobal(true);
            ASSERT_TRUE(pm.IsPriorityGlobal());
            pm.SetPriorityGlobal(false);
            EXPECT_FALSE(pm.IsPriorityGlobal());
        }

        TEST_F(PluginMetadataTest, gettingYamlPriorityValueForAGlobalPriorityShouldReturnValueWithGlobalFlagSet) {
            PluginMetadata pm;
            pm.Priority(10);
            pm.SetPriorityGlobal(true);
            EXPECT_EQ(1000010, pm.GetYamlPriorityValue());

            pm.Priority(-20);
            EXPECT_EQ(-1000020, pm.GetYamlPriorityValue());
        }

        TEST_F(PluginMetadataTest, gettingYamlPriorityValueForANonGlobalPriorityShouldReturnValueWithGlobalFlagNotSet) {
            PluginMetadata pm;
            pm.Priority(10);
            EXPECT_EQ(10, pm.GetYamlPriorityValue());

            pm.Priority(-20);
            EXPECT_EQ(-20, pm.GetYamlPriorityValue());
        }

        TEST_F(PluginMetadataTest, LoadAfter) {
            PluginMetadata pm;
            ASSERT_TRUE(pm.LoadAfter().empty());
            pm.LoadAfter({
                File("Blank.esm")
            });
            EXPECT_EQ(std::set<File>({
                File("Blank.esm")
            }), pm.LoadAfter());
        }

        TEST_F(PluginMetadataTest, Reqs) {
            PluginMetadata pm;
            ASSERT_TRUE(pm.Reqs().empty());
            pm.Reqs({
                File("Blank.esm")
            });
            EXPECT_EQ(std::set<File>({
                File("Blank.esm")
            }), pm.Reqs());
        }

        TEST_F(PluginMetadataTest, Incs) {
            PluginMetadata pm;
            ASSERT_TRUE(pm.Incs().empty());
            pm.Incs({
                File("Blank.esm")
            });
            EXPECT_EQ(std::set<File>({
                File("Blank.esm")
            }), pm.Incs());
        }

        TEST_F(PluginMetadataTest, Messages) {
            PluginMetadata pm;
            ASSERT_TRUE(pm.Messages().empty());
            pm.Messages({
                Message(Message::say, "content")
            });
            EXPECT_EQ(std::list<Message>({
                Message(Message::say, "content")
            }), pm.Messages());
        }

        TEST_F(PluginMetadataTest, Tags) {
            PluginMetadata pm;
            ASSERT_TRUE(pm.Tags().empty());
            pm.Tags({
                Tag("Relev")
            });
            EXPECT_EQ(std::set<Tag>({
                Tag("Relev")
            }), pm.Tags());
        }

        TEST_F(PluginMetadataTest, DirtyInfo) {
            PluginMetadata pm;
            ASSERT_TRUE(pm.DirtyInfo().empty());
            pm.DirtyInfo({
                PluginDirtyInfo(5, 0, 1, 2, "utility")
            });
            EXPECT_EQ(std::set<PluginDirtyInfo>({
                PluginDirtyInfo(5, 0, 1, 2, "utility")
            }), pm.DirtyInfo());
        }

        TEST_F(PluginMetadataTest, Locations) {
            PluginMetadata pm;
            ASSERT_TRUE(pm.Locations().empty());
            pm.Locations({
                Location("http://www.example.com")
            });
            EXPECT_EQ(std::set<Location>({
                Location("http://www.example.com")
            }), pm.Locations());
        }

        TEST_F(PluginMetadataTest, EvalAllConditions) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            PluginMetadata pm(YAML::Load(
                "name: 'Blank.esp'\n"
                "after:\n"
                "  - name: 'Blank.esm'\n"
                "    condition: 'file(\"Blank.esm\")'\n"
                "req:\n"
                "  - name: 'Blank.esm'\n"
                "    condition: 'file(\"Blank.missing.esm\")'\n"
                "inc:\n"
                "  - name: 'Blank.esm'\n"
                "    condition: 'file(\"Blank.esm\")'\n"
                "msg:\n"
                "  - type: say\n"
                "    content: 'content'\n"
                "tag:\n"
                "  - name: Relev\n"
                "    condition: 'file(\"Blank.missing.esm\")'\n"
                "dirty:\n"
                "  - crc: 0x24F0E2A1\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2\n"
                "  - crc: 0xDEADBEEF\n"
                "    util: 'utility'\n"
                "    udr: 1\n"
                "    nav: 2"
                ).as<PluginMetadata>());

            pm.Messages({Message(Message::say, "content")});
            EXPECT_NO_THROW(pm.EvalAllConditions(game, Language::english));
            EXPECT_EQ(std::set<File>({
                File("Blank.esm", "", "file(\"Blank.esm\")")
            }), pm.LoadAfter());
            EXPECT_TRUE(pm.Reqs().empty());
            EXPECT_EQ(std::set<File>({
                File("Blank.esm", "", "file(\"Blank.esm\")")
            }), pm.Incs());
            EXPECT_EQ(std::list<Message>({
                Message(Message::say, "content")
            }), pm.Messages());
            EXPECT_TRUE(pm.Tags().empty());
            EXPECT_EQ(std::set<PluginDirtyInfo>({
                PluginDirtyInfo(0x24F0E2A1, 0, 1, 2, "utility")
            }), pm.DirtyInfo());
        }

        TEST_F(PluginMetadataTest, HasNameOnly) {
            PluginMetadata pm;
            EXPECT_TRUE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            EXPECT_TRUE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.Enabled(false);
            EXPECT_TRUE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.SetPriorityExplicit(true);
            EXPECT_FALSE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.LoadAfter({File("Blank.esm")});
            EXPECT_FALSE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.Reqs({File("Blank.esm")});
            EXPECT_FALSE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.Incs({File("Blank.esm")});
            EXPECT_FALSE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.Messages({Message(Message::say, "content")});
            EXPECT_FALSE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.Tags({Tag("Relev")});
            EXPECT_FALSE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.DirtyInfo({PluginDirtyInfo(5, 0, 1, 2, "utility")});
            EXPECT_FALSE(pm.HasNameOnly());

            pm = PluginMetadata("Blank.esp");
            pm.Locations({Location("http://www.example.com")});
            EXPECT_FALSE(pm.HasNameOnly());
        }

        TEST_F(PluginMetadataTest, IsRegexPlugin) {
            PluginMetadata pm;
            EXPECT_FALSE(pm.IsRegexPlugin());

            pm = PluginMetadata("Blank.esm");
            EXPECT_FALSE(pm.IsRegexPlugin());

            pm = PluginMetadata("Blank[[:blank:]]- Different.esm");
            EXPECT_TRUE(pm.IsRegexPlugin());

            pm = PluginMetadata("Blank\\.esm");
            EXPECT_TRUE(pm.IsRegexPlugin());

            pm = PluginMetadata("Blank - (Different )*Master Dependent.esm");
            EXPECT_TRUE(pm.IsRegexPlugin());

            pm = PluginMetadata("Blank - (Different )?Master Dependent.esm");
            EXPECT_TRUE(pm.IsRegexPlugin());

            pm = PluginMetadata("Blank.es(p|m)");
            EXPECT_TRUE(pm.IsRegexPlugin());
        }

        TEST_F(PluginMetadataTest, YamlEmitter) {
            PluginMetadata pm("Blank.esp");
            YAML::Emitter e1;
            e1 << pm;
            EXPECT_STREQ("", e1.c_str());

            pm.SetPriorityExplicit(true);
            YAML::Emitter e2;
            e2 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0", e2.c_str());

            pm.Enabled(false);
            YAML::Emitter e3;
            e3 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0\n"
                         "enabled: false", e3.c_str());

            pm.LoadAfter({File("Blank.esm")});
            YAML::Emitter e4;
            e4 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0\n"
                         "enabled: false\n"
                         "after:\n"
                         "  - 'Blank.esm'", e4.c_str());

            pm.Reqs({File("Blank.esm")});
            YAML::Emitter e5;
            e5 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0\n"
                         "enabled: false\n"
                         "after:\n"
                         "  - 'Blank.esm'\n"
                         "req:\n"
                         "  - 'Blank.esm'", e5.c_str());

            pm.Incs({File("Blank.esm")});
            YAML::Emitter e6;
            e6 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0\n"
                         "enabled: false\n"
                         "after:\n"
                         "  - 'Blank.esm'\n"
                         "req:\n"
                         "  - 'Blank.esm'\n"
                         "inc:\n"
                         "  - 'Blank.esm'", e6.c_str());

            pm.Messages({Message(Message::say, "content")});
            YAML::Emitter e7;
            e7 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0\n"
                         "enabled: false\n"
                         "after:\n"
                         "  - 'Blank.esm'\n"
                         "req:\n"
                         "  - 'Blank.esm'\n"
                         "inc:\n"
                         "  - 'Blank.esm'\n"
                         "msg:\n"
                         "  - type: say\n"
                         "    content: 'content'", e7.c_str());

            pm.Tags({Tag("Relev")});
            YAML::Emitter e8;
            e8 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0\n"
                         "enabled: false\n"
                         "after:\n"
                         "  - 'Blank.esm'\n"
                         "req:\n"
                         "  - 'Blank.esm'\n"
                         "inc:\n"
                         "  - 'Blank.esm'\n"
                         "msg:\n"
                         "  - type: say\n"
                         "    content: 'content'\n"
                         "tag:\n"
                         "  - Relev", e8.c_str());

            pm.DirtyInfo({PluginDirtyInfo(5, 0, 1, 2, "utility")});
            YAML::Emitter e9;
            e9 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0\n"
                         "enabled: false\n"
                         "after:\n"
                         "  - 'Blank.esm'\n"
                         "req:\n"
                         "  - 'Blank.esm'\n"
                         "inc:\n"
                         "  - 'Blank.esm'\n"
                         "msg:\n"
                         "  - type: say\n"
                         "    content: 'content'\n"
                         "tag:\n"
                         "  - Relev\n"
                         "dirty:\n"
                         "  - crc: 0x5\n"
                         "    util: 'utility'\n"
                         "    udr: 1\n"
                         "    nav: 2", e9.c_str());

            pm.Locations({Location("http://www.example.com")});
            YAML::Emitter e10;
            e10 << pm;
            EXPECT_STREQ("name: 'Blank.esp'\n"
                         "priority: 0\n"
                         "enabled: false\n"
                         "after:\n"
                         "  - 'Blank.esm'\n"
                         "req:\n"
                         "  - 'Blank.esm'\n"
                         "inc:\n"
                         "  - 'Blank.esm'\n"
                         "msg:\n"
                         "  - type: say\n"
                         "    content: 'content'\n"
                         "tag:\n"
                         "  - Relev\n"
                         "dirty:\n"
                         "  - crc: 0x5\n"
                         "    util: 'utility'\n"
                         "    udr: 1\n"
                         "    nav: 2\n"
                         "url:\n"
                         "  - 'http://www.example.com'", e10.c_str());
        }

        TEST_F(PluginMetadataTest, YamlEncode) {
            YAML::Node node;

            PluginMetadata pm("Blank.esp");
            node = pm;
            EXPECT_EQ("Blank.esp", node["name"].as<std::string>());
            EXPECT_FALSE(node["enabled"]);
            EXPECT_FALSE(node["priority"]);
            EXPECT_FALSE(node["after"]);
            EXPECT_FALSE(node["req"]);
            EXPECT_FALSE(node["inc"]);
            EXPECT_FALSE(node["msg"]);
            EXPECT_FALSE(node["tag"]);
            EXPECT_FALSE(node["dirty"]);
            EXPECT_FALSE(node["url"]);

            pm.SetPriorityExplicit(true);
            node = pm;
            EXPECT_EQ(0, node["priority"].as<int>());

            pm.Enabled(false);
            node = pm;
            EXPECT_FALSE(node["enabled"].as<bool>());

            pm.LoadAfter({File("Blank.esm")});
            node = pm;
            EXPECT_EQ(pm.LoadAfter(), node["after"].as<std::set<File>>());

            pm.Reqs({File("Blank.esm")});
            node = pm;
            EXPECT_EQ(pm.Reqs(), node["req"].as<std::set<File>>());

            pm.Incs({File("Blank.esm")});
            node = pm;
            EXPECT_EQ(pm.Incs(), node["inc"].as<std::set<File>>());

            pm.Messages({Message(Message::say, "content")});
            node = pm;
            EXPECT_EQ(pm.Messages(), node["msg"].as<std::list<Message>>());

            pm.Tags({Tag("Relev")});
            node = pm;
            EXPECT_EQ(pm.Tags(), node["tag"].as<std::set<Tag>>());

            pm.DirtyInfo({PluginDirtyInfo(5, 0, 1, 2, "utility")});
            node = pm;
            EXPECT_EQ(pm.DirtyInfo(), node["dirty"].as<std::set<PluginDirtyInfo>>());

            pm.Locations({Location("http://www.example.com")});
            node = pm;
            EXPECT_EQ(pm.Locations(), node["url"].as<std::set<Location>>());
        }

        TEST_F(PluginMetadataTest, YamlDecode) {
            YAML::Node node;
            PluginMetadata pm;

            node = YAML::Load("name: Blank.esp");
            pm = node.as<PluginMetadata>();
            EXPECT_EQ("Blank.esp", pm.Name());
            EXPECT_EQ(0, pm.Priority());
            EXPECT_FALSE(pm.IsPriorityExplicit());
            EXPECT_FALSE(pm.IsPriorityGlobal());

            node = YAML::Load("name: 'Blank.esp'\n"
                              "priority: 5\n"
                              "enabled: false\n"
                              "after:\n"
                              "  - 'Blank.esm'\n"
                              "req:\n"
                              "  - 'Blank.esm'\n"
                              "inc:\n"
                              "  - 'Blank.esm'\n"
                              "msg:\n"
                              "  - type: say\n"
                              "    content: 'content'\n"
                              "tag:\n"
                              "  - Relev\n"
                              "dirty:\n"
                              "  - crc: 0x5\n"
                              "    util: 'utility'\n"
                              "    udr: 1\n"
                              "    nav: 2\n"
                              "url:\n"
                              "  - 'http://www.example.com'");
            pm = node.as<PluginMetadata>();
            EXPECT_EQ("Blank.esp", pm.Name());
            EXPECT_EQ(5, pm.Priority());
            EXPECT_TRUE(pm.IsPriorityExplicit());
            EXPECT_FALSE(pm.IsPriorityGlobal());
            EXPECT_FALSE(pm.Enabled());
            EXPECT_EQ(std::set<File>({
                File("Blank.esm")
            }), pm.LoadAfter());
            EXPECT_EQ(std::set<File>({
                File("Blank.esm")
            }), pm.Reqs());
            EXPECT_EQ(std::set<File>({
                File("Blank.esm")
            }), pm.Incs());
            EXPECT_EQ(std::list<Message>({
                Message(Message::say, "content")
            }), pm.Messages());
            EXPECT_EQ(std::set<Tag>({
                Tag("Relev")
            }), pm.Tags());
            EXPECT_EQ(std::set<PluginDirtyInfo>({
                PluginDirtyInfo(5, 0, 1, 2, "utility")
            }), pm.DirtyInfo());
            EXPECT_EQ(std::set<Location>({
                Location("http://www.example.com")
            }), pm.Locations());

            // Don't allow dirty metadata in regex entries.
            node = YAML::Load("name: 'Blank\\.esp'\n"
                              "dirty:\n"
                              "  - crc: 0x5\n"
                              "    util: 'utility'\n"
                              "    udr: 1\n"
                              "    nav: 2");
            EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);

            // Don't allow invalid regex.
            node = YAML::Load("name: 'RagnvaldBook(Farengar(+Ragnvald)?)?\\.esp'\n"
                              "dirty:\n"
                              "  - crc: 0x5\n"
                              "    util: 'utility'\n"
                              "    udr: 1\n"
                              "    nav: 2");
            EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);

            // Catch condition syntax errors.
            node = YAML::Load(
                "name: 'Blank.esp'\n"
                "after:\n"
                "  - name: 'Blank.esm'\n"
                "    condition: 'file(\"Blank.esm\")'\n"
                "req:\n"
                "  - name: 'Blank.esm'\n"
                "    condition: 'file(\"Blank.missing.esm\")'\n"
                "inc:\n"
                "  - name: 'Blank.esm'\n"
                "    condition: 'file(\"Blank.esm\")'\n"
                "msg:\n"
                "  - type: say\n"
                "    content: 'content'\n"
                "    condition: 'condition'\n"
                "tag:\n"
                "  - name: Relev\n"
                "    condition: 'file(\"Blank.missing.esm\")'"
                );
            EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);

            node = YAML::Load("scalar");
            EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);

            node = YAML::Load("[0, 1, 2]");
            EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);
        }
    }
}

#endif
