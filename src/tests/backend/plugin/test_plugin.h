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

#ifndef LOOT_TEST_BACKEND_PLUGIN
#define LOOT_TEST_BACKEND_PLUGIN

#include "backend/plugin/plugin.h"
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class PluginTest : public SkyrimTest {
        protected:
            inline virtual void SetUp() {
                SkyrimTest::SetUp();

                game = Game(Game::tes5);
                game.SetGamePath(dataPath.parent_path());
                ASSERT_NO_THROW(game.Init(false, localPath));
            }

            Game game;
        };

        TEST_F(PluginTest, ConstructorsAndDataAccess) {
            Plugin plugin(game, "Blank.esm", true);
            EXPECT_EQ("Blank.esm", plugin.Name());
            EXPECT_TRUE(plugin.getFormIds().empty());
            EXPECT_TRUE(plugin.getMasters().empty());
            EXPECT_TRUE(plugin.isMasterFile());
            EXPECT_FALSE(plugin.IsEmpty());
            EXPECT_EQ("v5.0", plugin.getDescription());
            EXPECT_EQ(0, plugin.Crc());

            plugin = Plugin(game, "Blank.esm", false);
            EXPECT_EQ("Blank.esm", plugin.Name());
            EXPECT_EQ(std::set<libespm::FormId>({
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF0),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF1),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF2),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF3),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF4),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF5),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF6),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF7),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF8),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF9),
            }), plugin.getFormIds());
            EXPECT_TRUE(plugin.getMasters().empty());
            EXPECT_TRUE(plugin.isMasterFile());
            EXPECT_FALSE(plugin.IsEmpty());
            EXPECT_EQ("v5.0", plugin.getDescription());
            EXPECT_EQ(0x187BE342, plugin.Crc());

            plugin = Plugin(game, "Blank - Master Dependent.esp", false);
            EXPECT_EQ("Blank - Master Dependent.esp", plugin.Name());
            EXPECT_FALSE(plugin.IsEmpty());
            EXPECT_FALSE(plugin.isMasterFile());
            EXPECT_EQ(std::set<libespm::FormId>({
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF0),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF1),
                libespm::FormId("Blank - Master Dependent.esp", std::vector<std::string>(), 0xCE9),
                libespm::FormId("Blank - Master Dependent.esp", std::vector<std::string>(), 0xCEA),
            }), plugin.getFormIds());
            EXPECT_EQ(std::vector<std::string>({
                "Blank.esm"
            }), plugin.getMasters());
            EXPECT_EQ("", plugin.getDescription());
            EXPECT_EQ(0x832152DC, plugin.Crc());
        }

        TEST_F(PluginTest, LoadsArchive) {
            EXPECT_FALSE(Plugin(game, "Blank - Different.esm", true).LoadsArchive());
            EXPECT_FALSE(Plugin(game, "Blank\\.esm", true).LoadsArchive());
            EXPECT_TRUE(Plugin(game, "Blank.esm", true).LoadsArchive());
        }

        TEST_F(PluginTest, IsValid) {
            EXPECT_TRUE(Plugin::IsValid("Blank.esm", game));
            EXPECT_FALSE(Plugin::IsValid("NotAPlugin.esm", game));
            EXPECT_FALSE(Plugin::IsValid("EmptyFile.esm", game));
        }

        TEST_F(PluginTest, IsActive) {
            Plugin plugin(game, "Blank.esm", true);
            EXPECT_TRUE(plugin.IsActive());

            plugin = Plugin(game, "Blank.esp", true);
            EXPECT_FALSE(plugin.IsActive());
        }

        TEST_F(PluginTest, EqualityOperator) {
            Plugin plugin1(game, "Blank.esm", true);
            Plugin plugin2(game, "blank.esm", true);
            EXPECT_TRUE(plugin1 == plugin2);
            EXPECT_TRUE(plugin2 == plugin1);

            plugin2 = Plugin(game, "Blank.esp", true);
            EXPECT_FALSE(plugin1 == plugin2);
            EXPECT_FALSE(plugin2 == plugin1);

            plugin2 = Plugin(game, "Blan.\\.esm", true);
            EXPECT_TRUE(plugin1 == plugin2);
            EXPECT_TRUE(plugin2 == plugin1);

            plugin1 = Plugin(game, "Blan.esm", true);
            EXPECT_FALSE(plugin1 == plugin2);
            EXPECT_FALSE(plugin2 == plugin1);

            plugin1 = Plugin(game, "Blan.\\.esm", true);
            EXPECT_TRUE(plugin1 == plugin2);
            EXPECT_TRUE(plugin2 == plugin1);

            plugin1 = Plugin(game, "Blan(k|p).esm", true);
            EXPECT_FALSE(plugin1 == plugin2);
            EXPECT_FALSE(plugin2 == plugin1);
        }

        TEST_F(PluginTest, InequalityOperator) {
            Plugin plugin1(game, "Blank.esm", true);
            Plugin plugin2(game, "blank.esm", true);
            EXPECT_FALSE(plugin1 != plugin2);
            EXPECT_FALSE(plugin2 != plugin1);

            plugin2 = Plugin(game, "Blank.esp", true);
            EXPECT_TRUE(plugin1 != plugin2);
            EXPECT_TRUE(plugin2 != plugin1);

            plugin2 = Plugin(game, "Blan.\\.esm", true);
            EXPECT_FALSE(plugin1 != plugin2);
            EXPECT_FALSE(plugin2 != plugin1);

            plugin1 = Plugin(game, "Blan.esm", true);
            EXPECT_TRUE(plugin1 != plugin2);
            EXPECT_TRUE(plugin2 != plugin1);

            plugin1 = Plugin(game, "Blan.\\.esm", true);
            EXPECT_FALSE(plugin1 != plugin2);
            EXPECT_FALSE(plugin2 != plugin1);

            plugin1 = Plugin(game, "Blan(k|p).esm", true);
            EXPECT_TRUE(plugin1 != plugin2);
            EXPECT_TRUE(plugin2 != plugin1);
        }

        TEST_F(PluginTest, DoFormIDsOverlap) {
            Plugin plugin1(game, "Blank.esm", true);
            Plugin plugin2(game, "blank.esm", true);
            EXPECT_FALSE(plugin1.DoFormIDsOverlap(plugin2));
            EXPECT_FALSE(plugin2.DoFormIDsOverlap(plugin1));

            plugin1 = Plugin(game, "Blank.esm", true);
            plugin2 = Plugin(game, "Blank - Master Dependent.esm", true);
            EXPECT_FALSE(plugin1.DoFormIDsOverlap(plugin2));
            EXPECT_FALSE(plugin2.DoFormIDsOverlap(plugin1));

            plugin1 = Plugin(game, "Blank.esm", false);
            plugin2 = Plugin(game, "Blank.esp", false);
            EXPECT_FALSE(plugin1.DoFormIDsOverlap(plugin2));
            EXPECT_FALSE(plugin2.DoFormIDsOverlap(plugin1));

            plugin1 = Plugin(game, "Blank.esm", false);
            plugin2 = Plugin(game, "Blank - Master Dependent.esm", false);
            EXPECT_TRUE(plugin1.DoFormIDsOverlap(plugin2));
            EXPECT_TRUE(plugin2.DoFormIDsOverlap(plugin1));
        }

        TEST_F(PluginTest, OverlapFormIDs) {
            Plugin plugin1(game, "Blank.esm", true);
            Plugin plugin2(game, "blank.esm", true);
            EXPECT_TRUE(plugin1.OverlapFormIDs(plugin2).empty());
            EXPECT_TRUE(plugin2.OverlapFormIDs(plugin1).empty());

            plugin1 = Plugin(game, "Blank.esm", true);
            plugin2 = Plugin(game, "Blank - Master Dependent.esm", true);
            EXPECT_TRUE(plugin1.OverlapFormIDs(plugin2).empty());
            EXPECT_TRUE(plugin2.OverlapFormIDs(plugin1).empty());

            plugin1 = Plugin(game, "Blank.esm", false);
            plugin2 = Plugin(game, "Blank.esp", false);
            EXPECT_TRUE(plugin1.OverlapFormIDs(plugin2).empty());
            EXPECT_TRUE(plugin2.OverlapFormIDs(plugin1).empty());

            plugin1 = Plugin(game, "Blank.esm", false);
            plugin2 = Plugin(game, "Blank - Master Dependent.esm", false);
            EXPECT_EQ(std::set<libespm::FormId>({
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF0),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF1),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF2),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF3),
            }), plugin1.OverlapFormIDs(plugin2));
            EXPECT_EQ(std::set<libespm::FormId>({
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF0),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF1),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF2),
                libespm::FormId("Blank.esm", std::vector<std::string>(), 0xCF3),
            }), plugin2.OverlapFormIDs(plugin1));
        }

        TEST_F(PluginTest, CheckInstallValidity) {
            Plugin plugin(game, "Blank.esm", false);
            plugin.Reqs({
                File("Blank.missing.esm"),
                File("Blank.esp"),
                File("Blank - Master Dependent.esm"),
            });
            plugin.Incs({
                File("Blank - Different.esm"),
                File("Skyrim.esm"),
            });
            plugin.DirtyInfo({
                PluginDirtyInfo(0x187BE342, 0, 1, 2, "utility1"),
                PluginDirtyInfo(0xDEADBEEF, 0, 5, 10, "utility2"),
            });
            EXPECT_TRUE(plugin.CheckInstallValidity(game));
            EXPECT_EQ(std::list<Message>({
                Message(Message::error, "This plugin requires \"Blank.missing.esm\" to be installed, but it is missing."),
                Message(Message::error, "This plugin is incompatible with \"Skyrim.esm\", but both are present."),
                PluginDirtyInfo(0x187BE342, 0, 1, 2, "utility1").AsMessage(),
                PluginDirtyInfo(0xDEADBEEF, 0, 5, 10, "utility2").AsMessage(),
            }), plugin.Messages());

            plugin = Plugin(game, "Blank - Different Master Dependent.esp", false);
            EXPECT_FALSE(plugin.CheckInstallValidity(game));
            EXPECT_EQ(std::list<Message>({
                Message(Message::error, "This plugin requires \"Blank - Different.esm\" to be active, but it is inactive."),
            }), plugin.Messages());

            plugin = Plugin(game, "Blank - Different Master Dependent.esp", false);
            plugin.Tags({Tag("Filter")});
            EXPECT_FALSE(plugin.CheckInstallValidity(game));
            EXPECT_TRUE(plugin.Messages().empty());
        }
    }
}

#endif
