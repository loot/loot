/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014 WrinklyNinja

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

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_DETAIL_TEST
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_DETAIL_TEST

#include "gui/state/game/detection/detail.h"
#include "tests/common_game_test_fixture.h"

namespace loot::test {
static const std::array<InstallSource, 5> ALL_INSTALL_SOURCES = {
    InstallSource::steam,
    InstallSource::gog,
    InstallSource::epic,
    InstallSource::microsoft,
    InstallSource::unknown};

class GetDefaultLootFolderNameTest : public ::testing::TestWithParam<GameId> {};

INSTANTIATE_TEST_SUITE_P(,
                         GetDefaultLootFolderNameTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(GetDefaultLootFolderNameTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(GetDefaultLootFolderName(GetParam()));
}

class GetDefaultMasterlistRepositoryNameTest
    : public ::testing::TestWithParam<GameId> {};

INSTANTIATE_TEST_SUITE_P(,
                         GetDefaultMasterlistRepositoryNameTest,
                         ::testing::ValuesIn(ALL_GAME_IDS));

TEST_P(GetDefaultMasterlistRepositoryNameTest,
       shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(GetDefaultMasterlistRepositoryName(GetParam()));
}

class GetSourceDescriptionTest
    : public ::testing::TestWithParam<InstallSource> {};

INSTANTIATE_TEST_SUITE_P(,
                         GetSourceDescriptionTest,
                         ::testing::ValuesIn(ALL_INSTALL_SOURCES));

TEST_P(GetSourceDescriptionTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(GetSourceDescription(GetParam()));
}

class GetNameSourceSuffixTest : public ::testing::TestWithParam<InstallSource> {
};

INSTANTIATE_TEST_SUITE_P(,
                         GetNameSourceSuffixTest,
                         ::testing::ValuesIn(ALL_INSTALL_SOURCES));

TEST_P(GetNameSourceSuffixTest, shouldNotThrowForAnyValidGameId) {
  EXPECT_NO_THROW(GetNameSourceSuffix(GetParam()));
}
TEST(FindGameInstalls, shouldReturnAnEmptyVectorIfNoGamesAreInstalled) {
  EXPECT_TRUE(FindGameInstalls({}, {}).empty());
}

TEST(CountGameInstalls, shouldCountConfiguredAndNewInstallsByGameAndSource) {
  const std::vector<GameInstall> configuredInstalls{
      {GameId::tes3, InstallSource::epic},
      {GameId::tes4, InstallSource::gog},
      {GameId::tes4, InstallSource::steam}};

  const std::vector<GameInstall> newInstalls{
      {GameId::tes3, InstallSource::epic},
      {GameId::tes5, InstallSource::microsoft}};

  const auto counts = CountGameInstalls(configuredInstalls, newInstalls);

  EXPECT_EQ(3, counts.size());
  EXPECT_EQ(1, counts.at(GameId::tes3).size());
  EXPECT_EQ(2, counts.at(GameId::tes3).at(InstallSource::epic));
  EXPECT_EQ(2, counts.at(GameId::tes4).size());
  EXPECT_EQ(1, counts.at(GameId::tes4).at(InstallSource::gog));
  EXPECT_EQ(1, counts.at(GameId::tes4).at(InstallSource::steam));
  EXPECT_EQ(1, counts.at(GameId::tes5).size());
  EXPECT_EQ(1, counts.at(GameId::tes5).at(InstallSource::microsoft));
}

TEST(DeriveName, shouldUseBaseNameIfItDoesNotAlreadyExist) {
  const auto baseName = "Morrowind";
  const auto name = DeriveName({GameId::tes3, InstallSource::unknown},
                               baseName,
                               {{GameId::tes3, {{InstallSource::gog, 1}}},
                                {GameId::tes4, {{InstallSource::steam, 1}}}},
                               {});

  EXPECT_EQ(baseName, name);
}

TEST(DeriveName, shouldAddSourceToNameIfAnotherSourceForTheSameGameExists) {
  const std::string baseName = "Morrowind";
  const auto name = DeriveName(
      {GameId::tes3, InstallSource::gog},
      baseName,
      {{GameId::tes3, {{InstallSource::gog, 1}, {InstallSource::steam, 1}}},
       {GameId::tes4, {{InstallSource::steam, 1}}}},
      {});

  EXPECT_EQ(baseName + " (GOG)", name);
}

TEST(DeriveName, shouldAddIndexToNameIfItAlreadyExists) {
  const std::string baseName = "Morrowind";
  const auto name = DeriveName({GameId::tes3, InstallSource::unknown},
                               baseName,
                               {{GameId::tes3, {{InstallSource::gog, 1}}},
                                {GameId::tes4, {{InstallSource::steam, 1}}}},
                               {baseName});

  EXPECT_EQ(baseName + " (1)", name);
}

TEST(DeriveName, shouldAddIndexToNameWithSourceIfItAlreadyExists) {
  const std::string baseName = "Morrowind";
  const auto name = DeriveName(
      {GameId::tes3, InstallSource::gog},
      baseName,
      {{GameId::tes3, {{InstallSource::gog, 1}, {InstallSource::steam, 1}}},
       {GameId::tes4, {{InstallSource::steam, 1}}}},
      {baseName + " (GOG)"});

  EXPECT_EQ(baseName + " (GOG) (1)", name);
}

TEST(DeriveName, shouldIncrementIndexIfNameWithIndexAlreadyExists) {
  const std::string baseName = "Morrowind";
  const auto name = DeriveName({GameId::tes3, InstallSource::gog},
                               baseName,
                               {{GameId::tes3, {{InstallSource::gog, 1}}},
                                {GameId::tes4, {{InstallSource::steam, 1}}}},
                               {baseName, baseName + " (1)"});

  EXPECT_EQ(baseName + " (2)", name);
}

TEST(UpdateSettingsPaths, shouldUpdateInstallPathIfItIsCurrentlyEmpty) {
  GameSettings settings;
  GameInstall install;
  install.installPath = "test";

  ASSERT_TRUE(settings.GamePath().empty());

  UpdateSettingsPaths(settings, install);

  EXPECT_EQ(install.installPath, settings.GamePath());
}

TEST(UpdateSettingsPaths, shouldNotUpdateInstallPathIfItIsNotCurrentlyEmpty) {
  const std::filesystem::path initialPath = "initial";

  GameSettings settings;
  settings.SetGamePath(initialPath);

  GameInstall install;
  install.installPath = "test";

  UpdateSettingsPaths(settings, install);

  EXPECT_EQ(initialPath, settings.GamePath());
}

TEST(UpdateSettingsPaths, shouldUpdateLocalPathIfItIsCurrentlyEmpty) {
  GameSettings settings;
  GameInstall install;
  install.localPath = "test";

  ASSERT_TRUE(settings.GameLocalPath().empty());

  UpdateSettingsPaths(settings, install);

  EXPECT_EQ(install.localPath, settings.GameLocalPath());
}

TEST(UpdateSettingsPaths, shouldNotUpdateLocalPathIfItIsNotCurrentlyEmpty) {
  const std::filesystem::path initialPath = "initial";

  GameSettings settings;
  settings.SetGameLocalPath(initialPath);

  GameInstall install;
  install.localPath = "test";

  UpdateSettingsPaths(settings, install);

  EXPECT_EQ(initialPath, settings.GameLocalPath());
}

TEST(
    UpdateMatchingSettings,
    shouldUpdateSettingsPathsIfThePrimaryComparatorFindsASettingsObjectThatMatchesAnInstall) {
  const std::filesystem::path installPath = "install";
  const std::filesystem::path localPath = "local";
  std::vector<GameSettings> gamesSettings{
      GameSettings(), GameSettings().SetGamePath(installPath)};
  const std::vector<GameInstall> gameInstalls{GameInstall{
      GameId::tes5, InstallSource::unknown, installPath, localPath}};
  const auto comparator = [](const GameSettings& settings,
                             const GameInstall& install) {
    return settings.GamePath() == install.installPath;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  EXPECT_TRUE(newInstalls.empty());
  EXPECT_TRUE(gamesSettings[0].GamePath().empty());
  EXPECT_TRUE(gamesSettings[0].GameLocalPath().empty());
  EXPECT_EQ(installPath, gamesSettings[1].GamePath());
  EXPECT_EQ(localPath, gamesSettings[1].GameLocalPath());
}

TEST(
    UpdateMatchingSettings,
    shouldTryMatchingSettingsWithEmptyInstallPathByGameTypeIfPrimaryComparatorFindsNoMatch) {
  const std::filesystem::path installPath = "install";
  const std::filesystem::path localPath = "local";
  std::vector<GameSettings> gamesSettings{GameSettings(),
                                          GameSettings(GameType::fo4)};
  const std::vector<GameInstall> gameInstalls{
      GameInstall{GameId::fo4, InstallSource::unknown, installPath, localPath}};
  const auto comparator = [](const GameSettings&, const GameInstall&) {
    return false;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  EXPECT_TRUE(newInstalls.empty());
  EXPECT_TRUE(gamesSettings[0].GamePath().empty());
  EXPECT_TRUE(gamesSettings[0].GameLocalPath().empty());
  EXPECT_EQ(installPath, gamesSettings[1].GamePath());
  EXPECT_EQ(localPath, gamesSettings[1].GameLocalPath());
}

TEST(UpdateMatchingSettings,
     shouldTrySecondaryMatchingByGameTypeAndMasterFilenameIfGameTypeIsTes4) {
  const std::filesystem::path installPath1 = "install1";
  const std::filesystem::path installPath2 = "install2";
  const std::filesystem::path localPath1 = "local1";
  const std::filesystem::path localPath2 = "local2";
  std::vector<GameSettings> gamesSettings{
      GameSettings(GameType::tes4).SetMaster("Nehrim.esm"),
      GameSettings(GameType::tes4),
      GameSettings(GameType::tes4).SetMaster("other.esm")};
  const std::vector<GameInstall> gameInstalls{
      GameInstall{
          GameId::nehrim, InstallSource::unknown, installPath1, localPath1},
      GameInstall{
          GameId::tes4, InstallSource::unknown, installPath2, localPath2}};
  const auto comparator = [](const GameSettings&, const GameInstall&) {
    return false;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  EXPECT_TRUE(newInstalls.empty());
  EXPECT_EQ(installPath1, gamesSettings[0].GamePath());
  EXPECT_EQ(localPath1, gamesSettings[0].GameLocalPath());
  EXPECT_EQ(installPath2, gamesSettings[1].GamePath());
  EXPECT_EQ(localPath2, gamesSettings[1].GameLocalPath());
  EXPECT_TRUE(gamesSettings[2].GamePath().empty());
  EXPECT_TRUE(gamesSettings[2].GameLocalPath().empty());
}

TEST(UpdateMatchingSettings,
     shouldTrySecondaryMatchingByGameTypeAndNameIfGameTypeIsTes5) {
  const std::filesystem::path installPath1 = "install1";
  const std::filesystem::path installPath2 = "install2";
  const std::filesystem::path localPath1 = "local1";
  const std::filesystem::path localPath2 = "local2";
  std::vector<GameSettings> gamesSettings{
      GameSettings(GameType::tes5).SetName("A name with Enderal in it"),
      GameSettings(GameType::tes5)};
  const std::vector<GameInstall> gameInstalls{
      GameInstall{
          GameId::enderal, InstallSource::unknown, installPath1, localPath1},
      GameInstall{
          GameId::tes5, InstallSource::unknown, installPath2, localPath2}};
  const auto comparator = [](const GameSettings&, const GameInstall&) {
    return false;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  EXPECT_TRUE(newInstalls.empty());
  EXPECT_EQ(installPath1, gamesSettings[0].GamePath());
  EXPECT_EQ(localPath1, gamesSettings[0].GameLocalPath());
  EXPECT_EQ(installPath2, gamesSettings[1].GamePath());
  EXPECT_EQ(localPath2, gamesSettings[1].GameLocalPath());
}

TEST(UpdateMatchingSettings,
     shouldTrySecondaryMatchingByGameTypeAndNameIfGameTypeIsTes5se) {
  const std::filesystem::path installPath1 = "install1";
  const std::filesystem::path installPath2 = "install2";
  const std::filesystem::path localPath1 = "local1";
  const std::filesystem::path localPath2 = "local2";
  std::vector<GameSettings> gamesSettings{
      GameSettings(GameType::tes5se).SetName("A name with Enderal in it"),
      GameSettings(GameType::tes5se)};
  const std::vector<GameInstall> gameInstalls{
      GameInstall{
          GameId::enderalse, InstallSource::unknown, installPath1, localPath1},
      GameInstall{
          GameId::tes5se, InstallSource::unknown, installPath2, localPath2}};
  const auto comparator = [](const GameSettings&, const GameInstall&) {
    return false;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  EXPECT_TRUE(newInstalls.empty());
  EXPECT_EQ(installPath1, gamesSettings[0].GamePath());
  EXPECT_EQ(localPath1, gamesSettings[0].GameLocalPath());
  EXPECT_EQ(installPath2, gamesSettings[1].GamePath());
  EXPECT_EQ(localPath2, gamesSettings[1].GameLocalPath());
}

TEST(UpdateMatchingSettings,
     shouldIncludeInstallInReturnedVectorIfItMatchesNoSettings) {
  const std::filesystem::path installPath = "install";
  const std::filesystem::path localPath = "local";
  std::vector<GameSettings> gamesSettings{GameSettings(GameType::tes4)};
  const std::vector<GameInstall> gameInstalls{GameInstall{
      GameId::tes3, InstallSource::unknown, installPath, localPath}};
  const auto comparator = [](const GameSettings&, const GameInstall&) {
    return false;
  };

  const auto newInstalls =
      UpdateMatchingSettings(gamesSettings, gameInstalls, comparator);

  ASSERT_EQ(1, newInstalls.size());
  EXPECT_EQ(gameInstalls[0].gameId, newInstalls[0].gameId);
  EXPECT_EQ(gameInstalls[0].source, newInstalls[0].source);
  EXPECT_EQ(gameInstalls[0].installPath, newInstalls[0].installPath);
  EXPECT_EQ(gameInstalls[0].localPath, newInstalls[0].localPath);

  EXPECT_TRUE(gamesSettings[0].GamePath().empty());
  EXPECT_TRUE(gamesSettings[0].GameLocalPath().empty());
}

TEST(AppendNewGamesSettings,
     shouldAppendSettingsForEachNewInstallUsingGameDefaults) {
  const std::filesystem::path installPath = "install";
  const std::filesystem::path localPath = "local";
  std::vector<GameSettings> gamesSettings;

  AppendNewGamesSettings(
      gamesSettings,
      {{GameId::enderal, {{InstallSource::unknown, 1}}}},
      {{GameId::enderal, InstallSource::unknown, installPath, localPath},
       {GameId::nehrim}});

  ASSERT_EQ(2, gamesSettings.size());

  EXPECT_EQ(GameType::tes5, gamesSettings[0].Type());
  EXPECT_EQ("Enderal", gamesSettings[0].FolderName());
  EXPECT_EQ("Enderal: Forgotten Stories", gamesSettings[0].Name());
  EXPECT_EQ("Skyrim.esm", gamesSettings[0].Master());
  EXPECT_EQ(
      "https://raw.githubusercontent.com/loot/enderal/v0.18/masterlist.yaml",
      gamesSettings[0].MasterlistSource());
  EXPECT_EQ(installPath, gamesSettings[0].GamePath());
  EXPECT_EQ(localPath, gamesSettings[0].GameLocalPath());

  EXPECT_EQ(GameType::tes4, gamesSettings[1].Type());
  EXPECT_EQ("Nehrim", gamesSettings[1].FolderName());
  EXPECT_EQ("Nehrim - At Fate's Edge", gamesSettings[1].Name());
  EXPECT_EQ("Nehrim.esm", gamesSettings[1].Master());
  EXPECT_EQ(
      "https://raw.githubusercontent.com/loot/oblivion/v0.18/masterlist.yaml",
      gamesSettings[1].MasterlistSource());
}

TEST(AppendNewGamesSettings,
     shouldDeriveGameAndFolderNamesUsingTheExistingNamesOfPreviousSettings) {
  std::vector<GameSettings> gamesSettings{
      GameSettings(GameType::tes3, "Morrowind").SetName("TES III: Morrowind")};

  AppendNewGamesSettings(
      gamesSettings,
      {{GameId::tes3, {{InstallSource::gog, 3}}}},
      {{GameId::tes3, InstallSource::gog}, {GameId::tes3, InstallSource::gog}});

  ASSERT_EQ(3, gamesSettings.size());
  EXPECT_EQ("Morrowind", gamesSettings[0].FolderName());
  EXPECT_EQ("TES III: Morrowind", gamesSettings[0].Name());
  EXPECT_EQ("Morrowind (1)", gamesSettings[1].FolderName());
  EXPECT_EQ("TES III: Morrowind (1)", gamesSettings[1].Name());
  EXPECT_EQ("Morrowind (2)", gamesSettings[2].FolderName());
  EXPECT_EQ("TES III: Morrowind (2)", gamesSettings[2].Name());
}
}

#endif
