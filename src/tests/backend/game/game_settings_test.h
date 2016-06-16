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

#ifndef LOOT_TEST_BACKEND_GAME_SETTINGS
#define LOOT_TEST_BACKEND_GAME_SETTINGS

#include "backend/app/loot_paths.h"
#include "backend/game/game_settings.h"

#include "tests/base_game_test.h"

namespace loot {
    namespace test {
        class GameSettingsTest : public BaseGameTest {
        protected:
            GameSettings game;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        // Just test with one game because if it works for one it will work for them
        // all.
        INSTANTIATE_TEST_CASE_P(,
                                GameSettingsTest,
                                ::testing::Values(
                                    GameSettings::tes5));

        TEST_P(GameSettingsTest, defaultConstructorShouldInitialiseIdToAutodetectAndAllOtherSettingsToEmptyStrings) {
            EXPECT_EQ(GameSettings::autodetect, game.Id());
            EXPECT_EQ("", game.Name());
            EXPECT_EQ("", game.FolderName());
            EXPECT_EQ("", game.Master());
            EXPECT_EQ("", game.RegistryKey());
            EXPECT_EQ("", game.RepoURL());
            EXPECT_EQ("", game.RepoBranch());

            EXPECT_EQ("", game.GamePath());
            EXPECT_EQ("", game.DataPath());
            EXPECT_EQ("", game.MasterlistPath());
            EXPECT_EQ("", game.UserlistPath());

            EXPECT_TRUE(game.getSupportedBashTags().empty());
        }

        TEST_P(GameSettingsTest, idConstructorShouldInitialiseSettingsToDefaultsForThatGame) {
            game = GameSettings(GameSettings::tes5);

            EXPECT_EQ(GameSettings::tes5, game.Id());
            EXPECT_EQ("TES V: Skyrim", game.Name());
            EXPECT_EQ("Skyrim", game.FolderName());
            EXPECT_EQ("Skyrim.esm", game.Master());
            EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path", game.RegistryKey());
            EXPECT_EQ("https://github.com/loot/skyrim.git", game.RepoURL());
            // Repo branch changes between LOOT versions, so don't check an exact value.
            EXPECT_NE("", game.RepoBranch());

            EXPECT_EQ("", game.GamePath());
            EXPECT_EQ("", game.DataPath());
            EXPECT_EQ(LootPaths::getLootDataPath() / "Skyrim" / "masterlist.yaml", game.MasterlistPath());
            EXPECT_EQ(LootPaths::getLootDataPath() / "Skyrim" / "userlist.yaml", game.UserlistPath());
        }

        TEST_P(GameSettingsTest, idConstructorShouldSetGameFolderIfGiven) {
            game = GameSettings(GameSettings::tes5, "folder");

            EXPECT_EQ("folder", game.FolderName());
            EXPECT_EQ(LootPaths::getLootDataPath() / "folder" / "masterlist.yaml", game.MasterlistPath());
            EXPECT_EQ(LootPaths::getLootDataPath() / "folder" / "userlist.yaml", game.UserlistPath());
        }

        TEST_P(GameSettingsTest, idConstructorShouldSetCorrectBashTagsForTES4) {
            game = GameSettings(GameSettings::tes4);

            std::vector<std::string> expectedBashTags = {
                "Actors.ACBS",
                "Actors.AIData",
                "Actors.AIPackages",
                "Actors.AIPackagesForceAdd",
                "Actors.Animations",
                "Actors.CombatStyle",
                "Actors.DeathItem",
                "Actors.Skeleton",
                "Actors.Spells",
                "Actors.SpellsForceAdd",
                "Actors.Stats",
                "Body-F",
                "Body-M",
                "Body-Size-F",
                "Body-Size-M",
                "C.Climate",
                "C.Light",
                "C.Music",
                "C.Name",
                "C.Owner",
                "C.RecordFlags",
                "C.Water",
                "Creatures.Blood",
                "Deactivate",
                "Delev",
                "Eyes",
                "Factions",
                "Filter",
                "Graphics",
                "Hair",
                "IIM",
                "Invent",
                "InventOnly",
                "Merge",
                "MustBeActiveIfImported",
                "Names",
                "NoMerge",
                "NpcFaces",
                "NpcFacesForceFullImport",
                "NPC.Class",
                "Npc.EyesOnly",
                "Npc.HairOnly",
                "NPC.Race",
                "Relations",
                "Relev",
                "Roads",
                "R.AddSpells",
                "R.Attributes-F",
                "R.Attributes-M",
                "R.ChangeSpells",
                "R.Description",
                "R.Ears",
                "R.Head",
                "R.Mouth",
                "R.Relations",
                "R.Skills",
                "R.Teeth",
                "ScriptContents",
                "Scripts",
                "Sound",
                "SpellStats",
                "Stats",
                "Voice-F",
                "Voice-M",
            };
            EXPECT_EQ(expectedBashTags, game.getSupportedBashTags());
        }

