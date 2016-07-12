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

#ifndef LOOT_TEST_GUI_LOOT_SETTINGS
#define LOOT_TEST_GUI_LOOT_SETTINGS

#include "backend/app/loot_settings.h"
#include "backend/app/loot_version.h"

#include <gtest/gtest.h>

namespace loot {
    namespace test {
        class LootSettingsTest : public ::testing::Test {
        protected:
            LootSettingsTest() : settingsFile("./settings.yaml") {}

            ~LootSettingsTest() {
                boost::filesystem::remove(settingsFile);
            }

            boost::filesystem::path settingsFile;
            LootSettings settings;
        };

        TEST_F(LootSettingsTest, defaultConstructorShouldSetDefaultValues) {
            const std::string currentVersion = LootVersion::string();
            const std::vector<GameSettings> expectedGameSettings({
                GameSettings(GameType::tes4),
                GameSettings(GameType::tes5),
                GameSettings(GameType::fo3),
                GameSettings(GameType::fonv),
                GameSettings(GameType::fo4),
                GameSettings(GameType::tes4, "Nehrim")
                    .SetName("Nehrim - At Fate's Edge")
                    .SetMaster("Nehrim.esm")
                    .SetRegistryKey("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1\\InstallLocation"),
            });

            EXPECT_FALSE(settings.isDebugLoggingEnabled());
            EXPECT_EQ("auto", settings.getGame());
            EXPECT_EQ("en", settings.getLanguage().GetLocale());
            EXPECT_EQ("auto", settings.getLastGame());
            EXPECT_FALSE(settings.isWindowPositionStored());

            const YAML::Node node = settings.toYaml();
            EXPECT_TRUE(node["updateMasterlist"].as<bool>());
            EXPECT_TRUE(node["lastVersion"].as<std::string>().empty());
            EXPECT_FALSE(node["filters"]);

            // GameSettings equality only checks name and folder, so check
            // other settings individually.
            const std::vector<GameSettings> actualGameSettings = settings.getGameSettings();
            EXPECT_EQ(expectedGameSettings, actualGameSettings);

            EXPECT_EQ(expectedGameSettings[0].Type(), actualGameSettings[0].Type());
            EXPECT_EQ(expectedGameSettings[0].Master(), actualGameSettings[0].Master());
            EXPECT_EQ(expectedGameSettings[0].RegistryKey(), actualGameSettings[0].RegistryKey());
            EXPECT_EQ(expectedGameSettings[0].RepoURL(), actualGameSettings[0].RepoURL());
            EXPECT_EQ(expectedGameSettings[0].RepoBranch(), actualGameSettings[0].RepoBranch());

            EXPECT_EQ(expectedGameSettings[1].Type(), actualGameSettings[1].Type());
            EXPECT_EQ(expectedGameSettings[1].Master(), actualGameSettings[1].Master());
            EXPECT_EQ(expectedGameSettings[1].RegistryKey(), actualGameSettings[1].RegistryKey());
            EXPECT_EQ(expectedGameSettings[1].RepoURL(), actualGameSettings[1].RepoURL());
            EXPECT_EQ(expectedGameSettings[1].RepoBranch(), actualGameSettings[1].RepoBranch());

            EXPECT_EQ(expectedGameSettings[2].Type(), actualGameSettings[2].Type());
            EXPECT_EQ(expectedGameSettings[2].Master(), actualGameSettings[2].Master());
            EXPECT_EQ(expectedGameSettings[2].RegistryKey(), actualGameSettings[2].RegistryKey());
            EXPECT_EQ(expectedGameSettings[2].RepoURL(), actualGameSettings[2].RepoURL());
            EXPECT_EQ(expectedGameSettings[2].RepoBranch(), actualGameSettings[2].RepoBranch());

            EXPECT_EQ(expectedGameSettings[3].Type(), actualGameSettings[3].Type());
            EXPECT_EQ(expectedGameSettings[3].Master(), actualGameSettings[3].Master());
            EXPECT_EQ(expectedGameSettings[3].RegistryKey(), actualGameSettings[3].RegistryKey());
            EXPECT_EQ(expectedGameSettings[3].RepoURL(), actualGameSettings[3].RepoURL());
            EXPECT_EQ(expectedGameSettings[3].RepoBranch(), actualGameSettings[3].RepoBranch());

            EXPECT_EQ(expectedGameSettings[4].Type(), actualGameSettings[4].Type());
            EXPECT_EQ(expectedGameSettings[4].Master(), actualGameSettings[4].Master());
            EXPECT_EQ(expectedGameSettings[4].RegistryKey(), actualGameSettings[4].RegistryKey());
            EXPECT_EQ(expectedGameSettings[4].RepoURL(), actualGameSettings[4].RepoURL());
            EXPECT_EQ(expectedGameSettings[4].RepoBranch(), actualGameSettings[4].RepoBranch());

            EXPECT_EQ(expectedGameSettings[5].Type(), actualGameSettings[5].Type());
            EXPECT_EQ(expectedGameSettings[5].Master(), actualGameSettings[5].Master());
            EXPECT_EQ(expectedGameSettings[5].RegistryKey(), actualGameSettings[5].RegistryKey());
            EXPECT_EQ(expectedGameSettings[5].RepoURL(), actualGameSettings[5].RepoURL());
            EXPECT_EQ(expectedGameSettings[5].RepoBranch(), actualGameSettings[5].RepoBranch());
        }

