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

#ifndef LOOT_TEST_BACKEND_METADATA_LIST
#define LOOT_TEST_BACKEND_METADATA_LIST

#include "backend/metadata_list.h"
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class MetadataListTest : public SkyrimTest {
        protected:
            MetadataListTest() :
                metadataPath("./testing-metadata/masterlist.yaml"),
                savedMetadataPath("./testing-metadata/saved.masterlist.yaml"),
                invalidMetadataPaths({"./testing-metadata/invalid/non_unique.yaml"}) {
                PluginMetadataToString = [](const PluginMetadata& plugin) {
                    return plugin.Name();
                };
            }

            inline virtual void SetUp() {
                SkyrimTest::SetUp();

                ASSERT_TRUE(boost::filesystem::exists(metadataPath));
                ASSERT_FALSE(boost::filesystem::exists(savedMetadataPath));

                for (const auto& path : invalidMetadataPaths) {
                    ASSERT_TRUE(boost::filesystem::exists(path));
                }
            }

            inline virtual void TearDown() {
                SkyrimTest::TearDown();

                ASSERT_TRUE(boost::filesystem::exists(metadataPath));
                ASSERT_NO_THROW(boost::filesystem::remove(savedMetadataPath));

                for (const auto& path : invalidMetadataPaths) {
                    ASSERT_TRUE(boost::filesystem::exists(path));
                }
            }

            const boost::filesystem::path metadataPath;
            const boost::filesystem::path savedMetadataPath;
            const std::vector<boost::filesystem::path> invalidMetadataPaths;

            std::function<std::string(const PluginMetadata&)> PluginMetadataToString;
        };

        TEST_F(MetadataListTest, Load) {
            MetadataList ml;
            EXPECT_NO_THROW(ml.Load(metadataPath));
            EXPECT_EQ(std::list<Message>({
                Message(Message::say, "A global message."),
            }), ml.Messages());

            // Non-regex plugins can be outputted in any order, and regex entries can
            // match each other, so convert the list to a set of strings for
            // comparison.
            std::list<PluginMetadata> result(ml.Plugins());
            std::set<std::string> names;
            std::transform(result.begin(),
                           result.end(),
                           std::insert_iterator<std::set<std::string>>(names, names.begin()),
                           PluginMetadataToString);
            EXPECT_EQ(std::set<std::string>({
                "Blank.esm",
                "Blank.esp",
                "Blank.+\\.esp",
                "Blank.+(Different)?.*\\.esp",
            }), names);

            EXPECT_ANY_THROW(ml.Load("NotAPlugin.esm"));
            EXPECT_TRUE(ml.Messages().empty());
            EXPECT_TRUE(ml.Plugins().empty());

            // Fill the list again.
            ASSERT_NO_THROW(ml.Load(metadataPath));

            EXPECT_ANY_THROW(ml.Load("Blank.missing.esm"));
            EXPECT_TRUE(ml.Messages().empty());
            EXPECT_TRUE(ml.Plugins().empty());
        }

        TEST_F(MetadataListTest, Load_Invalid) {
            MetadataList ml;
            for (const auto& path : invalidMetadataPaths) {
                EXPECT_ANY_THROW(ml.Load(path));
            }
        }

        TEST_F(MetadataListTest, Save) {
            MetadataList ml;
            ASSERT_NO_THROW(ml.Load(metadataPath));

            EXPECT_NO_THROW(ml.Save(savedMetadataPath));
            EXPECT_TRUE(boost::filesystem::exists(savedMetadataPath));
            EXPECT_NO_THROW(ml.Load(savedMetadataPath));

            EXPECT_EQ(std::list<Message>({
                Message(Message::say, "A global message."),
            }), ml.Messages());

            // Non-regex plugins can be outputted in any order, and regex entries can
            // match each other, so convert the list to a set of strings for
            // comparison.
            std::list<PluginMetadata> result(ml.Plugins());
            std::set<std::string> names;
            std::transform(result.begin(),
                           result.end(),
                           std::insert_iterator<std::set<std::string>>(names, names.begin()),
                           PluginMetadataToString);
            EXPECT_EQ(std::set<std::string>({
                "Blank.esm",
                "Blank.esp",
                "Blank.+\\.esp",
                "Blank.+(Different)?.*\\.esp",
            }), names);
        }

        TEST_F(MetadataListTest, clear) {
            MetadataList ml;
            ASSERT_NO_THROW(ml.Load(metadataPath));
            ASSERT_FALSE(ml.Messages().empty());
            ASSERT_FALSE(ml.Plugins().empty());

            ml.clear();
            EXPECT_TRUE(ml.Messages().empty());
            EXPECT_TRUE(ml.Plugins().empty());
        }

        TEST_F(MetadataListTest, FindPlugin) {
            MetadataList ml;
            ASSERT_NO_THROW(ml.Load(metadataPath));

            PluginMetadata pm = ml.FindPlugin(PluginMetadata("Blank - Different.esm"));
            EXPECT_EQ("Blank - Different.esm", pm.Name());
            EXPECT_TRUE(pm.HasNameOnly());

            pm = ml.FindPlugin(PluginMetadata("Blank - Different.esp"));
            EXPECT_EQ("Blank - Different.esp", pm.Name());
            EXPECT_EQ(std::set<File>({
                File("Blank.esm"),
            }), pm.LoadAfter());
            EXPECT_EQ(std::set<File>({
                File("Blank.esp"),
            }), pm.Incs());
        }

        TEST_F(MetadataListTest, AddPlugin) {
            MetadataList ml;
            ASSERT_NO_THROW(ml.Load(metadataPath));

            PluginMetadata pm = ml.FindPlugin(PluginMetadata("Blank - Different.esm"));
            ASSERT_EQ("Blank - Different.esm", pm.Name());
            ASSERT_TRUE(pm.HasNameOnly());

            pm.Priority(1000);
            ml.AddPlugin(pm);
            pm = ml.FindPlugin(pm);
            EXPECT_EQ("Blank - Different.esm", pm.Name());
            EXPECT_EQ(1000, pm.Priority());

            pm = PluginMetadata(".+Dependent\\.esp");
            pm.Priority(-10);
            ml.AddPlugin(pm);
            pm = ml.FindPlugin(PluginMetadata("Blank - Plugin Dependent.esp"));
            EXPECT_EQ(-10, pm.Priority());
        }

        TEST_F(MetadataListTest, AddPlugin_NonUnique) {
            MetadataList ml;
            ASSERT_NO_THROW(ml.Load(metadataPath));

            PluginMetadata pm = ml.FindPlugin(PluginMetadata("Blank.esm"));
            ASSERT_EQ("Blank.esm", pm.Name());
            ASSERT_FALSE(pm.HasNameOnly());

            ASSERT_ANY_THROW(ml.AddPlugin(PluginMetadata("Blank.esm")));
        }

        TEST_F(MetadataListTest, ErasePlugin) {
            MetadataList ml;
            ASSERT_NO_THROW(ml.Load(metadataPath));

            PluginMetadata pm = ml.FindPlugin(PluginMetadata("Blank.esp"));
            ASSERT_EQ("Blank.esp", pm.Name());
            ASSERT_FALSE(pm.HasNameOnly());

            ml.ErasePlugin(pm);
            pm = ml.FindPlugin(pm);
            ASSERT_EQ("Blank.esp", pm.Name());
            ASSERT_TRUE(pm.HasNameOnly());
        }

        TEST_F(MetadataListTest, EvalAllConditions) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            MetadataList ml;
            ASSERT_NO_THROW(ml.Load(metadataPath));

            PluginMetadata pm = ml.FindPlugin(PluginMetadata("Blank.esm"));
            ASSERT_EQ(std::list<Message>({
                Message(Message::warn, "This is a warning."),
                Message(Message::say, "This message should be removed when evaluating conditions."),
            }), pm.Messages());

            pm = ml.FindPlugin(PluginMetadata("Blank.esp"));
            ASSERT_EQ("Blank.esp", pm.Name());
            ASSERT_FALSE(pm.HasNameOnly());

            EXPECT_NO_THROW(ml.EvalAllConditions(game, Language::english));

            pm = ml.FindPlugin(PluginMetadata("Blank.esm"));
            EXPECT_EQ(std::list<Message>({
                Message(Message::warn, "This is a warning."),
            }), pm.Messages());

            pm = ml.FindPlugin(PluginMetadata("Blank.esp"));
            EXPECT_EQ("Blank.esp", pm.Name());
            EXPECT_TRUE(pm.HasNameOnly());
        }
    }
}

#endif