        TEST_P(GameSettingsTest, idConstructorShouldSetCorrectBashTagsForTES5) {
            game = GameSettings(GameSettings::tes5);

            std::vector<std::string> expectedBashTags = {
                "Actors.ACBS",
                "Actors.AIData",
                "Actors.AIPackages",
                "Actors.AIPackagesForceAdd",
                "Actors.Animations",
                "Actors.CombatStyle",
                "Actors.DeathItem",
                "Actors.Skeleton",
                "Actors.Spells",
                "Actors.SpellsForceAdd",
                "Actors.Stats",
                "Body-F",
                "Body-M",
                "Body-Size-F",
                "Body-Size-M",
                "C.Acoustic",
                "C.Climate",
                "C.ImageSpace",
                "C.Light",
                "C.Location",
                "C.Music",
                "C.Name",
                "C.Owner",
                "C.RecordFlags",
                "C.Regions",
                "C.SkyLighting",
                "C.Water",
                "Creatures.Blood",
                "Deactivate",
                "Delev",
                "Eyes",
                "Factions",
                "Filter",
                "Graphics",
                "Hair",
                "IIM",
                "Invent",
                "InventOnly",
                "Merge",
                "MustBeActiveIfImported",
                "Names",
                "NoMerge",
                "NpcFaces",
                "NpcFacesForceFullImport",
                "NPC.Class",
                "Npc.EyesOnly",
                "Npc.HairOnly",
                "NPC.Race",
                "Relations",
                "Relev",
                "Roads",
                "R.AddSpells",
                "R.Attributes-F",
                "R.Attributes-M",
                "R.ChangeSpells",
                "R.Description",
                "R.Ears",
                "R.Head",
                "R.Mouth",
                "R.Relations",
                "R.Skills",
                "R.Teeth",
                "ScriptContents",
                "Scripts",
                "Sound",
                "SpellStats",
                "Stats",
                "Voice-F",
                "Voice-M",
            };
            EXPECT_EQ(expectedBashTags, game.getSupportedBashTags());
        }

        TEST_P(GameSettingsTest, idConstructorShouldSetNoBashTagsForFO3) {
            game = GameSettings(GameSettings::fo3);

            std::vector<std::string> expectedBashTags = {
                "Deflst",
                "Destructible",
            };
            EXPECT_EQ(expectedBashTags, game.getSupportedBashTags());
        }

        TEST_P(GameSettingsTest, idConstructorShouldSetNoBashTagsForFONV) {
            game = GameSettings(GameSettings::fonv);

            std::vector<std::string> expectedBashTags = {
                "Deflst",
                "Destructible",
                "WeaponMods",
            };
            EXPECT_EQ(expectedBashTags, game.getSupportedBashTags());
        }

        TEST_P(GameSettingsTest, idConstructorShouldSetNoBashTagsForFO4) {
            EXPECT_TRUE(GameSettings(GameSettings::fo4).getSupportedBashTags().empty());
        }

        TEST_P(GameSettingsTest, isInstalledShouldBeFalseIfGamePathIsNotSet) {
            GameSettings game;
            EXPECT_FALSE(game.IsInstalled());
        }

        TEST_P(GameSettingsTest, isInstalledShouldBeTrueIfGamePathIsValid) {
            game = GameSettings(GameSettings::tes5);
            game.SetGamePath(dataPath.parent_path());
            EXPECT_TRUE(game.IsInstalled());
        }

        TEST_P(GameSettingsTest, gameSettingsWithTheSameIdsShouldBeEqual) {
            GameSettings game1 = GameSettings(GameSettings::tes5, "game1")
                .SetMaster("master1")
                .SetRegistryKey("key1")
                .SetRepoURL("url1")
                .SetRepoBranch("branch1")
                .SetGamePath("path1");
            GameSettings game2 = GameSettings(GameSettings::tes5, "game2")
                .SetMaster("master2")
                .SetRegistryKey("key2")
                .SetRepoURL("url2")
                .SetRepoBranch("branch2")
                .SetGamePath("path2");

            EXPECT_TRUE(game1 == game2);
        }

        TEST_P(GameSettingsTest, gameSettingsWithTheSameNameShouldBeEqual) {
            GameSettings game1 = GameSettings(GameSettings::tes4)
                .SetName("name");
            GameSettings game2 = GameSettings(GameSettings::tes5)
                .SetName("name");

            EXPECT_TRUE(game1 == game2);
        }

        TEST_P(GameSettingsTest, gameSettingsWithDifferentIdsAndNamesShouldNotBeEqual) {
            GameSettings game1 = GameSettings(GameSettings::tes4);
            GameSettings game2 = GameSettings(GameSettings::tes5);

            EXPECT_FALSE(game1 == game2);
        }

        TEST_P(GameSettingsTest, setNameShouldStoreGivenValue) {
            GameSettings game;
            game.SetName("name");
            EXPECT_EQ("name", game.Name());
        }

        TEST_P(GameSettingsTest, setMasterShouldStoreGivenValue) {
            GameSettings game;
            game.SetMaster("master");
            EXPECT_EQ("master", game.Master());
        }

        TEST_P(GameSettingsTest, setRegistryKeyShouldStoreGivenValue) {
            GameSettings game;
            game.SetRegistryKey("key");
            EXPECT_EQ("key", game.RegistryKey());
        }