        TEST_F(LootSettingsTest, loadingFromFileShouldLoadContentAsYaml) {
            boost::filesystem::ofstream out(settingsFile);
            out << "enableDebugLogging: true" << std::endl;
            out.close();

            settings.load(settingsFile);

            EXPECT_TRUE(settings.isDebugLoggingEnabled());
        }

        TEST_F(LootSettingsTest, loadingFromYamlShouldStoreLoadedValues) {
            const bool enableDebugLogging = true;
            const bool updateMasterlist = true;
            const std::string game = "Oblivion";
            const std::string language = "fr";
            const std::string lastGame = "Skyrim";
            const std::string lastVersion = "0.7.1";
            const std::map<std::string, long> window({
                {"top", 1},
                {"bottom", 2},
                {"left", 3},
                {"right", 4},
            });
            const std::vector<GameSettings> games({
                GameSettings(GameType::tes4).SetName("Game Name"),
            });
            const std::map<std::string, bool> filters({
                {"hideBashTags", false},
                {"hideCRCs", true},
            });

            YAML::Node inputYaml;
            inputYaml["enableDebugLogging"] = enableDebugLogging;
            inputYaml["updateMasterlist"] = updateMasterlist;
            inputYaml["game"] = game;
            inputYaml["language"] = language;
            inputYaml["lastGame"] = lastGame;
            inputYaml["lastVersion"] = lastVersion;
            inputYaml["window"] = window;
            inputYaml["games"] = games;
            inputYaml["filters"] = filters;

            settings.load(inputYaml);

            EXPECT_EQ(enableDebugLogging, settings.isDebugLoggingEnabled());
            EXPECT_EQ(game, settings.getGame());
            EXPECT_EQ(language, settings.getLanguage().GetLocale());
            EXPECT_EQ(lastGame, settings.getLastGame());

            EXPECT_EQ(1, settings.getWindowPosition().top);
            EXPECT_EQ(2, settings.getWindowPosition().bottom);
            EXPECT_EQ(3, settings.getWindowPosition().left);
            EXPECT_EQ(4, settings.getWindowPosition().right);

            const YAML::Node outputYaml = settings.toYaml();
            EXPECT_EQ(updateMasterlist, outputYaml["updateMasterlist"].as<bool>());
            EXPECT_EQ(lastVersion, outputYaml["lastVersion"].as<std::string>());

            for (const auto& filter : filters) {
                EXPECT_EQ(filter.second, outputYaml["filters"][filter.first].as<bool>());
            }

            EXPECT_EQ(games[0].Name(), settings.getGameSettings()[0].Name());
        }

        TEST_F(LootSettingsTest, loadingFromEmptyYamlShouldNotThrow) {
            YAML::Node yaml;
            EXPECT_NO_THROW(settings.load(yaml));
        }

        TEST_F(LootSettingsTest, loadingFromYamlShouldUpgradeFromVersion0Point6Format) {
            const unsigned int DebugVerbosity = 3;
            const bool UpdateMasterlist = true;
            const std::string Game = "Oblivion";
            const std::string Language = "fr";
            const std::string LastGame = "Skyrim";
            const std::vector<GameSettings> Games({
                GameSettings(GameType::tes4).SetName("Game Name"),
            });

            YAML::Node inputYaml;
            inputYaml["Debug Verbosity"] = DebugVerbosity;
            inputYaml["Update Masterlist"] = UpdateMasterlist;
            inputYaml["Game"] = Game;
            inputYaml["Language"] = Language;
            inputYaml["Last Game"] = LastGame;

            inputYaml["Games"] = Games;
            inputYaml["Games"][0]["url"] = inputYaml["Games"][0]["repo"];
            inputYaml["Games"][0].remove("repo");
            inputYaml["Games"][0].remove("branch");

            settings.load(inputYaml);

            const YAML::Node outputYaml = settings.toYaml();
            EXPECT_TRUE(settings.isDebugLoggingEnabled());
            EXPECT_EQ(UpdateMasterlist, outputYaml["updateMasterlist"].as<bool>());
            EXPECT_EQ(Game, settings.getGame());
            EXPECT_EQ(Language, settings.getLanguage().GetLocale());
            EXPECT_EQ(LastGame, settings.getLastGame());

            EXPECT_EQ(Games[0].Name(), settings.getGameSettings()[0].Name());
            EXPECT_EQ(Games[0].RepoURL(), settings.getGameSettings()[0].RepoURL());
            EXPECT_EQ(Games[0].RepoBranch(), settings.getGameSettings()[0].RepoBranch());
        }

        TEST_F(LootSettingsTest, loadingFromYamlShouldNotUpgradeVersion0Point6SettingsIfEquivalentsAlreadyExist) {
            const unsigned int DebugVerbosity = 3;
            const bool enableDebugLogging = false;
            const bool UpdateMasterlist = true;
            const bool updateMasterlist = false;
            const std::string Game = "Oblivion";
            const std::string game = "auto";
            const std::string Language = "fr";
            const std::string language = "en";
            const std::string LastGame = "Skyrim";
            const std::string lastGame = "auto";
            const std::vector<GameSettings> Games({
                GameSettings(GameType::tes4).SetName("Old Game Name"),
            });
            const std::vector<GameSettings> games({
                GameSettings(GameType::fo3).SetName("Game Name"),
            });

            YAML::Node inputYaml;
            inputYaml["Debug Verbosity"] = DebugVerbosity;
            inputYaml["enableDebugLogging"] = enableDebugLogging;
            inputYaml["Update Masterlist"] = UpdateMasterlist;
            inputYaml["updateMasterlist"] = updateMasterlist;
            inputYaml["Game"] = Game;
            inputYaml["game"] = game;
            inputYaml["Language"] = Language;
            inputYaml["language"] = language;
            inputYaml["Last Game"] = LastGame;
            inputYaml["lastGame"] = lastGame;

            inputYaml["Games"] = Games;
            inputYaml["Games"][0]["url"] = inputYaml["Games"][0]["repo"];
            inputYaml["Games"][0].remove("repo");
            inputYaml["Games"][0].remove("branch");
            inputYaml["games"] = games;

            settings.load(inputYaml);

            const YAML::Node outputYaml = settings.toYaml();

            EXPECT_EQ(enableDebugLogging, settings.isDebugLoggingEnabled());
            EXPECT_EQ(updateMasterlist, outputYaml["updateMasterlist"].as<bool>());
            EXPECT_EQ(game, settings.getGame());
            EXPECT_EQ(language, settings.getLanguage().GetLocale());
            EXPECT_EQ(lastGame, settings.getLastGame());

            EXPECT_EQ(games[0].Name(), settings.getGameSettings()[0].Name());
        }