        TEST_P(GameSettingsTest, setRepoUrlShouldStoreGivenValue) {
            GameSettings game;
            game.SetRepoURL("url");
            EXPECT_EQ("url", game.RepoURL());
        }

        TEST_P(GameSettingsTest, setRepoBranchShouldStoreGivenValue) {
            GameSettings game;
            game.SetRepoBranch("branch");
            EXPECT_EQ("branch", game.RepoBranch());
        }

        TEST_P(GameSettingsTest, setGamePathShouldStoreGivenValue) {
            std::string pathValue = "path";
            GameSettings game;

            game.SetGamePath(pathValue);
            EXPECT_EQ(pathValue, game.GamePath().string());
            EXPECT_EQ(boost::filesystem::path(pathValue) / "Data", game.DataPath());
        }

        TEST_P(GameSettingsTest, emittingYamlShouldSerialiseDataCorrectly) {
            GameSettings game(GameSettings::tes5, "folder1");
            game.SetName("name1")
                .SetMaster("master1")
                .SetRegistryKey("key1")
                .SetRepoURL("url1")
                .SetRepoBranch("branch1")
                .SetGamePath("path1");

            YAML::Emitter e;
            e << game;
            EXPECT_STREQ("type: 'Skyrim'\n"
                         "folder: 'folder1'\n"
                         "name: 'name1'\n"
                         "master: 'master1'\n"
                         "repo: 'url1'\n"
                         "branch: 'branch1'\n"
                         "path: 'path1'\n"
                         "registry: 'key1'", e.c_str());
        }

        TEST_P(GameSettingsTest, encodingAsYamlShouldConvertDataCorrectly) {
            GameSettings game(GameSettings::tes5, "folder1");
            game.SetName("name1")
                .SetMaster("master1")
                .SetRegistryKey("key1")
                .SetRepoURL("url1")
                .SetRepoBranch("branch1")
                .SetGamePath("path1");

            YAML::Node node;
            node = game;
            EXPECT_EQ("Skyrim", node["type"].as<std::string>());
            EXPECT_EQ("folder1", node["folder"].as<std::string>());
            EXPECT_EQ("name1", node["name"].as<std::string>());
            EXPECT_EQ("master1", node["master"].as<std::string>());
            EXPECT_EQ("url1", node["repo"].as<std::string>());
            EXPECT_EQ("branch1", node["branch"].as<std::string>());
            EXPECT_EQ("path1", node["path"].as<std::string>());
            EXPECT_EQ("key1", node["registry"].as<std::string>());
        }

        TEST_P(GameSettingsTest, decodingFromYamlShouldInterpretTheYamlCorrectly) {
            YAML::Node node = YAML::Load("type: 'Skyrim'\n"
                                         "folder: 'folder1'\n"
                                         "name: 'name1'\n"
                                         "master: 'master1'\n"
                                         "repo: 'url1'\n"
                                         "branch: 'branch1'\n"
                                         "path: 'path1'\n"
                                         "registry: 'key1'");

            GameSettings game = node.as<GameSettings>();
            EXPECT_EQ(GameSettings::tes5, game.Id());
            EXPECT_EQ("name1", game.Name());
            EXPECT_EQ("folder1", game.FolderName());
            EXPECT_EQ("master1", game.Master());
            EXPECT_EQ("key1", game.RegistryKey());
            EXPECT_EQ("url1", game.RepoURL());
            EXPECT_EQ("branch1", game.RepoBranch());
            EXPECT_EQ("path1", game.GamePath());
        }

        TEST_P(GameSettingsTest, decodingFromAnInvalidYamlMapShouldThrowAnException) {
            YAML::Node node = YAML::Load("type: 'Invalid'\n");
            EXPECT_ANY_THROW(node.as<GameSettings>());
        }

        TEST_P(GameSettingsTest, decodingFromAYamlScalarShouldThrowAnException) {
            YAML::Node node = YAML::Load("scalar");
            EXPECT_ANY_THROW(node.as<GameSettings>());
        }

        TEST_P(GameSettingsTest, decodingFromAYamlListShouldThrowAnException) {
            YAML::Node node = YAML::Load("[0, 1, 2]");
            EXPECT_ANY_THROW(node.as<GameSettings>());
        }

        TEST_P(GameSettingsTest, decodingFromAnIncompleteYamlMapShouldUseDefaultValuesForMissingSettings) {
            // Test inheritance of unspecified settings.
            YAML::Node node = YAML::Load("type: 'Skyrim'\n"
                                         "folder: 'folder1'\n"
                                         "master: 'master1'\n"
                                         "repo: 'url1'\n"
                                         "branch: 'branch1'\n");

            game = node.as<GameSettings>();
            EXPECT_EQ(GameSettings::tes5, game.Id());
            EXPECT_EQ("TES V: Skyrim", game.Name());
            EXPECT_EQ("Software\\Bethesda Softworks\\Skyrim\\Installed Path", game.RegistryKey());
            EXPECT_EQ("", game.GamePath());
        }
    }
}
#endif