        TEST_F(LootSettingsTest, loadingFromYamlShouldUpgradeOldDefaultGameRepositoryBranches) {
            const std::vector<GameSettings> games({GameSettings(GameType::tes4)});

            YAML::Node inputYaml;
            inputYaml["games"] = games;
            inputYaml["games"][0]["branch"] = "v0.7";

            settings.load(inputYaml);

            EXPECT_EQ(games[0].RepoBranch(), settings.getGameSettings()[0].RepoBranch());
        }

        TEST_F(LootSettingsTest, loadingFromYamlShouldNotUpgradeNonDefaultGameRepositoryBranches) {
            const std::vector<GameSettings> games({GameSettings(GameType::tes4)});

            YAML::Node inputYaml;
            inputYaml["games"] = games;
            inputYaml["games"][0]["branch"] = "foo";

            settings.load(inputYaml);

            EXPECT_EQ("foo", settings.getGameSettings()[0].RepoBranch());
        }

        TEST_F(LootSettingsTest, loadingFromYamlShouldAddMissingBaseGames) {
            const std::vector<GameSettings> games({GameSettings(GameType::tes4)});
            YAML::Node inputYaml;
            inputYaml["games"] = games;

            settings.load(inputYaml);

            const std::vector<GameSettings> expectedGameSettings({
                GameSettings(GameType::tes4),
                GameSettings(GameType::tes5),
                GameSettings(GameType::fo3),
                GameSettings(GameType::fonv),
                GameSettings(GameType::fo4),
            });
            EXPECT_EQ(expectedGameSettings, settings.getGameSettings());
        }

        TEST_F(LootSettingsTest, loadingFromYamlShouldSkipUnrecognisedGames) {
            YAML::Node inputYaml;
            inputYaml["games"][0] = GameSettings(GameType::tes4);
            inputYaml["games"][0]["type"] = "Foobar";
            inputYaml["games"][0]["name"] = "Foobar";
            inputYaml["games"][1] = GameSettings(GameType::tes5).SetName("Game Name");

            settings.load(inputYaml);

            EXPECT_EQ("Game Name", settings.getGameSettings()[0].Name());
        }

        TEST_F(LootSettingsTest, loadingFromYamlShouldRemoveTheContentFilterSetting) {
            YAML::Node inputYaml;
            inputYaml["filters"]["contentFilter"] = "foo";

            settings.load(inputYaml);
        }

        TEST_F(LootSettingsTest, saveShouldWriteSettingsAsYamlToPassedFile) {
            settings.storeLastGame("Skyrim");
            settings.save(settingsFile);

            settings.storeLastGame("auto");
            settings.load(settingsFile);

            EXPECT_EQ("Skyrim", settings.getLastGame());
        }

        TEST_F(LootSettingsTest, getLanguageShouldReturnTheCurrentValue) {
            YAML::Node inputYaml;
            inputYaml["language"] = "fr";

            settings.load(inputYaml);

            EXPECT_EQ("fr", settings.getLanguage().GetLocale());
        }

        TEST_F(LootSettingsTest, isWindowPositionStoredShouldReturnFalseIfAllPositionValuesAreZero) {
            LootSettings::WindowPosition position;
            settings.storeWindowPosition(position);

            EXPECT_FALSE(settings.isWindowPositionStored());
        }

        TEST_F(LootSettingsTest, isWindowPositionStoredShouldReturnTrueIfTopPositionValueIsNonZero) {
            LootSettings::WindowPosition position;
            position.top = 1;
            settings.storeWindowPosition(position);

            EXPECT_TRUE(settings.isWindowPositionStored());
        }

        TEST_F(LootSettingsTest, isWindowPositionStoredShouldReturnTrueIfBottomPositionValueIsNonZero) {
            LootSettings::WindowPosition position;
            position.bottom = 1;
            settings.storeWindowPosition(position);

            EXPECT_TRUE(settings.isWindowPositionStored());
        }

        TEST_F(LootSettingsTest, isWindowPositionStoredShouldReturnTrueIfLeftPositionValueIsNonZero) {
            LootSettings::WindowPosition position;
            position.left = 1;
            settings.storeWindowPosition(position);

            EXPECT_TRUE(settings.isWindowPositionStored());
        }

        TEST_F(LootSettingsTest, isWindowPositionStoredShouldReturnTrueIfRightPositionValueIsNonZero) {
            LootSettings::WindowPosition position;
            position.right = 1;
            settings.storeWindowPosition(position);

            EXPECT_TRUE(settings.isWindowPositionStored());
        }

        TEST_F(LootSettingsTest, storeGameSettingsShouldReplaceExistingGameSettings) {
            const std::vector<GameSettings> gameSettings({GameSettings(GameType::tes5)});
            settings.storeGameSettings(gameSettings);

            EXPECT_EQ(gameSettings, settings.getGameSettings());
        }

        TEST_F(LootSettingsTest, storeLastGameShouldReplaceExistingValue) {
            settings.storeLastGame("Fallout3");

            EXPECT_EQ("Fallout3", settings.getLastGame());
        }

        TEST_F(LootSettingsTest, storeWindowPositionShouldReplaceExistingValue) {
            LootSettings::WindowPosition expectedPosition;
            expectedPosition.top = 1;
            settings.storeWindowPosition(expectedPosition);

            LootSettings::WindowPosition actualPosition = settings.getWindowPosition();
            EXPECT_EQ(expectedPosition.top, actualPosition.top);
            EXPECT_EQ(expectedPosition.bottom, actualPosition.bottom);
            EXPECT_EQ(expectedPosition.left, actualPosition.left);
            EXPECT_EQ(expectedPosition.right, actualPosition.right);
        }

        TEST_F(LootSettingsTest, updateLastVersionShouldSetValueToCurrentLootVersion) {
            const std::string currentVersion = LootVersion::string();
            YAML::Node inputYaml;
            inputYaml["lastVersion"] = "v0.7.1";

            settings.load(inputYaml);
            settings.updateLastVersion();

            EXPECT_EQ(currentVersion, settings.getLastVersion());
        }

        TEST_F(LootSettingsTest, toYamlShouldOutputStoredSettings) {
            const bool enableDebugLogging = true;
            const bool updateMasterlist = true;
            const std::string game = "Oblivion";
            const std::string language = "fr";
            const std::string lastGame = "Skyrim";
            const std::string lastVersion = "0.7.1";
            const std::map<std::string, long> window({
                {"top", 1},
                {"bottom", 2},
                {"left", 3},
                {"right", 4},
            });
            const std::vector<GameSettings> games({
                GameSettings(GameType::tes4).SetName("Game Name"),
            });
            const std::map<std::string, bool> filters({
                {"hideBashTags", false},
                {"hideCRCs", true},
            });

            YAML::Node inputYaml;
            inputYaml["enableDebugLogging"] = enableDebugLogging;
            inputYaml["updateMasterlist"] = updateMasterlist;
            inputYaml["game"] = game;
            inputYaml["language"] = language;
            inputYaml["lastGame"] = lastGame;
            inputYaml["lastVersion"] = lastVersion;
            inputYaml["window"] = window;
            inputYaml["games"] = games;
            inputYaml["filters"] = filters;

            settings.load(inputYaml);

            const YAML::Node outputYaml = settings.toYaml();

            EXPECT_EQ(enableDebugLogging, outputYaml["enableDebugLogging"].as<bool>());
            EXPECT_EQ(updateMasterlist, outputYaml["updateMasterlist"].as<bool>());
            EXPECT_EQ(game, outputYaml["game"].as<std::string>());
            EXPECT_EQ(language, outputYaml["language"].as<std::string>());
            EXPECT_EQ(lastGame, outputYaml["lastGame"].as<std::string>());
            EXPECT_EQ(lastVersion, outputYaml["lastVersion"].as<std::string>());

            for (const auto& position : window) {
                EXPECT_EQ(position.second, outputYaml["window"][position.first].as<long>());
            }

            for (const auto& filter : filters) {
                EXPECT_EQ(filter.second, outputYaml["filters"][filter.first].as<bool>());
            }

            EXPECT_EQ(games[0].Name(), outputYaml["games"][0]["name"].as<std::string>());
        }
    }
}

#endif
